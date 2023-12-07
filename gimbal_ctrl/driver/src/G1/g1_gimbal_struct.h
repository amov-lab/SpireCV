/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2022-10-27 18:10:07
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-05 16:29:48
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/G1/g1_gimbal_struct.h
 */
#ifndef G1_GIMBAL_STRUCT_H
#define G1_GIMBAL_STRUCT_H

#include <stdint.h>
namespace G1
{
#define G1_MAX_GIMBAL_PAYLOAD 256
#define G1_PAYLOAD_OFFSET 5
#define G1_SCALE_FACTOR 0.01f
#define G1_SERIAL_HEAD 0XAE
#define G1_SERIAL_VERSION 0X01

    typedef enum
    {
        GIMBAL_CMD_SET_STATE = 0X01,
        GIMBAL_CMD_SET_POS = 0X85,
        GIMBAL_CMD_CAMERA = 0X86,
        GIMBAL_CMD_RCV_POS = 0X87
    } GIMBAL_CMD_T;

    typedef enum
    {
        GIMBAL_CMD_POS_MODE_SPEED = 1,
        GIMBAL_CMD_POS_MODE_ANGLE = 2,
        GIMBAL_CMD_POS_MODE_HOME = 3
    } GIMBAL_CMD_POS_MODE_T;

    typedef enum
    {
        GIMBAL_CMD_CAMERA_REC = 1,
        GIMBAL_CMD_CAMERA_TACK = 2
    } GIMBAL_CMD_CAMERA_T;

    typedef enum
    {
        GIMBAL_SERIAL_STATE_IDLE,
        GIMBAL_SERIAL_STATE_VERSION,
        GIMBAL_SERIAL_STATE_LENGHT,
        GIMBAL_SERIAL_STATE_CMD,
        GIMBAL_SERIAL_STATE_CHECK,
        GIMBAL_SERIAL_STATE_PAYLOAD,
    } GIMBAL_CMD_PARSER_STATE_T;

#pragma pack(1)
    typedef struct
    {
        uint8_t head;
        uint8_t version;
        uint8_t lenght;
        uint8_t cmd;
        uint8_t checksum;
        uint8_t payload[G1_MAX_GIMBAL_PAYLOAD + sizeof(uint32_t)];
        union
        {
            uint8_t u8[4];
            uint32_t u32;
        } crc;
    } GIMBAL_FRAME_T;

    typedef struct
    {
        uint8_t mode;
        int16_t angle_roll;
        int16_t angle_pitch;
        int16_t angle_yaw;
        int16_t speed_roll;
        int16_t speed_pitch;
        int16_t speed_yaw;
    } GIMBAL_SET_POS_MSG_T;

    typedef struct
    {
        int16_t IMU_roll;
        int16_t IMU_pitch;
        int16_t IMU_yaw;
        int16_t HALL_roll;
        int16_t HALL_pitch;
        int16_t HALL_yaw;
    } GIMBAL_RCV_POS_MSG_T;

    typedef struct
    {
        float q[4];
        float acc[3];
        float yawSetPoint;
        float yawSpeedSetPoint;
    } GIMBAL_ATT_CORR_MSG_T;


    typedef struct 
    {
        uint8_t cmd;
        uint8_t data[256];
        uint8_t len;
    }GIMBAL_STD_MSG_T;
    
#pragma pack()

}
#endif