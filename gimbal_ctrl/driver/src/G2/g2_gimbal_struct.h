/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-03-01 09:21:57
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-04-18 10:13:23
 * @FilePath: /gimbal-sdk-multi-platform/src/G2/g2_gimbal_struct.h
 */

#ifndef G2_GIMBAL_STRUCT_H
#define G2_GIMBAL_STRUCT_H

#include <stdint.h>

namespace G2
{

#define G2_MAX_GIMBAL_PAYLOAD 64
#define G2_PAYLOAD_OFFSET 6
#define G2_SCALE_FACTOR 0.01f
#define G2_SERIAL_HEAD 0XAF
#define G2_SERIAL_VERSION 0X02

    typedef enum
    {
        IAP_COMMAND_JUMP = 80,
        IAP_COMMAND_FLASH_ERASE,
        IAP_COMMAND_BOLCK_INFO,
        IAP_COMMAND_BLOCK_WRITE,
        IAP_COMMAND_SOFT_INFO,
        IAP_COMMAND_HARDWARE_INFO,
        IAP_COMMAND_BLOCK_START,
        IAP_COMMAND_BLOCK_END = 117,
    } GIMBAL_CMD_T;

    typedef enum
    {
        IAP_STATE_FAILD = 0,
        IAP_STATE_SUCCEED,
        IAP_STATE_READY,
        IAP_STATE_FIRMWARE_BROKE,
        IAP_STATE_JUMP_FAILD,
        IAP_STATE_ADDR_ERR,
        IAP_STATE_CRC_ERR,
        IAP_STATE_WRITE_ERR,
        IAP_STATE_WRITE_TIMEOUT,
    } GIMBAL_IAP_STATE_T;

    typedef enum
    {
        GIMBAL_SERIAL_STATE_IDEL = 0,
        GIMBAL_SERIAL_STATE_HEAD_RCV,
        GIMBAL_SERIAL_STATE_VERSION_RCV,
        GIMBAL_SERIAL_STATE_TARGET_RCV,
        GIMBAL_SERIAL_STATE_SOURCE_RCV,
        GIMBAL_SERIAL_STATE_LENGHT_RCV,
        GIMBAL_SERIAL_STATE_DATA_RCV,
        GIMBAL_SERIAL_STATE_CRC_RCV1,
        GIMBAL_SERIAL_STATE_END,
    } GIMBAL_CMD_PARSER_STATE_T;

#pragma pack(1)
    typedef struct
    {
        uint8_t head;
        uint8_t version;
        uint8_t target;
        uint8_t source;
        uint8_t len;
        uint8_t command;
        uint8_t data[G2_MAX_GIMBAL_PAYLOAD];
        union
        {
            uint8_t f8[2];
            uint16_t f16;
        } crc;
    } GIMBAL_FRAME_T;
#pragma pack(0)

}
#endif