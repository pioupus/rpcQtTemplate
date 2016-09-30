/*
 * crc16.c
 *
 *  Created on: 30.03.2015
 *      Author: ak
 */

#include "channel_codec/crc16.h"

uint16_t crc16_buffer(uint8_t* data_p, uint16_t length){
	uint8_t x;
    uint16_t crc = 0xFFFF;

    while (length--){
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
    }
    return crc;
}

void crc16_online(uint8_t data, uint16_t *crc16){
	uint8_t x;

    x = *crc16 >> 8 ^ data;
    x ^= x>>4;
    *crc16 = (*crc16 << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
}


void crc16_online_init(uint16_t *crc16){
    *crc16 = 0xFFFF;
}
