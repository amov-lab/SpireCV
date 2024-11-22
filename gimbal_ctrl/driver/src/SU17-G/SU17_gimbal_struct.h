/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-03-01 09:21:57
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-04-18 10:13:23
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/SU17-G/SU17_gimbal_struct.h
 */

#ifndef SU17_G_GIMBAL_STRUCT_H
#define SU17_G_GIMBAL_STRUCT_H

#include <stdint.h>

namespace SU17
{

#define SU17_MAX_GIMBAL_PAYLOAD 64
#define SU17_PAYLOAD_OFFSET 6
#define SU17_SCALE_FACTOR 0.01f
#define SU17_SERIAL_HEAD 0XAF
#define SU17_SERIAL_VERSION 0X02

    typedef enum
    {
        GIMBAL_CMD_HEART = 20,
        GIMBAL_CMD_ATT = 160,
        GIMBAL_CMD_SET_POS = 162,
        GIMBAL_CMD_TAKEPIC = 180,
        GIMBAL_CMD__VIDEO = 181
    } GIMBAL_CMD_T;

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

    typedef enum
    {
        GIMBAL_CMD_MODE_POS = 0X01,
        GIMBAL_CMD_MODE_SPEED = 0X02,
        GIMBAL_CMD_MODE_MIX = 0X03
    } GIMBAL_CMD_MODE_T;

#pragma pack(1)
    typedef struct
    {
        int16_t roll;
        int16_t pitch;
        int16_t yaw;
        int16_t rollSpeed;
        int16_t pitchSpeed;
        int16_t yawSpeed;
    } GIMBAL_ATT_T;

    typedef struct
    {
        uint8_t mode;
        GIMBAL_ATT_T att;
    } GIMBAL_SET_ATT_T;

    typedef struct
    {
        uint8_t head;
        uint8_t version;
        uint8_t target;
        uint8_t source;
        uint8_t len;
        uint8_t command;
        uint8_t data[SU17_MAX_GIMBAL_PAYLOAD];
        union
        {
            uint8_t f8[2];
            uint16_t f16;
        } crc;
    } GIMBAL_FRAME_T;
#pragma pack(0)

}
#endif