/*
 * crc16.h
 *
 *  Created on: 30.03.2015
 *      Author: ak
 */

#ifndef CRC16_H_
#define CRC16_H_

#include <stdint.h>

uint16_t crc16_buffer(uint8_t* data_p, uint16_t length);
void crc16_online(uint8_t data, uint16_t *crc16);
void crc16_online_init(uint16_t *crc16);

#endif /* CRC16_H_ */
