/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-03-02 10:00:52
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-05 16:29:51
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/G1/g1_gimbal_funtion.cpp
 */
#include "g1_gimbal_driver.h"
#include "g1_gimbal_crc32.h"
#include "string.h"
#include <math.h>

/**
 * It sets the gimbal position.
 *
 * @param pos the position of the gimbal
 *
 * @return The return value is the number of bytes written to the buffer.
 */
uint32_t g1GimbalDriver::setGimabalPos(const AMOV_GIMBAL_POS_T &pos)
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
 * It takes a struct of type AMOV_GIMBAL_POS_T and converts it to a struct of type
 * G1::GIMBAL_SET_POS_MSG_T
 *
 * @param speed the speed of the gimbal
 *
 * @return The return value is the number of bytes written to the buffer.
 */
uint32_t g1GimbalDriver::setGimabalSpeed(const AMOV_GIMBAL_POS_T &speed)
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
uint32_t g1GimbalDriver::setGimabalFollowSpeed(const AMOV_GIMBAL_POS_T &followSpeed)
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
uint32_t g1GimbalDriver::setVideo(const AMOV_GIMBAL_VIDEO_T newState)
{
    uint8_t temp = G1::GIMBAL_CMD_CAMERA_REC;

    mState.lock();
    if (state.video == AMOV_GIMBAL_VIDEO_TAKE)
    {
        state.video = AMOV_GIMBAL_VIDEO_OFF;
    }
    else
    {
        state.video = AMOV_GIMBAL_VIDEO_TAKE;
    }
    mState.unlock();

    return pack(G1::GIMBAL_CMD_CAMERA, &temp, sizeof(uint8_t));
}

/**
 * The function `attitudeCorrection` takes in quaternion, velocity, acceleration, and external data,
 * and returns a packed message.
 *
 * @param quaterion The "quaterion" parameter is a structure of type "AMOV_GIMBAL_QUATERNION_T" which
 * contains the following fields:
 * @param speed The "speed" parameter is of type `AMOV_GIMBAL_VELOCITY_T` and represents
 * the velocity of the gimbal. It contains three components: `x`, `y`, and `z`, which represent the
 * velocity in the respective axes.
 * @param acc The "acc" parameter is of type "AMOV_GIMBAL_VELOCITY_T" and represents the
 * acceleration of the gimbal in three dimensions (x, y, z).
 * @param extenData The extenData parameter is a void pointer that can be used to pass additional data
 * to the attitudeCorrection function. In this case, it is being cast to a float pointer and then
 * accessed as an array. The first element of the array is assigned to the temp.yawSetPoint variable,
 * and
 *
 * @return a uint32_t value.
 */
uint32_t g1GimbalDriver::attitudeCorrection(const AMOV_GIMBAL_QUATERNION_T &quaterion,
                                            const AMOV_GIMBAL_VELOCITY_T &speed,
                                            const AMOV_GIMBAL_VELOCITY_T &acc,
                                            void *extenData)
{
    G1::GIMBAL_ATT_CORR_MSG_T temp;
    temp.q[0] = quaterion.q0;
    temp.q[1] = quaterion.q1;
    temp.q[2] = quaterion.q2;
    temp.q[3] = quaterion.q3;

    temp.acc[0] = acc.x;
    temp.acc[1] = acc.y;
    temp.acc[2] = acc.z;

    temp.yawSetPoint = ((float *)extenData)[0];
    temp.yawSpeedSetPoint = ((float *)extenData)[1];

    return pack(G1::GIMBAL_CMD_SET_STATE, reinterpret_cast<uint8_t *>(&temp), sizeof(G1::GIMBAL_ATT_CORR_MSG_T));
}

/**
 * The function `attitudeCorrection` calculates the attitude correction for a gimbal based on the given
 * position, velocity, and acceleration values.
 *
 * @param pos The "pos" parameter is of type AMOV_GIMBAL_POS_T and represents the current
 * position of the gimbal. It contains the pitch, roll, and yaw angles of the gimbal.
 * @param seppd seppd stands for "Separate Pointing Device" and it represents the velocity of the
 * gimbal in terms of pitch, roll, and yaw. It is of type `AMOV_GIMBAL_VELOCITY_T` which
 * likely contains three float values for pitch,
 * @param acc The "acc" parameter is of type "AMOV_GIMBAL_VELOCITY_T" and represents the
 * acceleration of the gimbal in three dimensions (x, y, z).
 * @param extenData The `extenData` parameter is a void pointer that can be used to pass additional
 * data to the `attitudeCorrection` function. In this code snippet, it is assumed that `extenData` is a
 * pointer to a float array with two elements.
 *
 * @return a uint32_t value.
 */
uint32_t g1GimbalDriver::attitudeCorrection(const AMOV_GIMBAL_POS_T &pos,
                                            const AMOV_GIMBAL_VELOCITY_T &seppd,
                                            const AMOV_GIMBAL_VELOCITY_T &acc,
                                            void *extenData)
{
    G1::GIMBAL_ATT_CORR_MSG_T temp;

    float pitch = pos.pitch * 0.5f;
    float roll = pos.roll * 0.5f;
    float yaw = pos.yaw * 0.5f;

    temp.q[0] = cosf(pitch) * cosf(roll) * cosf(yaw) +
                sinf(pitch) * sinf(roll) * sinf(yaw);
    temp.q[1] = cosf(pitch) * sinf(roll) * cosf(yaw) -
                sinf(pitch) * cosf(roll) * sinf(yaw);
    temp.q[2] = sinf(pitch) * cosf(roll) * cosf(yaw) +
                cosf(pitch) * sinf(roll) * sinf(yaw);
    temp.q[3] = cosf(pitch) * sinf(roll) * sinf(yaw) -
                sinf(pitch) * cosf(roll) * cosf(yaw);

    temp.acc[0] = acc.x;
    temp.acc[1] = acc.y;
    temp.acc[2] = acc.z;

    temp.yawSetPoint = ((float *)extenData)[0];
    temp.yawSpeedSetPoint = ((float *)extenData)[1];

    return pack(G1::GIMBAL_CMD_SET_STATE, reinterpret_cast<uint8_t *>(&temp), sizeof(G1::GIMBAL_ATT_CORR_MSG_T));
}

/**
 * The function `extensionFuntions` in the `g1GimbalDriver` class takes a void pointer `cmd`, casts it
 * to a `G1::GIMBAL_STD_MSG_T` pointer, and returns the result of calling the `pack` function with the
 * `cmd`'s `cmd`, `data`, and `len` members as arguments.
 * 
 * @param cmd The "cmd" parameter is a void pointer, which means it can point to any type of data. In
 * this case, it is being cast to a G1::GIMBAL_STD_MSG_T pointer using reinterpret_cast.
 * 
 * @return the result of the `pack` function, which is of type `uint32_t`.
 */
uint32_t g1GimbalDriver::extensionFuntions(void *cmd)
{
    G1::GIMBAL_STD_MSG_T *tempCmd;

    tempCmd = reinterpret_cast<G1::GIMBAL_STD_MSG_T *>(cmd);
    return pack(tempCmd->cmd, tempCmd->data, tempCmd->len);
}