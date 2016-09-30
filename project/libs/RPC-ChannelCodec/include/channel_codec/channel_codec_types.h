/*
 * channel_codec_types.h
 *
 *  Created on: 03.08.2016
 *      Author: ak
 */

#ifndef INCLUDE_CHANNEL_CODEC_CHANNEL_CODEC_TYPES_H_
#define INCLUDE_CHANNEL_CODEC_CHANNEL_CODEC_TYPES_H_

#define CHANNEL_CODEC_PREAMBLE_LENGTH 3

#include "channel_codec/channel_codec_config.h"


typedef enum{csNone,
	csFoundPreamble,
	csLoadingPayload,
	csPayloadComplete,
	csCRCAndPackageComplete} channel_codec_channel_state_t;

typedef struct{
	char *buffer;
	size_t bufferLength;
	uint16_t indexInBlock;
	uint16_t bitMaskPositionInBuffer;
	uint16_t writePointer;
	uint16_t crc16;
}channel_codec_txState_t;

typedef struct{
	char *buffer;
	size_t bufferLength;
	uint8_t indexInBlock;
	uint8_t bitmask;
	uint16_t writePointer;
	uint8_t preambleBuffer[CHANNEL_CODEC_PREAMBLE_LENGTH];
	RPC_SIZE_RESULT messageResult;
}channel_codec_rxState_t;

typedef struct{
	channel_codec_txState_t txState;
	channel_codec_rxState_t rxState;
	channel_codec_channel_state_t ccChannelState;
	bool initialized;
}channel_codec_rawinstance_t;

typedef struct{
	channel_codec_rawinstance_t i;
	channel_codec_conf_auxdata_t aux;
}channel_codec_instance_t;

#endif /* INCLUDE_CHANNEL_CODEC_CHANNEL_CODEC_TYPES_H_ */
