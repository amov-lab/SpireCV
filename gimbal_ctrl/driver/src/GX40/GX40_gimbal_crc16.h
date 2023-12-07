/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-10-20 16:33:07
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-05 16:29:39
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/GX40/GX40_gimbal_crc16.h
 */
#ifndef GX40_GIMBAL_CRC16_H
#define GX40_GIMBAL_CRC16_H

#include <stdint.h>

namespace GX40
{
    const static uint16_t crc16Tab[16] = {
        0x0000, 0x1021, 0x2042, 0x3063,
        0x4084, 0x50a5, 0x60c6, 0x70e7,
        0x8108, 0x9129, 0xa14a, 0xb16b,
        0xc18c, 0xd1ad, 0xe1ce, 0xf1ef};

    static inline uint16_t CalculateCrc16(const uint8_t *ptr, uint8_t len)
    {
        uint16_t crc = 0;
        uint8_t temp;

        while (len-- != 0)
        {
            temp = crc >> 12;
            crc <<= 4;
            crc ^= crc16Tab[temp ^ (*ptr >> 4)];
            temp = crc >> 12;
            crc <<= 4;
            crc ^= crc16Tab[temp ^ (*ptr & 0x0F)];
            ptr++;
        }
        crc = (crc >> 8) | (crc << 8);
        return (crc);
    }
}
#endif