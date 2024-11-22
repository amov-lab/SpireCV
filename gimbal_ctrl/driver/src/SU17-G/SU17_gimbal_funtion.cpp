/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-03-13 11:58:54
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-03-13 12:31:58
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/SU17-G/SU17_gimbal_funtion.cpp
 */
#include "SU17_gimbal_driver.h"
#include "SU17_gimbal_crc.h"
#include "string.h"

/**
 * It sets the gimbal position.
 *
 * @param pos the position of the gimbal
 *
 * @return The return value is the number of bytes written to the buffer.
 */
uint32_t su17GimbalDriver::setGimabalPos(const AMOV_GIMBAL_POS_T &pos)
{
    SU17::GIMBAL_SET_ATT_T temp;

    temp.mode = SU17::GIMBAL_CMD_MODE_POS;
    temp.att.pitch = pos.pitch / SU17_SCALE_FACTOR;
    temp.att.roll = pos.roll / SU17_SCALE_FACTOR;
    temp.att.yaw = pos.yaw / SU17_SCALE_FACTOR;
    return pack(SU17::GIMBAL_CMD_SET_POS, (uint8_t *)&temp, sizeof(temp));
}

/**
 * It takes a struct of type AMOV_GIMBAL_POS_T and converts it to a struct of type
 * G1::GIMBAL_SET_POS_MSG_T
 *
 * @param speed the speed of the gimbal
 *
 * @return The return value is the number of bytes written to the buffer.
 */
uint32_t su17GimbalDriver::setGimabalSpeed(const AMOV_GIMBAL_POS_T &speed)
{
    SU17::GIMBAL_SET_ATT_T temp;

    temp.mode = SU17::GIMBAL_CMD_MODE_SPEED;
    temp.att.pitchSpeed = speed.pitch / SU17_SCALE_FACTOR;
    temp.att.rollSpeed = speed.roll / SU17_SCALE_FACTOR;
    temp.att.yawSpeed = speed.yaw / SU17_SCALE_FACTOR;
    return pack(SU17::GIMBAL_CMD_SET_POS, (uint8_t *)&temp, sizeof(temp));
}

/**
 * This function sets the gimbal's follow speed
 *
 * @param followSpeed the speed of the gimbal
 *
 * @return The return value is the number of bytes written to the buffer.
 */
// uint32_t su17GimbalDriver::setGimabalFollowSpeed(const AMOV_GIMBAL_POS_T &followSpeed)
// {
//     return 0;
// }

/**
 * This function sets the gimbal to its home position
 *
 * @return The return value is the number of bytes written to the buffer.
 */
// uint32_t su17GimbalDriver::setGimabalHome(void)
// {
//     return 0;
// }

/**
 * It takes a picture.
 *
 * @return The return value is the number of bytes written to the serial port.
 */
uint32_t su17GimbalDriver::takePic(void)
{
    return pack(SU17::GIMBAL_CMD_TAKEPIC, nullptr, 0);
}

/**
 * The function sets the video state of the gimbal
 *
 * @param newState The new state of the video.
 *
 * @return The return value is the number of bytes written to the serial port.
 */
uint32_t su17GimbalDriver::setVideo(const AMOV_GIMBAL_VIDEO_T newState)
{
    uint8_t cmd = 0;
    return pack(SU17::GIMBAL_CMD__VIDEO, &cmd, 1);
}
