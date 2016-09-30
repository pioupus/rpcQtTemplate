/*
 * channel.c
 *
 *  Created on: 30.03.2015
 *      Author: ak
 */

#include <stdbool.h>

#include <stdio.h>
#include "errorlogger/generic_eeprom_errorlogger.h"
#include "channel_codec/channel_codec.h"
#include "channel_codec/crc16.h"
#include "channel_codec/phylayer.h"


#define CRC_LENGTH 2


#if CHANNEL_BLOCKLENGTH == 16
#define BLOCK_SIZEFACTOR 2
#elif CHANNEL_BLOCKLENGTH == 8
#define BLOCK_SIZEFACTOR 1
#else
#error illegal CHANNEL_BLOCKLENGTH
#endif




static void channel_decode(channel_codec_instance_t *instance,unsigned char byte);
static void channel_encode(channel_codec_instance_t *instance,unsigned char byte);


static uint8_t searchForPreamble(channel_codec_instance_t *instance, uint8_t byte);
static bool isRPCAnswer(channel_codec_instance_t *instance, const size_t size_bytes, RPC_SIZE_RESULT *sizeResult);
static bool isRPCRequest(channel_codec_instance_t *instance, const size_t size_bytes, RPC_SIZE_RESULT *sizeResult);

bool channel_is_initialized(channel_codec_instance_t *instance){
	bool result = true;
	if (instance == NULL){
		result = false;
	}

	if (instance->i.initialized == false){
		result = false;
	}

	if ((instance->i.rxState.buffer && instance->i.txState.buffer && instance->i.rxState.bufferLength && instance->i.txState.bufferLength) == false){
		result = false;
	}

	return result;
}

static void assertIfNotInitialized(channel_codec_instance_t *instance){
	if (channel_is_initialized(instance) == false){
		GEN_ASSERT(0,errlog_E_CHCODEC_instance_index_beyond_max, "CC instance index beyond max\n");
	}
}

static void reset_rx(channel_codec_instance_t *instance){
	assertIfNotInitialized(instance);
	instance->i.rxState.writePointer=0;
	instance->i.rxState.bitmask=0;
	instance->i.rxState.indexInBlock=0;
	instance->i.rxState.messageResult.result = RPC_COMMAND_UNKNOWN;
	instance->i.ccChannelState = csNone;
}

static void reset_tx(channel_codec_instance_t *instance){
	assertIfNotInitialized(instance);
	instance->i.txState.writePointer=0;
	instance->i.txState.bitMaskPositionInBuffer = 0;
	instance->i.txState.indexInBlock = 0;
	crc16_online_init(&instance->i.txState.crc16);
}

void channel_init_instance(channel_codec_instance_t *instance, char *rxBuffer, size_t rxBufferLength, char *txBuffer, size_t txBufferLength){

	instance->i.rxState.buffer = rxBuffer;
	instance->i.rxState.bufferLength = rxBufferLength;
	instance->i.txState.buffer = txBuffer;
	instance->i.txState.bufferLength = txBufferLength;
	instance->i.initialized = rxBuffer && txBuffer && rxBufferLength && txBufferLength;


	reset_tx(instance);
	reset_rx(instance);
	instance->i.rxState.preambleBuffer[0] = 0;
	instance->i.rxState.preambleBuffer[1] = 0;
	instance->i.rxState.preambleBuffer[2] = 0;

	RPC_CHANNEL_CODEC_parser_init(instance);
}

void channel_uninit_instance(channel_codec_instance_t *instance){
	instance->i.rxState.buffer = 0;
	instance->i.rxState.bufferLength = 0;
	instance->i.txState.buffer = 0;
	instance->i.txState.bufferLength = 0;
	instance->i.initialized = 0;
}

static void appendByteToTXBuffer(channel_codec_instance_t *instance, char byte){
	assertIfNotInitialized(instance);
	if (instance->i.txState.writePointer < instance->i.txState.bufferLength){
		instance->i.txState.buffer[instance->i.txState.writePointer] = byte;
		instance->i.txState.writePointer++;
	}else{
		GEN_ASSERT(0,errlog_E_CHCODEC_exceeding_RPC_TX_Buffer, "CC Exceeding RPC TX Buffer %d\n",instance->i.txState.writePointer);
	}
}

static void appendByteToRXBuffer(channel_codec_instance_t *instance, char byte){
	assertIfNotInitialized(instance);
	if (instance->i.rxState.writePointer < instance->i.rxState.bufferLength){
#if 0
		printf("[%d] %02X\n",channel_rx_write_pointer,(unsigned char)byte);
#endif
		instance->i.rxState.buffer[instance->i.rxState.writePointer] = byte;
		instance->i.rxState.writePointer++;
	}else{
		reset_rx(instance);
        GEN_WARNING_CC(instance, errlog_W_CHCODEC_exceeding_RPC_RX_buffer,"CC Exceeding RPC RX Buffer %d. Buffer reset\n",instance->i.rxState.writePointer);
	}
}

void channel_start_message_from_RPC(channel_codec_instance_t *instance, size_t size){
/*  This function is called when a new message starts. {size} is the number of
    bytes the message will require. In the implementation you can allocate  a
    buffers or write a preamble. The implementation can be empty if you do not
    need to do that. */
	assertIfNotInitialized(instance);

	(void)size;
	reset_tx(instance);
	appendByteToTXBuffer(instance,0xFF);
	appendByteToTXBuffer(instance,0xFF);
	appendByteToTXBuffer(instance,0xFF);
}

void channel_decode(channel_codec_instance_t *instance, unsigned char byte){
	assertIfNotInitialized(instance);
	//indexInBlock index == 0: incoming byte is a bitmask
	//indexInBlock index == 1: incoming byte is first Byte in Block
	if (instance->i.rxState.indexInBlock == 0){
		instance->i.rxState.bitmask = byte;
		#if 0
		printf("channel_rx_bitmask %02X\n",channel_rx_bitmask);
		#endif
	}else {/*  channel_rx_bit_pointer > 0 */
		uint8_t bit = instance->i.rxState.bitmask;
		if ((BLOCK_SIZEFACTOR == 1) || (instance->i.rxState.indexInBlock & (BLOCK_SIZEFACTOR-1))){
			bit >>= (instance->i.rxState.indexInBlock-1)/BLOCK_SIZEFACTOR;
			bit &= 1;
		}else{
			bit = 0;
		}
		byte |= bit;
		appendByteToRXBuffer(instance,byte);
	}
	instance->i.rxState.indexInBlock++;
	if(instance->i.rxState.indexInBlock==CHANNEL_BLOCKLENGTH-BLOCK_SIZEFACTOR+1){
		instance->i.rxState.indexInBlock=0;
	}
}

void channel_encode(channel_codec_instance_t *instance, unsigned char byte){
	assertIfNotInitialized(instance);
	if (instance->i.txState.bitMaskPositionInBuffer == 0){
		appendByteToTXBuffer(instance,0);
		instance->i.txState.bitMaskPositionInBuffer = instance->i.txState.writePointer-1;
		instance->i.txState.indexInBlock = 0;
	}
	uint8_t bit = byte & 1;

	if ((instance->i.txState.indexInBlock & (BLOCK_SIZEFACTOR-1))==0){
		uint8_t shiftBy = instance->i.txState.indexInBlock/BLOCK_SIZEFACTOR;
		instance->i.txState.buffer[instance->i.txState.bitMaskPositionInBuffer] |= bit << shiftBy;
		byte &= 0xFE;
	}
	appendByteToTXBuffer(instance,byte);
	instance->i.txState.indexInBlock++;
	if (instance->i.txState.indexInBlock == CHANNEL_BLOCKLENGTH-BLOCK_SIZEFACTOR){
#if 0
		printf("mask: %02X\n",(unsigned char)channel_tx_buffer[bitMaskBitPointer]);
#endif
		instance->i.txState.bitMaskPositionInBuffer = 0;
		instance->i.txState.indexInBlock = 0;
	}
#if 0
	printf("%02X\n",(unsigned char)byte);
#endif

}

void channel_push_byte_from_RPC(channel_codec_instance_t *instance, unsigned char byte){
	assertIfNotInitialized(instance);
/* Pushes a byte to be sent via network. You should put all the pushed bytes
   into a buffer and send the buffer when RPC_commit is called. If you run
   out of buffer you can send multiple partial messages as long as the other
   side puts them back together. */
	crc16_online(byte,&instance->i.txState.crc16);
#if 0
	printf("%02X ",(unsigned char)byte);
#endif
	channel_encode(instance,byte);
}

RPC_RESULT channel_commit_from_RPC(channel_codec_instance_t *instance){
	assertIfNotInitialized(instance);
	appendByteToTXBuffer(instance,instance->i.txState.crc16);
	appendByteToTXBuffer(instance,instance->i.txState.crc16 >> 8) ;
	/*printf("%X\n",crc16val);*/
	//printf("commit %d\n",txState.buffer[4]);
	RPC_RESULT result = phyPushDataBuffer(instance, instance->i.txState.buffer,instance->i.txState.writePointer);
	reset_tx(instance);
	return result;
}

static uint8_t searchForPreamble(channel_codec_instance_t *instance, uint8_t byte){
	assertIfNotInitialized(instance);
	instance->i.rxState.preambleBuffer[0] = instance->i.rxState.preambleBuffer[1];
	instance->i.rxState.preambleBuffer[1] = instance->i.rxState.preambleBuffer[2];
	instance->i.rxState.preambleBuffer[2] = byte;
#if 0
	printf("%02X %02X %02X\n",channel_rx_preamblebuffer[0],
			channel_rx_preamblebuffer[2],
			channel_rx_preamblebuffer[1]
		);
#endif

	if(	(instance->i.rxState.preambleBuffer[0]==0xFF)&&
		(instance->i.rxState.preambleBuffer[2]==0xFF)&&
		(instance->i.rxState.preambleBuffer[1]==0xFF)){

		return 1;
	}
	return 0;
}

bool isRPCAnswer(channel_codec_instance_t *instance, const size_t size_bytes, RPC_SIZE_RESULT *sizeResult){
	assertIfNotInitialized(instance);
	bool result=false;
	RPC_SIZE_RESULT testResult;
	if(size_bytes == 0){
		testResult.result = RPC_COMMAND_INCOMPLETE;
		testResult.size = 1;
	}else{
		testResult = RPC_CHANNEL_CODEC_get_answer_length(instance,instance->i.rxState.buffer, size_bytes);
	}
	if (testResult.result == RPC_SUCCESS){
		result = true;
		*sizeResult = testResult;
	}else if (testResult.result == RPC_COMMAND_INCOMPLETE){
		*sizeResult = testResult;
    }else{//RPC_TRANSMISSION_FAILURE or RPC_COMMAND_UNKNOWN
	}
	return result;
}

bool isRPCRequest(channel_codec_instance_t *instance,  const size_t size_bytes, RPC_SIZE_RESULT *sizeResult){
	assertIfNotInitialized(instance);
	bool result=false;
	RPC_SIZE_RESULT testResult;
	if(size_bytes == 0){
		testResult.result = RPC_COMMAND_INCOMPLETE;
		testResult.size = 1;
	}else{
		testResult = RPC_CHANNEL_CODEC_get_request_size(instance,instance->i.rxState.buffer, size_bytes);
	}
	if (testResult.result == RPC_SUCCESS){
		result = true;
		*sizeResult = testResult;
	}else if (testResult.result == RPC_COMMAND_INCOMPLETE){
		*sizeResult = testResult;
	}else{//RPC_TRANSMISSION_FAILURE or RPC_COMMAND_UNKNOWN


	}
	return result;
}


void channel_push_byte_to_RPC(channel_codec_instance_t *instance, unsigned char byte)
{
	assertIfNotInitialized(instance);
	if(searchForPreamble(instance,byte)){
		reset_rx(instance);
		instance->i.ccChannelState = csFoundPreamble;
		#if 0
		printf("found preamble\n");
		#endif

	}
	switch(instance->i.ccChannelState){
	case csNone:
		break;

	case csFoundPreamble:
		instance->i.ccChannelState = csLoadingPayload;
		break;

	case csLoadingPayload:
		channel_decode(instance,byte);
		#if 0
		printf("payload[%d] %02X\n",channel_rx_write_pointer-1, byte);
		#endif
		if (instance->i.rxState.messageResult.result != RPC_SUCCESS){

            if (isRPCAnswer(instance,instance->i.rxState.writePointer,&instance->i.rxState.messageResult) ) {

            }else if (isRPCRequest(instance,instance->i.rxState.writePointer, &instance->i.rxState.messageResult)){

            }

			#if 0
			printf("%d, %d %d\n",channel_rx_message_size.size,channel_rx_message_size.result, channel_rx_write_pointer);
			#endif
		}
		if ((instance->i.rxState.messageResult.result==RPC_SUCCESS) && (instance->i.rxState.messageResult.size == instance->i.rxState.writePointer)){
			instance->i.ccChannelState = csPayloadComplete;
		}
		break;
	case csPayloadComplete:
		appendByteToRXBuffer(instance,byte);//receive CRC
		#if 0
		printf("channel_rx_payload_complete %d\n",channel_rx_write_pointer);
		#endif
		if (instance->i.rxState.messageResult.size+CRC_LENGTH== instance->i.rxState.writePointer){
			instance->i.ccChannelState = csCRCAndPackageComplete;
		}else{
			break;
		}

	case csCRCAndPackageComplete:
		{

			uint16_t crc16val = crc16_buffer((uint8_t*)instance->i.rxState.buffer,instance->i.rxState.writePointer-CRC_LENGTH);
			uint8_t crc_16_msb = crc16val >> 8;
			uint8_t crc_16_lsb = crc16val & 0xFF;
	#if 0
			for (int i=0;i<channel_rx_write_pointer-2;i++){
				printf("%02X ",(unsigned char)channel_rx_buffer[i]);
			}


			printf("\n   [%d]%02X  %02X  \n",channel_rx_write_pointer-1,(unsigned char)channel_rx_buffer[channel_rx_write_pointer-CRC_LENGTH+1],crc_16_msb);
			printf("   [%d]%02X  %02X  \n",channel_rx_write_pointer-2,(unsigned char)channel_rx_buffer[channel_rx_write_pointer-CRC_LENGTH],crc_16_lsb);

			printf("%04X ",crc16val);
	#endif
			if ((crc_16_msb == (unsigned char)instance->i.rxState.buffer[instance->i.rxState.writePointer-CRC_LENGTH+1])
					&& (crc_16_lsb == (unsigned char)instance->i.rxState.buffer[instance->i.rxState.writePointer-CRC_LENGTH])){
				RPC_SIZE_RESULT rpcTRANSMISSIONSize;
				rpcTRANSMISSIONSize.result = RPC_COMMAND_UNKNOWN;
                if (isRPCAnswer(instance, instance->i.rxState.writePointer-CRC_LENGTH, &rpcTRANSMISSIONSize) ) {
					if (rpcTRANSMISSIONSize.result == RPC_SUCCESS){
						RPC_CHANNEL_CODEC_parse_answer(instance, instance->i.rxState.buffer, instance->i.rxState.writePointer-CRC_LENGTH);
					}
                }else if (isRPCRequest(instance, instance->i.rxState.writePointer-CRC_LENGTH, &rpcTRANSMISSIONSize)){
					if (rpcTRANSMISSIONSize.result == RPC_SUCCESS){
						RPC_CHANNEL_CODEC_parse_request(instance, instance->i.rxState.buffer, instance->i.rxState.writePointer-CRC_LENGTH);
					}
				}else{
					GEN_ASSERT(0,errlog_E_CHCODEC_RPC_parse_answer_request_Fail,"CC RPC_parse_answer/request Fail");
				}
			}else{
                GEN_WARNING_CC(instance, errlog_W_CHCODEC_RX_CRC_fail,"CC RX CRC Fail");
			}
			reset_rx(instance);
			instance->i.ccChannelState = csNone;
		}
		break;
	default:
	break;
	}
}
