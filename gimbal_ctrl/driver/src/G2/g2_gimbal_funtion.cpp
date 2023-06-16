/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-03-13 11:58:54
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-03-13 12:31:58
 * @FilePath: \gimbal-sdk-multi-platform\src\G2\g2_gimbal_funtion.cpp
 */
#include "g2_gimbal_driver.h"
#include "g2_gimbal_crc.h"
#include "string.h"

/**
 * It sets the gimbal position.
 *
 * @param pos the position of the gimbal
 *
 * @return The return value is the number of bytes written to the buffer.
 */
uint32_t g2GimbalDriver::setGimabalPos(const amovGimbal::AMOV_GIMBAL_POS_T &pos)
{
    return 0;
}

/**
 * It takes a struct of type amovGimbal::AMOV_GIMBAL_POS_T and converts it to a struct of type
 * G1::GIMBAL_SET_POS_MSG_T
 *
 * @param speed the speed of the gimbal
 *
 * @return The return value is the number of bytes written to the buffer.
 */
uint32_t g2GimbalDriver::setGimabalSpeed(const amovGimbal::AMOV_GIMBAL_POS_T &speed)
{
    return 0;
}

/**
 * This function sets the gimbal's follow speed
 *
 * @param followSpeed the speed of the gimbal
 *
 * @return The return value is the number of bytes written to the buffer.
 */
uint32_t g2GimbalDriver::setGimabalFollowSpeed(const amovGimbal::AMOV_GIMBAL_POS_T &followSpeed)
{
    return 0;
}

/**
 * This function sets the gimbal to its home position
 *
 * @return The return value is the number of bytes written to the buffer.
 */
uint32_t g2GimbalDriver::setGimabalHome(void)
{
    return 0;
}

/**
 * It takes a picture.
 *
 * @return The return value is the number of bytes written to the serial port.
 */
uint32_t g2GimbalDriver::takePic(void)
{
    return 0;
}

/**
 * The function sets the video state of the gimbal
 *
 * @param newState The new state of the video.
 *
 * @return The return value is the number of bytes written to the serial port.
 */
uint32_t g2GimbalDriver::setVideo(const amovGimbal::AMOV_GIMBAL_VIDEO_T newState)
{

    return 0;
}
