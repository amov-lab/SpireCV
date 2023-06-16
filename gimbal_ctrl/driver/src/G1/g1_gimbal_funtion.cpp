/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-03-02 10:00:52
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-03-17 18:29:33
 * @FilePath: \gimbal-sdk-multi-platform\src\G1\g1_gimbal_funtion.cpp
 */
#include "g1_gimbal_driver.h"
#include "g1_gimbal_crc32.h"
#include "string.h"

/**
 * It sets the gimbal position.
 *
 * @param pos the position of the gimbal
 *
 * @return The return value is the number of bytes written to the buffer.
 */
uint32_t g1GimbalDriver::setGimabalPos(const amovGimbal::AMOV_GIMBAL_POS_T &pos)
{
    G1::GIMBAL_SET_POS_MSG_T temp;
    temp.mode = G1::GIMBAL_CMD_POS_MODE_ANGLE;
    temp.angle_pitch = pos.pitch / G1_SCALE_FACTOR;
    temp.angle_roll = pos.roll / G1_SCALE_FACTOR;
    temp.angle_yaw = pos.yaw / G1_SCALE_FACTOR;
    temp.speed_pitch = state.maxFollow.pitch;
    temp.speed_roll = state.maxFollow.roll;
    temp.speed_yaw = state.maxFollow.yaw;
    return pack(G1::GIMBAL_CMD_SET_POS, reinterpret_cast<uint8_t *>(&temp), sizeof(G1::GIMBAL_SET_POS_MSG_T));
}

/**
 * It takes a struct of type amovGimbal::AMOV_GIMBAL_POS_T and converts it to a struct of type
 * G1::GIMBAL_SET_POS_MSG_T
 *
 * @param speed the speed of the gimbal
 *
 * @return The return value is the number of bytes written to the buffer.
 */
uint32_t g1GimbalDriver::setGimabalSpeed(const amovGimbal::AMOV_GIMBAL_POS_T &speed)
{
    G1::GIMBAL_SET_POS_MSG_T temp;
    temp.mode = G1::GIMBAL_CMD_POS_MODE_SPEED;
    temp.angle_pitch = 0;
    temp.angle_roll = 0;
    temp.angle_yaw = 0;
    temp.speed_pitch = speed.pitch / G1_SCALE_FACTOR;
    temp.speed_roll = speed.roll / G1_SCALE_FACTOR;
    temp.speed_yaw = speed.yaw / G1_SCALE_FACTOR;
    return pack(G1::GIMBAL_CMD_SET_POS, reinterpret_cast<uint8_t *>(&temp), sizeof(G1::GIMBAL_SET_POS_MSG_T));
}

/**
 * This function sets the gimbal's follow speed
 *
 * @param followSpeed the speed of the gimbal
 *
 * @return The return value is the number of bytes written to the buffer.
 */
uint32_t g1GimbalDriver::setGimabalFollowSpeed(const amovGimbal::AMOV_GIMBAL_POS_T &followSpeed)
{
    state.maxFollow.pitch = followSpeed.pitch / G1_SCALE_FACTOR;
    state.maxFollow.roll = followSpeed.roll / G1_SCALE_FACTOR;
    state.maxFollow.yaw = followSpeed.yaw / G1_SCALE_FACTOR;
    return 0;
}

/**
 * This function sets the gimbal to its home position
 *
 * @return The return value is the number of bytes written to the buffer.
 */
uint32_t g1GimbalDriver::setGimabalHome(void)
{
    G1::GIMBAL_SET_POS_MSG_T temp;
    temp.mode = G1::GIMBAL_CMD_POS_MODE_HOME;
    temp.speed_pitch = state.maxFollow.pitch;
    temp.speed_roll = state.maxFollow.roll;
    temp.speed_yaw = state.maxFollow.yaw;
    return pack(G1::GIMBAL_CMD_SET_POS, reinterpret_cast<uint8_t *>(&temp), sizeof(G1::GIMBAL_SET_POS_MSG_T));
}

/**
 * It takes a picture.
 * 
 * @return The return value is the number of bytes written to the serial port.
 */
uint32_t g1GimbalDriver::takePic(void)
{
    uint8_t temp = G1::GIMBAL_CMD_CAMERA_TACK;
    return pack(G1::GIMBAL_CMD_CAMERA, &temp, sizeof(uint8_t));
}

/**
 * The function sets the video state of the gimbal
 * 
 * @param newState The new state of the video.
 * 
 * @return The return value is the number of bytes written to the serial port.
 */
uint32_t g1GimbalDriver::setVideo(const amovGimbal::AMOV_GIMBAL_VIDEO_T newState)
{
    uint8_t temp = G1::GIMBAL_CMD_CAMERA_REC;

    mState.lock();
    if(state.video == amovGimbal::AMOV_GIMBAL_VIDEO_TAKE)
    {
        state.video = amovGimbal::AMOV_GIMBAL_VIDEO_OFF;
    }
    else
    {
        state.video = amovGimbal::AMOV_GIMBAL_VIDEO_TAKE;
    }
    mState.unlock();

    return pack(G1::GIMBAL_CMD_CAMERA, &temp, sizeof(uint8_t));
}
