/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2022-10-27 18:10:07
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-08-25 19:32:59
 * @FilePath: /gimbal-sdk-multi-platform/src/AT10/AT10_gimbal_struct.h
 */
#ifndef AT10_GIMBAL_STRUCT_H
#define AT10_GIMBAL_STRUCT_H

#include <stdint.h>
namespace AT10
{
#define AT10_MAX_GIMBAL_PAYLOAD 64
#define AT10_EXT_PAYLOAD_OFFSET 4
#define AT10_STD_PAYLOAD_OFFSET 6
#define AT10_EXT_SCALE_FACTOR_ANGLE 0.02197f
#define AT10_EXT_SCALE_FACTOR_SPEED 0.06103f

    typedef enum
    {
        GIMBAL_CMD_STD_NOP = 0X00,
        GIMBAL_CMD_STD_HEART = 0X10,
        GIMBAL_CMD_STD_RCV_STATE = 0X40,
        GIMBAL_CMD_STD_MOTOR = 0X1A,
        GIMBAL_CMD_STD_CAMERA = 0X1C,
        GIMBAL_CMD_STD_CAMERA2 = 0X2C,
        GIMBAL_CMD_STD_MOTOR2 = 0X32,
        GIMBAL_CMD_STD = 0XDCAA5500,
        GIMBAL_CMD_RCV_STATE = 0X721A583E,
        GIMBAL_CMD_SET_FEEDBACK_L = 0X143055AA,
        GIMBAL_CMD_SET_FEEDBACK_H = 0X003155AA,
        GIMBAL_CMD_OPEN_FEEDBACK = 0X3E003E3E,
        GIMBAL_CMD_CLOSE_FEEDBACK = 0X3D003D3E,
    } GIMBAL_CMD_T;

    typedef enum
    {
        GIMBAL_SERIAL_STATE_IDLE,
        GIMBAL_SERIAL_STATE_EXT_HEAD1,
        GIMBAL_SERIAL_STATE_EXT_HEAD2,
        GIMBAL_SERIAL_STATE_EXT_HEAD3,
        GIMBAL_SERIAL_STATE_EXT_DATE,
        GIMBAL_SERIAL_STATE_EXT_CHECK,
        GIMBAL_SERIAL_STATE_STD_HAED1,
        GIMBAL_SERIAL_STATE_STD_HAED2,
        GIMBAL_SERIAL_STATE_STD_LEN,
        GIMBAL_SERIAL_STATE_STD_CMD,
        GIMBAL_SERIAL_STATE_STD_DATE,
        GIMBAL_SERIAL_STATE_STD_CHECK,
    } GIMBAL_SERIAL_STATE_T;

#pragma pack(1)
    typedef struct
    {
        uint8_t cmd;
        uint8_t param[AT10_MAX_GIMBAL_PAYLOAD];
        uint8_t paramLen;
    } AT10_EXT_CMD_T;

    typedef struct
    {
        uint32_t head;
        uint8_t len;
        uint8_t cmd;
        uint8_t data[AT10_MAX_GIMBAL_PAYLOAD];
        uint8_t checkXOR;
    } GIMBAL_STD_FRAME_T;

    typedef struct
    {
        uint32_t head;
        uint8_t data[AT10_MAX_GIMBAL_PAYLOAD];
        uint8_t checkSum;
        uint8_t len;
    } GIMBAL_EXTEND_FRAME_T;

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

    typedef struct
    {
        uint8_t hight;
        uint8_t reserve;
        uint32_t lat;
        uint32_t log;
        int16_t alt;
        uint32_t latTar;
        uint32_t logTar;
        int16_t altTar;
    } GIMBAL_RCV_GPS_STATE_MSG_T;

    typedef struct
    {
        int16_t roll;
        int16_t yaw;
        int16_t pitch;
    } GIMBAL_RCV_MOTOR_STATE_MSG_T;

    typedef struct
    {
        uint8_t mode;
        uint8_t reserve;
        uint16_t camera;
        uint16_t distance;
        uint16_t fovY;
        uint16_t fovX;
        uint16_t rate;
    } GIMBAL_RCV_CAMERA_STATE_MSG_T;

    typedef struct
    {
        GIMBAL_RCV_GPS_STATE_MSG_T T1;
        uint8_t F1;
        GIMBAL_RCV_MOTOR_STATE_MSG_T B1;
        GIMBAL_RCV_CAMERA_STATE_MSG_T D1;
    } GIMBAL_RCV_STD_STATE_MSG_T;

    typedef struct
    {
        uint16_t param;
    } GIMBAL_CMD_C1_MSG_T;

    typedef struct
    {
        uint8_t cmd;
        uint16_t param;
    } GIMBAL_CMD_C2_MSG_T;

    typedef struct
    {
        uint8_t cmd;
        uint16_t param[4];
    } GIMBAL_CMD_A1_MSG_T;

    typedef struct
    {
        uint8_t cmd;
        uint8_t reserve;
        uint32_t param[3];
    } GIMBAL_CMD_S1_MSG_T;

    typedef struct
    {
        uint8_t param[3];
    } GIMBAL_CMD_E1_MSG_T;

    typedef struct
    {
        GIMBAL_CMD_A1_MSG_T a1;
        GIMBAL_CMD_C1_MSG_T c1;
        GIMBAL_CMD_E1_MSG_T e1;
        GIMBAL_CMD_S1_MSG_T s1;
    } GIMBAL_CMD_A1C1E1S1_MSG_T;

#pragma pack()

}
#endif
