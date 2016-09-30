/*
 * channel_codec_test.cpp
 *
 *  Created on: 30.03.2015
 *      Author: ak
 */



#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "errorlogger/generic_eeprom_errorlogger.h"
#include "CppUTestExt/MockSupport.h"
#include "CppUTest/TestHarness.h"

#include "channel_codec/channel_codec.h"
#include "channel_codec/channel_codec_test.h"


uint16_t TXDataLength;

#define DISABLE_TESTS 1

#if CHANNEL_BLOCKLENGTH == 8
#elif CHANNEL_BLOCKLENGTH == 16
#else
#error CHANNEL_BLOCKLENGTH has to be defined with either 8 or 16
#endif

extern "C"
{
	#include "CppUTestExt/MockSupport_c.h"
	#include "channel_codec/phylayer.h"

	#define CHANNEL_CODEC_TX_BUFFER_SIZE 128
	#define CHANNEL_CODEC_RX_BUFFER_SIZE 64

	static channel_codec_instance_t instances[channel_codec_comport_COUNT];
	static char rxBuffers[channel_codec_comport_COUNT][CHANNEL_CODEC_RX_BUFFER_SIZE];
	static char txBuffers[channel_codec_comport_COUNT][CHANNEL_CODEC_TX_BUFFER_SIZE];

	void ChannelCodec_errorHandler(channel_codec_instance_t *instance, channelCodecErrorNum_t errNum){
		(void)errNum;
		(void)instance;
	}

	RPC_RESULT phyPushDataBuffer(channel_codec_instance_t *instance, const char *buffer, size_t length){
		/*printf("eunistonePushDataBuffer\n");*/
#if 0
		printf("\n");
		for (uint16_t i=0;i<length;i++){
			printf("%02X ",(unsigned char)buffer[i]);
		}
		printf("\n");
#endif

		mock_c()->actualCall("eunistonePushDataBuffer")
						->withIntParameters("length", length);
		TXDataLength = length;
		return RPC_SUCCESS;
	}

	RPC_SIZE_RESULT RPC_CHANNEL_CODEC_get_request_size(channel_codec_instance_t *instance, const void *buffer, size_t size_bytes){
		(void)instance;
		return RPC_TRANSMISSION_get_request_size(buffer, size_bytes);
	}

	RPC_SIZE_RESULT RPC_CHANNEL_CODEC_get_answer_length(channel_codec_instance_t *instance, const void *buffer, size_t size_bytes){
		(void)instance;
		return RPC_TRANSMISSION_get_answer_length(buffer, size_bytes);
	}

	void RPC_CHANNEL_CODEC_parser_init(channel_codec_instance_t *instance){
		(void)instance;
		RPC_TRANSMISSION_Parser_init();
	}

	void RPC_CHANNEL_CODEC_parse_request(channel_codec_instance_t *instance, const void *buffer, size_t size_bytes){
		(void)instance;
		RPC_TRANSMISSION_parse_request(buffer, size_bytes);
	}

	void RPC_CHANNEL_CODEC_parse_answer(channel_codec_instance_t *instance, const void *buffer, size_t size_bytes){
		(void)instance;
		RPC_TRANSMISSION_parse_answer(buffer, size_bytes);
	}



	void RPC_TRANSMISSION_parse_answer(const void *buffer, size_t size){
		(void)buffer;
		uint8_t *buf = (uint8_t*)buffer;
#if 0
		printf("RPC_parse_answer:\n");
		for (uint16_t i=0;i<size;i++){
			printf("%02X ",(unsigned char)buf[i]);
		}
		printf("\n");
#endif
		mock_c()->actualCall("RPC_parse_answer")
						->withIntParameters("firstByte", (unsigned char)buf[0])
						->withIntParameters("length", size);

		//RPC_RESULT result = RPC_SUCCESS;
		//return result;
	}

	void RPC_TRANSMISSION_parse_request(const void *buffer, size_t size){
		(void)buffer;
		uint8_t *buf = (uint8_t*)buffer;
#if 0
		printf("RPC_parse_answer:\n");
		for (uint16_t i=0;i<size;i++){
			printf("%02X ",(unsigned char)buf[i]);
		}
		printf("\n");
#endif
		mock_c()->actualCall("RPC_parse_request")
						->withIntParameters("firstByte", (unsigned char)buf[0])
						->withIntParameters("length", size);

		//RPC_RESULT result = RPC_SUCCESS;
		//return result;
	}
#if 0
	RPC_TRANSMISSION_SIZE_RESULT RPC_get_answer_length(const void *buffer, size_t size){
		RPC_TRANSMISSION_SIZE_RESULT result;
		uint8_t *buf = (uint8_t*)buffer;

		result.result = RPC_COMMAND_INCOMPLETE;
		result.size = 0;
		if(buf[0] == 0){
			if (size < 5){
				result.result = RPC_COMMAND_INCOMPLETE;
			}else{
				result.result = RPC_SUCCESS;
				result.size = 10;
			}
		}else if (buf[0]==1){
			result.result = RPC_SUCCESS;
			result.size = 50;
		}
		return result;
	}
#endif
	uint16_t getMsgLength(uint16_t payLoadLength, uint8_t blocklength){
		uint16_t result=payLoadLength;

		if (blocklength==8){
			result+=(payLoadLength/7)+1;
			if ((payLoadLength%7) == 0)
				result--;
		}else if(blocklength==16){
			result+=(payLoadLength/15)+1;
			if ((payLoadLength%15) == 0)
				result--;
		}
		result += 5;
		return result;

	}

	uint8_t getByteSequence(uint8_t firstByte, uint16_t index){
		uint8_t byte;
		if(index){
			byte = index | 0x80;
		}else{
			byte = firstByte;
		}
		return byte;
	}

#if 0
	void unistExecRDDSRES_cb_t(int64_t bdAdress,char *name, int8_t nameLength, uint32_t CoD){
		genMutexLock(&mutexMockCrtitcalSection);
		memcpy(MockBuffer,name,nameLength);
		MockBuffer[nameLength] = 0;
#if 0
		printf("unistExecRDDSRES_cb %s\n",MockBuffer);
#endif
		mock_c()->actualCall("unistExecRDDSRES_cb")
						->withIntParameters("param1a", (bdAdress >> 24) & 0xFFFFFF)
						->withIntParameters("param1b", bdAdress & 0xFFFFFF)
						->withIntParameters("param3", CoD);
		genMutexUnlock(&mutexMockCrtitcalSection);
	}
#endif


}


TEST_GROUP(channel_codec)
{
	void setup()
	{
		channel_init_instance(&instances[channel_codec_comport_uart],
										 rxBuffers[channel_codec_comport_uart],CHANNEL_CODEC_RX_BUFFER_SIZE,
										 txBuffers[channel_codec_comport_uart],CHANNEL_CODEC_TX_BUFFER_SIZE);
		instances[channel_codec_comport_uart].aux.port = channel_codec_comport_uart;
	}

	void teardown()
	{
		mock().clear();
		TXDataLength = 0;

	}

	void checkPreambel(channel_codec_instance_t *instance){
		CHECK_EQUAL(0xFF,(unsigned char)instance->i.txState.buffer[0]);
		CHECK_EQUAL(0xFF,(unsigned char)instance->i.txState.buffer[1]);
		CHECK_EQUAL(0xFF,(unsigned char)instance->i.txState.buffer[2]);
	}

};
#if 1 && DISABLE_TESTS
TEST(channel_codec, Initialization)
{
	memset(instances,0xFF,sizeof(instances));

	channel_init_instance(&instances[channel_codec_comport_uart],
			rxBuffers[channel_codec_comport_uart],CHANNEL_CODEC_RX_BUFFER_SIZE,
			txBuffers[channel_codec_comport_uart],CHANNEL_CODEC_TX_BUFFER_SIZE);

	instances[channel_codec_comport_uart].aux.port = channel_codec_comport_uart;

	CHECK_EQUAL(1,channel_is_initialized(&instances[channel_codec_comport_uart]));

	for (uint8_t i = 0; i<channel_codec_comport_COUNT;i++){
		CHECK_EQUAL(0,instances[i].i.txState.writePointer);
		CHECK_EQUAL(0,instances[i].i.txState.bitMaskPositionInBuffer);
		CHECK_EQUAL(0,instances[i].i.txState.indexInBlock);
		CHECK_EQUAL(CHANNEL_CODEC_TX_BUFFER_SIZE,instances[i].i.txState.bufferLength);

		CHECK_EQUAL(0,instances[i].i.rxState.writePointer);
		CHECK_EQUAL(0,instances[i].i.rxState.bitmask);
		CHECK_EQUAL(0,instances[i].i.rxState.indexInBlock);
		CHECK_EQUAL(CHANNEL_CODEC_RX_BUFFER_SIZE,instances[i].i.rxState.bufferLength);
	}
	channel_uninit_instance(&instances[channel_codec_comport_uart]);
	CHECK_EQUAL(0,channel_is_initialized(&instances[channel_codec_comport_uart]));
}
#endif

#if 1 && DISABLE_TESTS
TEST(channel_codec, channel_start_message_from_RPC)
{
	channel_start_message_from_RPC(&instances[channel_codec_comport_uart],10);
	checkPreambel(&instances[channel_codec_comport_uart]);
	CHECK_EQUAL(3,instances[channel_codec_comport_uart].i.txState.writePointer);
}
#endif

#if 1 && DISABLE_TESTS
TEST(channel_codec, channel_push_byte_from_RPC)
{
	const channel_codec_conf_comport_t cp = channel_codec_comport_uart;

	channel_start_message_from_RPC(&instances[cp],10);
	CHECK_EQUAL(3,instances[cp].i.txState.writePointer);

	channel_push_byte_from_RPC(&instances[cp],0x81);
	CHECK_EQUAL(1,instances[cp].i.txState.indexInBlock);
	CHECK_EQUAL(3,instances[cp].i.txState.bitMaskPositionInBuffer);
	channel_push_byte_from_RPC(&instances[cp],0x82);
	channel_push_byte_from_RPC(&instances[cp],0x81);
	channel_push_byte_from_RPC(&instances[cp],0x82);
	channel_push_byte_from_RPC(&instances[cp],0x81);
	channel_push_byte_from_RPC(&instances[cp],0x82);
	channel_push_byte_from_RPC(&instances[cp],0x81);
	checkPreambel(&instances[cp]);
#if CHANNEL_BLOCKLENGTH == 8
	CHECK_EQUAL(0,instances[cp].txState.indexInBlock);
	CHECK_EQUAL(0,instances[cp].txState.bitMaskPositionInBuffer);
	CHECK_EQUAL(0x55,(unsigned char)instances[cp].txState.buffer[3]);
#endif




	CHECK_EQUAL(0x80,(unsigned char)instances[cp].i.txState.buffer[4]);
	CHECK_EQUAL(0x82,(unsigned char)instances[cp].i.txState.buffer[5]);
	CHECK_EQUAL(0x80,(unsigned char)instances[cp].i.txState.buffer[6]);
	CHECK_EQUAL(0x82,(unsigned char)instances[cp].i.txState.buffer[7]);
	CHECK_EQUAL(0x80,(unsigned char)instances[cp].i.txState.buffer[8]);
	CHECK_EQUAL(0x82,(unsigned char)instances[cp].i.txState.buffer[9]);
	CHECK_EQUAL(0x80,(unsigned char)instances[cp].i.txState.buffer[10]);

	channel_push_byte_from_RPC(&instances[cp],0x81);
	channel_push_byte_from_RPC(&instances[cp],0x82);
	channel_push_byte_from_RPC(&instances[cp],0x81);
	channel_push_byte_from_RPC(&instances[cp],0x82);
	channel_push_byte_from_RPC(&instances[cp],0x81);
	channel_push_byte_from_RPC(&instances[cp],0x82);
	channel_push_byte_from_RPC(&instances[cp],0x81);
#if CHANNEL_BLOCKLENGTH == 16
	CHECK_EQUAL(0,instances[cp].i.txState.indexInBlock);
	CHECK_EQUAL(0,instances[cp].i.txState.bitMaskPositionInBuffer);
	CHECK_EQUAL(15,(unsigned char)instances[cp].i.txState.buffer[3]);
	CHECK_EQUAL(0x81,(unsigned char)instances[cp].i.txState.buffer[11]);
	CHECK_EQUAL(0x82,(unsigned char)instances[cp].i.txState.buffer[12]);
	CHECK_EQUAL(0x81,(unsigned char)instances[cp].i.txState.buffer[13]);
	CHECK_EQUAL(0x82,(unsigned char)instances[cp].i.txState.buffer[14]);
	CHECK_EQUAL(0x81,(unsigned char)instances[cp].i.txState.buffer[15]);
	CHECK_EQUAL(0x82,(unsigned char)instances[cp].i.txState.buffer[16]);
	CHECK_EQUAL(0x81,(unsigned char)instances[cp].i.txState.buffer[17]);
	CHECK_EQUAL(18,instances[cp].i.txState.writePointer);
#else
	CHECK_EQUAL(0x55,(unsigned char)instances[cp].txState.buffer[11]);
	CHECK_EQUAL(0x80,(unsigned char)instances[cp].txState.buffer[12]);
	CHECK_EQUAL(0x82,(unsigned char)instances[cp].txState.buffer[13]);
	CHECK_EQUAL(0x80,(unsigned char)instances[cp].txState.buffer[14]);
	CHECK_EQUAL(0x82,(unsigned char)instances[cp].txState.buffer[15]);
	CHECK_EQUAL(0x80,(unsigned char)instances[cp].txState.buffer[16]);
	CHECK_EQUAL(0x82,(unsigned char)instances[cp].txState.buffer[17]);
	CHECK_EQUAL(0x80,(unsigned char)instances[cp].txState.buffer[18]);
	CHECK_EQUAL(19,instances[cp].txState.writePointer);
#endif







}
#endif


#if 1 && DISABLE_TESTS
TEST(channel_codec, channel_commit_from_RPC)
{
	const channel_codec_conf_comport_t cp = channel_codec_comport_uart;
	RPC_RESULT result;

	uint8_t sendbuffer[16] = {0x10,0x18,0x81,0x82, 0x81,0x82,0x81,0x81,   0x81,0x82,0x81,0x82, 0x81,0x82,0x81,0x81};
	uint16_t msglengh=getMsgLength(16, CHANNEL_BLOCKLENGTH);

	mock().expectOneCall("eunistonePushDataBuffer").withParameter("length", msglengh);

	channel_start_message_from_RPC(&instances[cp],16);
	CHECK_EQUAL(3,instances[cp].i.txState.writePointer);


	for (int i=0;i<16;i++){
		channel_push_byte_from_RPC(&instances[cp],sendbuffer[i]);
#if 0
		printf("%02X ",(unsigned char)sendbuffer[i]);
#endif
	}

#if 0
	printf("\n");
	for (int i=0;i<22;i++){
		printf("%02X ",(unsigned char)channel_tx_buffer[i]);
	}
	printf("\n");
#endif
	result = channel_commit_from_RPC(&instances[cp]);
#if CHANNEL_BLOCKLENGTH==8
	CHECK_EQUAL(0xB6,(unsigned char)instances[cp].i.txState.buffer[22]);
	CHECK_EQUAL(0xBD,(unsigned char)instances[cp].i.txState.buffer[23]);
#elif CHANNEL_BLOCKLENGTH==16
	CHECK_EQUAL(0xB6,(unsigned char)instances[cp].i.txState.buffer[21]);
	CHECK_EQUAL(0xBD,(unsigned char)instances[cp].i.txState.buffer[22]);
#endif
	mock().checkExpectations();

}
#endif

#if 1 && DISABLE_TESTS
TEST(channel_codec, channel_push_to_RPC_find_preamble)
{
	const channel_codec_conf_comport_t cp = channel_codec_comport_uart;
	channel_push_byte_to_RPC(&instances[cp],0x81);
	channel_push_byte_to_RPC(&instances[cp],0x81);
	CHECK_EQUAL(0,instances[cp].i.rxState.writePointer);
	channel_push_byte_to_RPC(&instances[cp],0xFF);
	channel_push_byte_to_RPC(&instances[cp],0xFF);
	channel_push_byte_to_RPC(&instances[cp],0xFF);
	CHECK_EQUAL(0,instances[cp].i.rxState.writePointer);
	channel_push_byte_to_RPC(&instances[cp],0x03);
	channel_push_byte_to_RPC(&instances[cp],0x80);
	channel_push_byte_to_RPC(&instances[cp],0x82);
	channel_push_byte_to_RPC(&instances[cp],0x82);

	CHECK_EQUAL(0x81,(unsigned char)instances[cp].i.rxState.buffer[0]);
#if CHANNEL_BLOCKLENGTH == 8
	CHECK_EQUAL(0x83,(unsigned char)instances[cp].i.rxState.buffer[1]);
	CHECK_EQUAL(0x82,(unsigned char)instances[cp].i.rxState.buffer[2]);
#elif CHANNEL_BLOCKLENGTH == 16
	CHECK_EQUAL(0x82,(unsigned char)instances[cp].i.rxState.buffer[1]);
	CHECK_EQUAL(0x83,(unsigned char)instances[cp].i.rxState.buffer[2]);
#endif
	CHECK_EQUAL(3,instances[cp].i.rxState.writePointer);
	mock().checkExpectations();

}
#endif

#if 1 && DISABLE_TESTS
TEST(channel_codec, channel_push_to_RPC_many_bytes_withour_crashing)
{
	const channel_codec_conf_comport_t cp = channel_codec_comport_uart;
	for(int i=0;i<256;i++) {
		channel_push_byte_to_RPC(&instances[cp],0x81);
	}
	CHECK_EQUAL(0,(unsigned char)instances[cp].i.rxState.writePointer);

}
#endif

#if 1 && DISABLE_TESTS
TEST(channel_codec, channel_push_to_RPC_bufsize)
{
	const channel_codec_conf_comport_t cp = channel_codec_comport_uart;
	mock().expectOneCall("RPC_parse_request").withParameter("firstByte", 10).withParameter("length", 10);
	channel_push_byte_to_RPC(&instances[cp],0xFF);
	channel_push_byte_to_RPC(&instances[cp],0xFF);
	channel_push_byte_to_RPC(&instances[cp],0xFF);

	channel_push_byte_to_RPC(&instances[cp],0x0E);
	channel_push_byte_to_RPC(&instances[cp],0x0A);
	//printf("%d %d\n",rxState.messageResult.result, rxState.messageResult.size);
	CHECK_EQUAL(RPC_COMMAND_INCOMPLETE,instances[cp].i.rxState.messageResult.result);
	channel_push_byte_to_RPC(&instances[cp],0x00);
	channel_push_byte_to_RPC(&instances[cp],0x00);
	channel_push_byte_to_RPC(&instances[cp],0x00);
	channel_push_byte_to_RPC(&instances[cp],0x00);
	channel_push_byte_to_RPC(&instances[cp],0x00);
	channel_push_byte_to_RPC(&instances[cp],0x00);
	CHECK_EQUAL(RPC_COMMAND_INCOMPLETE,instances[cp].i.rxState.messageResult.result);
	CHECK_EQUAL(10,instances[cp].i.rxState.messageResult.size);
#if CHANNEL_BLOCKLENGTH == 8
	channel_push_byte_to_RPC(0x0E);
#endif
	channel_push_byte_to_RPC(&instances[cp],0x00);
	channel_push_byte_to_RPC(&instances[cp],0x00);
	channel_push_byte_to_RPC(&instances[cp],0x00);

#if CHANNEL_BLOCKLENGTH == 8
	channel_push_byte_to_RPC(0x79);
	channel_push_byte_to_RPC(0x42);
#elif CHANNEL_BLOCKLENGTH==16
	//16ByteBlock:
	channel_push_byte_to_RPC(&instances[cp],0x3f);
	channel_push_byte_to_RPC(&instances[cp],0x01);
#endif
	CHECK_EQUAL(RPC_COMMAND_UNKNOWN,instances[cp].i.rxState.messageResult.result);
	CHECK_EQUAL(10,instances[cp].i.rxState.messageResult.size);
	mock().checkExpectations();
}
#endif

#if 1 && DISABLE_TESTS
TEST(channel_codec, channel_push_to_RPC_PingPong_10)
{
	#define RPC_IS_ANSWER 1
	#define RPC_IS_REQUEST 0
	const channel_codec_conf_comport_t cp = channel_codec_comport_uart;
	const uint8_t length = 10  | RPC_IS_REQUEST;
	uint8_t buf_loc[length+20];
	uint8_t msgLength = getMsgLength(length,CHANNEL_BLOCKLENGTH);
#if 0
	printf("msgLength = %d\n",msgLength);
#endif
	mock().expectOneCall("eunistonePushDataBuffer").withParameter("length", msgLength);
	mock().expectOneCall("RPC_parse_request").withParameter("firstByte", length).withParameter("length", length);
	channel_start_message_from_RPC(&instances[cp],length);
	for (uint8_t i=0;i<length;i++){
		uint8_t byte;
		byte = getByteSequence(0,i);
		if (i==0){
			byte = length;
		}
		channel_push_byte_from_RPC(&instances[cp],byte);
	}
	channel_commit_from_RPC(&instances[cp]);
#if 0
	printf("TXDataLength %d\n",TXDataLength);
#endif
	memcpy(buf_loc,instances[cp].i.txState.buffer,TXDataLength);
	for (uint8_t i=0;i<TXDataLength;i++){
		channel_push_byte_to_RPC(&instances[cp],buf_loc[i]);
	}
	for(int i = 0; i<length;i++){
		uint8_t byte;
		byte = getByteSequence(0,i);
		if (i==0){
			byte = length;
		}
		CHECK_EQUAL(byte,(unsigned char)instances[cp].i.rxState.buffer[i]);
	}
	mock().checkExpectations();
}
#endif


#if 1 && DISABLE_TESTS
TEST(channel_codec, channel_push_to_RPC_PingPong_50)
{
	#define RPC_IS_ANSWER 1
	#define RPC_IS_REQUEST 0
	const uint8_t length = 50 | RPC_IS_ANSWER;
	const channel_codec_conf_comport_t cp = channel_codec_comport_uart;
	uint8_t buf_loc[length+30];
	uint8_t msgLength = getMsgLength(length,CHANNEL_BLOCKLENGTH);
#if 0
	printf("msgLength = %d\n",msgLength);
#endif
	mock().expectOneCall("eunistonePushDataBuffer").withParameter("length", msgLength);
	mock().expectOneCall("RPC_parse_answer").withParameter("firstByte", 0xFF).withParameter("length", length);
	channel_start_message_from_RPC(&instances[cp],length);
	for (uint8_t i=0;i<length;i++){
		uint8_t byte;
		byte = getByteSequence(1,i);
		if (i<20){ //tests if a payload consisting of only ones triggers a preamble
			byte = 0xFF;
		}
		channel_push_byte_from_RPC(&instances[cp],byte);
	}
	channel_commit_from_RPC(&instances[cp]);
#if 0
	printf("TXDataLength %d\n",TXDataLength);
#endif
	memcpy(buf_loc,instances[cp].i.txState.buffer,TXDataLength);
	for (uint8_t i=0;i<TXDataLength;i++){
		channel_push_byte_to_RPC(&instances[cp],buf_loc[i]);
	}
	for(int i = 0; i<length;i++){
		uint8_t byte;
		byte = getByteSequence(1,i);
		if (i<20){
			byte = 0xFF;
		}
#if 0
		printf("%d\n",i);
#endif
		CHECK_EQUAL(byte,(unsigned char)instances[cp].i.rxState.buffer[i]);
	}
	mock().checkExpectations();
}
#endif
