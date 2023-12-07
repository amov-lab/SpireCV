/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-10-20 16:08:13
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-05 16:28:54
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/GX40/GX40_gimbal_struct.h
 */
#ifndef GX40_GIMBAL_STRUCT_H
#define GX40_GIMBAL_STRUCT_H

#include <stdint.h>

namespace GX40
{
#define XF_SEND_HEAD 0XE5A8
#define XF_RCV_HEAD 0X8A5E
#define XF_VERSION 0X00
#define XF_ANGLE_DPI 0.01f

    typedef enum
    {
        GIMBAL_FRAME_PARSER_STATE_IDLE,
        GIMBAL_FRAME_PARSER_STATE_HEAD,
        GIMBAL_FRAME_PARSER_STATE_LEN1,
        GIMBAL_FRAME_PARSER_STATE_LEN2,
        GIMBAL_FRAME_PARSER_STATE_VERSION,
        GIMBAL_FRAME_PARSER_STATE_PAYLOAD,
    } GIMBAL_FRAME_PARSER_STATE_T;

    typedef enum
    {
        GIMBAL_CMD_NOP = 0X00,
        GIMBAL_CMD_CAL = 0X01,
        GIMBAL_CMD_HOME = 0X03,
        GIMBAL_CMD_MODE_FPV = 0X10,
        GIMBAL_CMD_MODE_LOCK = 0X11,
        GIMBAL_CMD_MODE_FOLLOW = 0X12,
        GIMBAL_CMD_MODE_OVERLOCK = 0X13,
        GIMBAL_CMD_MODE_EULER = 0X14,
        GIMBAL_CMD_MODE_WATCH_POS = 0X15,
        GIMBAL_CMD_MODE_WATCH = 0X16,
        GIMBAL_CMD_MODE_TRACK = 0X17,
        GIMBAL_CMD_MODE_MOVE = 0X1A,
        GIMBAL_CMD_MODE_MOVE_TRACK = 0X1B,
        GIMBAL_CMD_TAKEPIC = 0X20,
        GIMBAL_CMD_TAKEVIDEO = 0X21,
        GIMBAL_CMD_ZOOM_OUT = 0X22,
        GIMBAL_CMD_ZOMM_IN = 0X23,
        GIMBAL_CMD_ZOOM_STOP = 0X24,
        GIMBAL_CMD_ZOOM = 0X25,
        GIMBAL_CMD_FOCUE = 0X26,
        GIMBAL_CMD_VIDEO_MODE = 0X2A,
        GIMBAL_CMD_NIGHT = 0X2B,
        GIMBAL_CMD_OSD = 0X73,
        GIMBAL_CMD_FIX_MODE = 0X74,
        GIMBAL_CMD_LIGHT = 0X80,
        GIMBAL_CMD_TAKE_DISTANCE = 0X81,
    } GIMBAL_CMD_T;

#pragma pack(1)

    typedef struct
    {
        union
        {
            uint8_t u8[2];
            uint16_t u16;
        } head;
        union
        {
            uint8_t u8[2];
            uint16_t u16;
        } lenght;
        uint8_t version;
        uint8_t primaryData[32];
        uint8_t secondaryData[32];
        uint8_t otherData[32];
        union
        {
            uint8_t u8[2];
            uint16_t u16;
        } crc16;
    } GIMBAL_FRAME_T;

    typedef struct
    {
        int16_t roll;
        int16_t pitch;
        int16_t yaw;
        uint8_t state;
        int16_t selfRoll;
        int16_t selfPitch;
        uint16_t selfYaw;
        int16_t accN;
        int16_t accE;
        int16_t accUp;
        int16_t speedN;
        int16_t speedE;
        int16_t speedUp;
        uint8_t secondaryFlag;
        uint8_t reserve[6];
    } GIMBAL_PRIMARY_MASTER_FRAME_T;

    typedef struct
    {
        uint8_t workMode;
        uint16_t state;
        int16_t offsetX;
        int16_t offsetY;
        uint16_t motorRoll;
        uint16_t motorPitch;
        uint16_t motorYaw;
        int16_t roll;
        int16_t pitch;
        uint16_t yaw;
        int16_t speedRoll;
        int16_t speedPitch;
        int16_t speedYaw;
        uint8_t reserve[7];
    } GIMBAL_PRIMARY_SLAVE_FRAME_T;

    typedef struct
    {
        uint8_t head;
        int32_t lng;
        int32_t lat;
        int32_t alt;
        uint8_t nState;
        uint32_t GPSms;
        int32_t GPSweeks;
        int32_t relAlt;
        uint8_t reserve[8];
    } GIMBAL_SECONDARY_MASTER_FRAME_T;

    typedef struct
    {
        uint8_t head;
        uint8_t versionHW;
        uint8_t versionSoft;
        uint8_t type;
        uint16_t error;
        int32_t targetDistance;
        int32_t targetLng;
        int32_t targetLat;
        int32_t targetAlt;
        uint16_t camera1Zoom;
        uint16_t camera2Zoom;
        uint8_t reserve[6];
    } GIMBAL_SECONDARY_SLAVE_FRAME_T;
#pragma pack()
}

#endif
