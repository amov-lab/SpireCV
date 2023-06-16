/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-04-12 09:12:52
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-04-18 11:37:42
 * @FilePath: /spirecv-gimbal-sdk/gimbal_ctrl/sv_gimbal.cpp
 */
#include "amov_gimbal.h"
#include "amov_gimbal_struct.h"

#include "sv_gimbal.h"
#include "sv_gimbal_io.hpp"

#include <iostream>
#include <string>
#include <thread>

/**
 * This function sets the serial port for a Gimbal object.
 *
 * @param port The parameter "port" is a constant reference to a string object. It is used to set the
 * serial port for a Gimbal object.
 */
void sv::Gimbal::setSerialPort(const std::string &port)
{
    this->m_serial_port = port;
}

/**
 * This function sets the baud rate for the serial port of a Gimbal object.
 *
 * @param baud_rate baud_rate is an integer parameter that represents the baud rate (bits per second)
 * for the serial port. It is used to set the communication speed between the Gimbal and the device it
 * is connected to via the serial port.
 */
void sv::Gimbal::setSerialPort(int baud_rate)
{
    this->m_serial_baud_rate = baud_rate;
}

/**
 * This function sets the serial port parameters for a Gimbal object.
 *
 * @param byte_size The number of bits in each byte of serial data. It can be 5, 6, 7, 8, or 9 bits.
 * @param parity Parity refers to the method of error detection in serial communication. It is used to
 * ensure that the data transmitted between two devices is accurate and error-free. There are three
 * types of parity: even, odd, and none. Even parity means that the number of 1s in the data byte plus
 * @param stop_bits Stop bits refer to the number of bits used to indicate the end of a character. It
 * is a parameter used in serial communication to ensure that the receiver knows when a character has
 * ended. Common values for stop bits are 1 or 2.
 * @param flowcontrol GimablSerialFlowControl is an enumeration type that represents the flow control
 * settings for the serial port. It can have one of the following values:
 * @param time_out The time-out parameter is an integer value that specifies the maximum amount of time
 * to wait for a response from the serial port before timing out. If no response is received within
 * this time, the function will return an error.
 */
void sv::Gimbal::setSerialPort(GimablSerialByteSize byte_size, GimablSerialParity parity,
                               GimablSerialStopBits stop_bits, GimablSerialFlowControl flowcontrol,
                               int time_out)
{
    this->m_serial_byte_size = (int)byte_size;
    this->m_serial_parity = (int)parity;
    this->m_serial_stopbits = (int)stop_bits;
    this->m_serial_flowcontrol = (int)flowcontrol;
    this->m_serial_timeout = (int)time_out;
}

/**
 * This function sets the network IP address for a Gimbal object in C++.
 *
 * @param ip The parameter "ip" is a constant reference to a string. It is used to set the value of the
 * member variable "m_net_ip" in the class "Gimbal".
 */
void sv::Gimbal::setNetIp(const std::string &ip)
{
    this->m_net_ip = ip;
}

/**
 * This function sets the network port for a Gimbal object in C++.
 *
 * @param port The "port" parameter is an integer value that represents the network port number that
 * the Gimbal object will use for communication. This function sets the value of the "m_net_port"
 * member variable of the Gimbal object to the value passed in as the "port" parameter.
 */
void sv::Gimbal::setNetPort(const int &port)
{
    this->m_net_port = port;
}

/**
 * The function sets a parser callback for a gimbal device.
 *
 * @param callback callback is a function pointer of type sv::PStateInvoke. It is a callback function
 * that will be invoked when the state of the Gimbal device changes. The function takes a single
 * parameter of type sv::PState, which represents the new state of the Gimbal device.
 */
void sv::Gimbal::setStateCallback(sv::PStateInvoke callback)
{
    amovGimbal::gimbal *pdevTemp = (amovGimbal::gimbal *)this->dev;
    pdevTemp->dev->setParserCallback(callback);
}

/**
 * The function opens a communication interface with a gimbal device and sets up a parser to handle
 * incoming data.
 *
 * @param callback callback is a function pointer to a PStateInvoke function, which is a callback
 * function that will be invoked when the gimbal receives a new packet of data. The function takes in a
 * PState object as its argument, which contains the current state of the gimbal. The purpose of the
 * callback function
 *
 * @return A boolean value is being returned.
 */
bool sv::Gimbal::open(PStateInvoke callback)
{
    if (this->m_gimbal_link == GimbalLink::SERIAL)
    {
        this->IO = new UART(this->m_serial_port,
                            (uint32_t)this->m_serial_baud_rate,
                            serial::Timeout::simpleTimeout(this->m_serial_timeout),
                            (serial::bytesize_t)this->m_serial_byte_size,
                            (serial::parity_t)this->m_serial_parity,
                            (serial::stopbits_t)this->m_serial_stopbits,
                            (serial::flowcontrol_t)this->m_serial_flowcontrol);
    }
    // Subsequent additions
    else if (this->m_gimbal_link == sv::GimbalLink::ETHERNET_TCP)
    {
        return false;
    }
    else if (this->m_gimbal_link == sv::GimbalLink::ETHERNET_UDP)
    {
        return false;
    }
    else
    {
        throw "Error: Unsupported communication interface class!!!";
        return false;
    }
    std::string driverName;
    switch (this->m_gimbal_type)
    {
    case sv::GimbalType::G1:
        driverName = "G1";
        break;
    case sv::GimbalType::Q10f:
        driverName = "Q10f";
        break;

    default:
        throw "Error: Unsupported driver!!!";
        return false;
        break;
    }
    this->dev = new amovGimbal::gimbal(driverName, (amovGimbal::IOStreamBase *)this->IO);

    amovGimbal::gimbal *pdevTemp = (amovGimbal::gimbal *)this->dev;

    pdevTemp->dev->startStack();
    pdevTemp->dev->parserAuto(callback);

    return true;
}

/**
 * This function sets the home position of a gimbal device and returns a boolean value indicating
 * success or failure.
 *
 * @return A boolean value is being returned. If the function call `setGimabalHome()` returns a value
 * greater than 0, then `true` is returned. Otherwise, `false` is returned.
 */
bool sv::Gimbal::setHome()
{
    amovGimbal::gimbal *pdevTemp = (amovGimbal::gimbal *)this->dev;
    if (pdevTemp->dev->setGimabalHome() > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * This function sets the zoom level of a gimbal device and returns a boolean indicating success or
 * failure.
 *
 * @param x The zoom level to set for the gimbal. It should be a positive double value.
 *
 * @return This function returns a boolean value. It returns true if the gimbal zoom is successfully
 * set to the specified value, and false if the specified value is less than or equal to zero or if the
 * setGimbalZoom function call returns a value less than or equal to zero.
 */
bool sv::Gimbal::setZoom(double x)
{
    amovGimbal::gimbal *pdevTemp = (amovGimbal::gimbal *)this->dev;

    if (x <= 0.0)
    {
        return false;
    }

    if (pdevTemp->dev->setGimbalZoom(amovGimbal::AMOV_GIMBAL_ZOOM_STOP, x) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * This function sets the auto zoom state of a gimbal device.
 *
 * @param state The state parameter is an integer that represents the desired state of the auto zoom
 * feature. It is used to enable or disable the auto zoom feature of the gimbal. A value of 1 enables
 * the auto zoom feature, while a value of 0 disables it.
 *
 * @return This function returns a boolean value. It returns true if the setGimbalZoom function call is
 * successful and false if it fails.
 */
bool sv::Gimbal::setAutoZoom(int state)
{
    amovGimbal::gimbal *pdevTemp = (amovGimbal::gimbal *)this->dev;

    if (pdevTemp->dev->setGimbalZoom((amovGimbal::AMOV_GIMBAL_ZOOM_T)state, 0.0f) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * This function sets the autofocus state of a gimbal device and returns a boolean indicating success
 * or failure.
 *
 * @param state The state parameter is an integer that represents the desired autofocus state. It is
 * likely that a value of 1 represents autofocus enabled and a value of 0 represents autofocus
 * disabled, but without more context it is impossible to say for certain.
 *
 * @return This function returns a boolean value. It returns true if the setGimbalFocus function call
 * is successful and returns a value greater than 0, and false if the function call fails and returns a
 * value less than or equal to 0.
 */
bool sv::Gimbal::setAutoFocus(int state)
{
    amovGimbal::gimbal *pdevTemp = (amovGimbal::gimbal *)this->dev;

    if (pdevTemp->dev->setGimbalFocus((amovGimbal::AMOV_GIMBAL_ZOOM_T)state, 0.0f) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * The function takes a photo using a gimbal device and returns true if successful, false otherwise.
 *
 * @return A boolean value is being returned. It will be true if the function call to takePic() returns
 * a value greater than 0, and false otherwise.
 */
bool sv::Gimbal::takePhoto()
{
    amovGimbal::gimbal *pdevTemp = (amovGimbal::gimbal *)this->dev;

    if (pdevTemp->dev->takePic() > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * The function takes a state parameter and sets the video state of a gimbal device accordingly.
 *
 * @param state The state parameter is an integer that determines the desired state of the video
 * recording function of the Gimbal device. It can have two possible values: 0 for turning off the
 * video recording and 1 for starting the video recording.
 *
 * @return a boolean value. It returns true if the video state was successfully set to the desired
 * state (either off or take), and false if there was an error in setting the state.
 */
bool sv::Gimbal::takeVideo(int state)
{
    amovGimbal::gimbal *pdevTemp = (amovGimbal::gimbal *)this->dev;

    amovGimbal::AMOV_GIMBAL_VIDEO_T newState;
    switch (state)
    {
    case 0:
        newState = amovGimbal::AMOV_GIMBAL_VIDEO_OFF;
        break;
    case 1:
        newState = amovGimbal::AMOV_GIMBAL_VIDEO_TAKE;
        break;
    default:
        newState = amovGimbal::AMOV_GIMBAL_VIDEO_OFF;
        break;
    }

    if (pdevTemp->dev->setVideo(newState) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * This function returns the current state of the video on a gimbal device.
 *
 * @return an integer value that represents the state of the video on the gimbal. If the video is being
 * taken, it returns 1. If the video is off, it returns 0. If the state is unknown, it throws an
 * exception with the message "Unknown state information!!!".
 */
int sv::Gimbal::getVideoState()
{
    amovGimbal::gimbal *pdevTemp = (amovGimbal::gimbal *)this->dev;
    int ret;
    amovGimbal::AMOV_GIMBAL_STATE_T state;
    state = pdevTemp->dev->getGimabalState();
    if (state.video == amovGimbal::AMOV_GIMBAL_VIDEO_TAKE)
    {
        ret = 1;
    }
    else if (state.video == amovGimbal::AMOV_GIMBAL_VIDEO_OFF)
    {
        ret = 0;
    }
    else
    {
        throw "Unknown state information!!!";
    }
    return ret;
}

/**
 * The function sets the angle and rate of a gimbal using Euler angles.
 *
 * @param roll The desired roll angle of the gimbal in degrees.
 * @param pitch The desired pitch angle of the gimbal in degrees.
 * @param yaw The desired yaw angle in degrees. Yaw is the rotation around the vertical axis.
 * @param roll_rate The rate at which the gimbal should rotate around the roll axis, in degrees per
 * second.
 * @param pitch_rate The desired pitch rotation rate in degrees per second. If it is set to 0, it will
 * default to 360 degrees per second.
 * @param yaw_rate The rate at which the yaw angle of the gimbal should change, in degrees per second.
 */
void sv::Gimbal::setAngleEuler(double roll, double pitch, double yaw,
                               double roll_rate, double pitch_rate, double yaw_rate)
{
    amovGimbal::gimbal *pdevTemp = (amovGimbal::gimbal *)this->dev;

    amovGimbal::AMOV_GIMBAL_POS_T temp;

    if (pitch_rate == 0.0f)
        pitch_rate = 360;
    if (roll_rate == 0.0f)
        roll_rate = 360;
    if (yaw_rate == 0.0f)
        yaw_rate = 360;

    temp.pitch = pitch_rate;
    temp.roll = roll_rate;
    temp.yaw = yaw_rate;
    pdevTemp->dev->setGimabalFollowSpeed(temp);
    temp.pitch = pitch;
    temp.roll = roll;
    temp.yaw = yaw;
    pdevTemp->dev->setGimabalPos(temp);
}

/**
 * This function sets the angle rate of a gimbal using Euler angles.
 *
 * @param roll_rate The rate of change of the roll angle of the gimbal, measured in degrees per second.
 * @param pitch_rate The rate of change of pitch angle in degrees per second.
 * @param yaw_rate The rate of change of the yaw angle of the gimbal. Yaw is the rotation of the gimbal
 * around the vertical axis.
 */
void sv::Gimbal::setAngleRateEuler(double roll_rate, double pitch_rate, double yaw_rate)
{
    amovGimbal::gimbal *pdevTemp = (amovGimbal::gimbal *)this->dev;

    amovGimbal::AMOV_GIMBAL_POS_T temp;
    temp.pitch = pitch_rate;
    temp.roll = roll_rate;
    temp.yaw = yaw_rate;
    pdevTemp->dev->setGimabalSpeed(temp);
}

sv::Gimbal::~Gimbal()
{
    delete (amovGimbal::gimbal *)this->dev;
    delete (amovGimbal::IOStreamBase *)this->IO;
}
