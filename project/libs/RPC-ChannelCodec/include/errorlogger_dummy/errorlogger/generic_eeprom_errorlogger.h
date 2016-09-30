/*
 * generic_config.h
 *
 *  Created on: 05.01.2016
 *      Author: ak
 */

#ifndef MODULES_ERRORLOGGER_INCLUDE_ERRORLOGGER_GENERIC_EEPROM_ERRORLOGGER_H_
#define MODULES_ERRORLOGGER_INCLUDE_ERRORLOGGER_GENERIC_EEPROM_ERRORLOGGER_H_

#include <stdint.h>
#include <assert.h>
#include "errlog_trace.h"


#ifdef __cplusplus
extern "C" {
#endif

#include "channel_codec/channel_codec_types.h"

typedef enum {
    errlog_E_CHCODEC_RPC_parse_answer_request_Fail,
    errlog_E_CHCODEC_exceeding_RPC_TX_Buffer,
	errlog_E_CHCODEC_RPC_Functions_uninitialized,
	errlog_E_CHCODEC_instance_index_beyond_max,
	errlog_E_MUTEX_mutex_could_not_be_locked_within_timeout,
	errlog_W_CHCODEC_RX_CRC_fail,
    errlog_W_CHCODEC_exceeding_RPC_RX_buffer

} channelCodecErrorNum_t;


void ChannelCodec_errorHandler(channel_codec_instance_t *instance, channelCodecErrorNum_t ErrNum);

#define GEN_ASSERT(EX, ERRNUMBER, ...); {TRACE_ERROR(__VA_ARGS__); assert(EX);}
#define GEN_ASSERT_NO_LOGGING(EX, ERRNUMBER, ...); {TRACE_ERROR(__VA_ARGS__); assert(EX);}
#ifdef __ASSERT_FUNC
#define GEN_WARNING_CC(INSTANCE, ERRNUMBER, ...); {TRACE_WARNING(__VA_ARGS__); ChannelCodec_errorHandler(INSTANCE, ERRNUMBER); printf("\nwarning at file: %s line: %d and function: %s\n",__FILE__, __LINE__,__ASSERT_FUNC);}
#else
#define GEN_WARNING_CC(INSTANCE, ERRNUMBER, ...); {TRACE_WARNING(__VA_ARGS__); ChannelCodec_errorHandler(INSTANCE, ERRNUMBER); printf("\nwarning at file: %s line: %d\n",__FILE__, __LINE__);}
#endif


#ifdef __cplusplus
}
#endif

#endif /* MODULES_ERRORLOGGER_INCLUDE_ERRORLOGGER_GENERIC_EEPROM_ERRORLOGGER_H_ */
