/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2022-10-27 18:10:07
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-05 16:27:27
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/Q10f/Q10f_gimbal_struct.h
 */
#ifndef Q10F_GIMBAL_STRUCT_H
#define Q10F_GIMBAL_STRUCT_H

#include <stdint.h>
namespace Q10f
{
#define Q10F_MAX_GIMBAL_PAYLOAD 64
#define Q10F_PAYLOAD_OFFSET 4
#define Q10F_SCALE_FACTOR_ANGLE 0.02197f
#define Q10F_SCALE_FACTOR_SPEED 0.06103f
#define Q10F_MAX_ZOOM 10.0f
#define Q10F_MAX_ZOOM_COUNT 0X4000

    typedef enum
    {
        GIMBAL_CMD_SET_POS = 0X100F01FF,
        GIMBAL_CMD_GET = 0X3D003D3E,
        GIMBAL_CMD_FOCUS = 0X08040181,
        GIMBAL_CMD_ZOOM = 0X07040181,
        GIMBAL_CMD_ZOOM_DIRECT = 0X47040181,
        GIMBAL_CMD_HOME = 0X010A0181,
        GIMBAL_CMD_CAMERA = 0X68040181,
        GIMBAL_CMD_RCV_STATE = 0X721A583E,
        GIMBAL_CMD_SET_FEEDBACK_L = 0X143055AA,
        GIMBAL_CMD_SET_FEEDBACK_H = 0X003155AA,
        GIMBAL_CMD_OPEN_FEEDBACK =0X3E003E3E,
        GIMBAL_CMD_CLOSE_FEEDBACK =0X3D003D3E,
    } GIMBAL_CMD_T;

    typedef enum
    {
        GIMBAL_CMD_POS_MODE_NO = 0X00,
        GIMBAL_CMD_POS_MODE_SPEED = 0X01,
        GIMBAL_CMD_POS_MODE_ANGLE = 0X02,
        GIMBAL_CMD_POS_MODE_ANGLE_SPEED = 0X03,
    } GIMBAL_CMD_POS_MODE_T;

    typedef enum
    {
        GIMBAL_CMD_ZOOM_IN = 0X27,
        GIMBAL_CMD_ZOOM_OUT = 0X37,
        GIMBAL_CMD_ZOOM_STOP = 0X00,
    } GIMBAL_CMD_ZOOM_T;

    typedef enum
    {
        GIMBAL_SERIAL_STATE_IDLE,
        GIMBAL_SERIAL_STATE_HEAD1,
        GIMBAL_SERIAL_STATE_HEAD2,
        GIMBAL_SERIAL_STATE_HEAD3,
        GIMBAL_SERIAL_STATE_DATE,
        GIMBAL_SERIAL_STATE_CHECK
    } GIMBAL_SERIAL_STATE_T;

#pragma pack(1)
    typedef struct
    {
        uint32_t head;
        uint8_t data[Q10F_MAX_GIMBAL_PAYLOAD];
        uint8_t checkSum;
        uint8_t len;
    } GIMBAL_FRAME_T;

    typedef struct
    {
        uint8_t modeR;
        uint8_t modeP;
        uint8_t modeY;
        int16_t speedR;
        int16_t angleR;
        int16_t speedP;
        int16_t angleP;
        int16_t speedY;
        int16_t angleY;
    } GIMBAL_SET_POS_MSG_T;

    typedef struct
    {
        uint16_t timeStamp;
        int16_t rollIMUAngle;
        int16_t pitchIMUAngle;
        int16_t yawIMUAngle;
        int16_t rollTAGAngle;
        int16_t pitchTAGAngle;
        int16_t yawTAGAngle;
        int16_t rollTAGSpeed;
        int16_t pitchTAGSpeed;
        int16_t yawTAGSpeed;
        int16_t rollStatorRotorAngle;
        int16_t pitchStatorRotorAngle;
        int16_t yawStatorRotorAngle;
    } GIMBAL_RCV_POS_MSG_T;

#pragma pack()

}
#endif