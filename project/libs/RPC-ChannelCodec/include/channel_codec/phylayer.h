#ifndef PHYLAYER_H_
#define PHYLAYER_H_



#include "channel_codec/channel_codec_types.h"

RPC_RESULT phyPushDataBuffer(channel_codec_instance_t *instance, const char *buffer, size_t length);

#endif
