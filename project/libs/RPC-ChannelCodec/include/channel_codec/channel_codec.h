/*
 * channel_codec.h
 *
 *  Created on: 30.03.2015
 *      Author: ak
 */

#ifndef CHANNELCODEC_CHANNELCODEC_H_
#define CHANNELCODEC_CHANNELCODEC_H_

#ifdef __cplusplus
extern "C" {
#endif



#include <stdint.h>


#include "channel_codec/channel_codec_types.h"
#include "rpc_transmission/client/generated_general/RPC_types.h"


#define CHANNEL_BLOCKLENGTH 16

#if 0
typedef void (*RPC_parse_request_t)(const void *buffer, size_t size);
typedef void (*RPC_parse_answer_t)(const void *buffer, size_t size);
typedef RPC_SIZE_RESULT (*RPC_get_request_size_t)(const void *buffer, size_t size_bytes);
typedef RPC_SIZE_RESULT (*RPC_get_answer_length_t)(const void *buffer, size_t size_bytes);
typedef void (*RPC_Parser_init_t)(void);
#endif




bool channel_is_initialized(channel_codec_instance_t *instance);
void channel_init_instance(channel_codec_instance_t *instance, char *rxBuffer, size_t rxBufferLength, char *txBuffer, size_t txBufferLength);
void channel_uninit_instance(channel_codec_instance_t *instance);

void channel_start_message_from_RPC(channel_codec_instance_t *instance, size_t size);
void channel_push_byte_from_RPC(channel_codec_instance_t *instance, unsigned char byte);
RPC_RESULT channel_commit_from_RPC(channel_codec_instance_t *instance );

void channel_push_byte_to_RPC(channel_codec_instance_t *instance, unsigned char byte);


RPC_SIZE_RESULT RPC_CHANNEL_CODEC_get_request_size(channel_codec_instance_t *instance, const void *buffer, size_t size_bytes);
RPC_SIZE_RESULT RPC_CHANNEL_CODEC_get_answer_length(channel_codec_instance_t *instance, const void *buffer, size_t size_bytes);

void RPC_CHANNEL_CODEC_parse_request(channel_codec_instance_t *instance, const void *buffer, size_t size_bytes);
void RPC_CHANNEL_CODEC_parse_answer(channel_codec_instance_t *instance, const void *buffer, size_t size_bytes);
void RPC_CHANNEL_CODEC_parser_init(channel_codec_instance_t *instance);

#ifdef __cplusplus
}
#endif

#endif /* CHANNELCODEC_CHANNELCODEC_H_ */
