/*
 * @Description: Common Data Structures of gimbal
 * @Author: L LC @amov
 * @Date: 2022-10-31 11:56:43
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-05 17:03:02
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/amovGimbal/amov_gimbal_struct.h
 */

#include <stdint.h>

#ifndef __AMOV_GIMABL_STRUCT_H
#define __AMOV_GIMABL_STRUCT_H

typedef void (*pAmovGimbalStateInvoke)(double frameAngleRoll, double frameAnglePitch, double frameAngleYaw,
                                       double imuAngleRoll, double imuAnglePitch, double imuAngleYaw,
                                       double fovX, double fovY, void *caller);
typedef void (*pAmovGimbalMsgInvoke)(void *msg, void *caller);
typedef uint32_t (*pAmovGimbalInputBytesInvoke)(uint8_t *pData, void *caller);
typedef uint32_t (*pAmovGimbalOutputBytesInvoke)(uint8_t *pData, uint32_t len, void *caller);

typedef enum
{
    AMOV_GIMBAL_SERVO_MODE_FPV = 0X10,
    AMOV_GIMBAL_SERVO_MODE_LOCK = 0X11,
    AMOV_GIMBAL_SERVO_MODE_FOLLOW = 0X12,
    AMOV_GIMBAL_SERVO_MODE_OVERLOOK = 0X13,
    AMOV_GIMBAL_SERVO_MODE_EULER = 0X14,
    AMOV_GIMBAL_SERVO_MODE_WATCH = 0X16,
    AMOV_GIMBAL_SERVO_MODE_TRACK = 0X17,
} AMOV_GIMBAL_SERVO_MODE_T;

typedef enum
{
    AMOV_GIMBAL_CAMERA_FLAG_INVERSION = 0X1000,
    AMOV_GIMBAL_CAMERA_FLAG_IR = 0X0200,
    AMOV_GIMBAL_CAMERA_FLAG_RF = 0X0100,
    AMOV_GIMBAL_CAMERA_FLAG_LOCK = 0X0001,
} AMOV_GIMBAL_CAMERA_FLAG_T;

typedef enum
{
    AMOV_GIMBAL_VIDEO_TAKE,
    AMOV_GIMBAL_VIDEO_OFF
} AMOV_GIMBAL_VIDEO_T;

typedef enum
{
    AMOV_GIMBAL_ZOOM_IN,
    AMOV_GIMBAL_ZOOM_OUT,
    AMOV_GIMBAL_ZOOM_STOP
} AMOV_GIMBAL_ZOOM_T;

typedef struct
{
    double yaw;
    double roll;
    double pitch;
} AMOV_GIMBAL_POS_T;

typedef struct
{
    double x;
    double y;
} AMOV_GIMBAL_FOV_T;

typedef struct
{
    AMOV_GIMBAL_SERVO_MODE_T workMode;
    AMOV_GIMBAL_CAMERA_FLAG_T cameraFlag;
    AMOV_GIMBAL_VIDEO_T video;
    AMOV_GIMBAL_POS_T abs;
    AMOV_GIMBAL_POS_T rel;
    AMOV_GIMBAL_POS_T relSpeed;
    AMOV_GIMBAL_POS_T maxFollow;
    AMOV_GIMBAL_FOV_T fov;
} AMOV_GIMBAL_STATE_T;

typedef struct
{
    uint32_t centreX;
    uint32_t centreY;
    uint32_t hight;
    uint32_t width;
} AMOV_GIMBAL_ROI_T;

typedef struct
{
    double q0;
    double q1;
    double q2;
    double q3;
} AMOV_GIMBAL_QUATERNION_T;

typedef struct
{
    double x; // or N
    double y; // or E
    double z; // or UP
} AMOV_GIMBAL_VELOCITY_T;

#endif
