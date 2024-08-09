/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-04-12 09:12:52
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-05 17:33:29
 * @FilePath: /SpireCV/gimbal_ctrl/sv_gimbal.cpp
 */
#include "amovGimbal/amov_gimbal.h"

#include "sv_gimbal.h"
#include "sv_gimbal_io.hpp"

#include <iostream>
#include <string>
#include <thread>
namespace sv
{
    std::map<std::string, void *> Gimbal::IOList;
    std::mutex Gimbal::IOListMutex;

    typedef struct
    {
        std::string name;
        GimbalLink supportLink;
    } gimbalTrait;

    std::map<GimbalType, gimbalTrait> gimbaltypeList =
        {
            {GimbalType::G1, {"G1", GimbalLink::SERIAL}},
            {GimbalType::Q10f, {"Q10f", GimbalLink::SERIAL}},
            {GimbalType::AT10, {"AT10", GimbalLink::SERIAL | GimbalLink::ETHERNET_TCP}},
            {GimbalType::GX40, {"GX40", GimbalLink::SERIAL | GimbalLink::ETHERNET_TCP | GimbalLink::ETHERNET_UDP}}};

    /**
     * The function `svGimbalType2Str` converts a `GimbalType` enum value to its corresponding string
     * representation.
     *
     * @return a reference to a string.
     */
    std::string &svGimbalType2Str(const GimbalType &type)
    {
        std::map<GimbalType, gimbalTrait>::iterator temp = gimbaltypeList.find(type);
        if (temp != gimbaltypeList.end())
        {
            return (temp->second).name;
        }
        throw "Error: Unsupported gimbal device type!!!!";
        exit(-1);
    }

    GimbalLink &svGimbalTypeFindLinkType(const GimbalType &type)
    {
        std::map<GimbalType, gimbalTrait>::iterator temp = gimbaltypeList.find(type);
        if (temp != gimbaltypeList.end())
        {
            return (temp->second).supportLink;
        }
        throw "Error: Unsupported gimbal device type!!!!";
        exit(-1);
    }
}

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
 * The function sets the TCP network port for the Gimbal object.
 *
 * @param port The parameter "port" is an integer that represents the TCP network port number.
 */
void sv::Gimbal::setTcpNetPort(const int &port)
{
    this->m_net_port = port;
}

/**
 * The function sets the UDP network ports for receiving and sending data.
 *
 * @param recvPort The recvPort parameter is the port number that the Gimbal object will use to receive
 * UDP packets.
 * @param sendPort The sendPort parameter is the port number used for sending data over UDP (User
 * Datagram Protocol) network communication.
 */
void sv::Gimbal::setUdpNetPort(const int &recvPort, const int &sendPort)
{
    this->m_net_recv_port = recvPort;
    this->m_net_send_port = sendPort;
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
    m_callback = callback;
    pdevTemp->setParserCallback(sv::Gimbal::gimbalUpdataCallback, this);
}

/**
 * The function `sv::Gimbal::creatIO` creates an IO object based on the specified gimbal type and link
 * type, and returns a pointer to the created object.
 *
 * @param dev The "dev" parameter is a pointer to an object of type "sv::Gimbal". It is used to access
 * the member variables of the Gimbal object, such as "m_serial_port", "m_serial_baud_rate",
 * "m_serial_timeout", etc. These variables store information about
 *
 * @return a void pointer.
 */
void *sv::Gimbal::creatIO(sv::Gimbal *dev)
{
    IOListMutex.lock();
    std::map<std::string, void *>::iterator list = IOList.find(dev->m_serial_port);
    std::pair<std::string, void *> key("NULL", nullptr);
    GimbalLink link = svGimbalTypeFindLinkType(dev->m_gimbal_type);

    if ((dev->m_gimbal_link & svGimbalTypeFindLinkType(dev->m_gimbal_type)) == GimbalLink::NONE)
    {
        throw std::runtime_error("gimbal Unsupported linktype !!!");
    }

    if (list == IOList.end())
    {
        if (dev->m_gimbal_link == GimbalLink::SERIAL)
        {
            UART *ser;
            ser = new UART(dev->m_serial_port,
                           (uint32_t)dev->m_serial_baud_rate,
                           serial::Timeout::simpleTimeout(dev->m_serial_timeout),
                           (serial::bytesize_t)dev->m_serial_byte_size,
                           (serial::parity_t)dev->m_serial_parity,
                           (serial::stopbits_t)dev->m_serial_stopbits,
                           (serial::flowcontrol_t)dev->m_serial_flowcontrol);
            key.first = dev->m_serial_port;
            key.second = (void *)ser;
            IOList.insert(key);
        }
        else if (dev->m_gimbal_link == sv::GimbalLink::ETHERNET_TCP)
        {
            TCPClient *tcp;
            tcp = new TCPClient(dev->m_net_ip, dev->m_net_port);
            key.first = dev->m_net_ip;
            key.second = (void *)tcp;
            IOList.insert(key);
        }
        else if (dev->m_gimbal_link == sv::GimbalLink::ETHERNET_UDP)
        {
            UDP *udp;
            udp = new UDP(dev->m_net_ip, dev->m_net_recv_port, dev->m_net_send_port);
            key.first = dev->m_net_ip;
            key.second = (void *)udp;
            IOList.insert(key);
        }
    }
    else
    {
        std::cout << "Error: gimbal IO has opened!!!" << std::endl;
    }
    IOListMutex.unlock();

    return key.second;
}

/**
 * The function removes a Gimbal device from the IOList.
 *
 * @param dev dev is a pointer to an object of type sv::Gimbal.
 */
void sv::Gimbal::removeIO(sv::Gimbal *dev)
{
    IOListMutex.lock();
    IOList.erase(dev->m_serial_port);

    IOListMutex.unlock();
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
    bool ret = false;

    this->IO = creatIO(this);

    if (this->IO != nullptr)
    {
        std::string driverName;
        driverName = sv::svGimbalType2Str(this->m_gimbal_type);
        this->dev = new amovGimbal::gimbal(driverName, (amovGimbal::IOStreamBase *)this->IO);
        amovGimbal::gimbal *pdevTemp = (amovGimbal::gimbal *)this->dev;
        pdevTemp->startStack();
        m_callback = callback;
        pdevTemp->parserAuto(sv::Gimbal::gimbalUpdataCallback, this);

        ret = true;
    }

    return ret;
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
    if (pdevTemp->setGimabalHome() > 0)
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

    if (pdevTemp->setGimbalZoom(AMOV_GIMBAL_ZOOM_STOP, x) > 0)
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

    if (pdevTemp->setGimbalZoom((AMOV_GIMBAL_ZOOM_T)state, 0.0f) > 0)
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

    if (pdevTemp->setGimbalFocus((AMOV_GIMBAL_ZOOM_T)state, 0.0f) > 0)
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

    if (pdevTemp->takePic() > 0)
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

    AMOV_GIMBAL_VIDEO_T newState;
    switch (state)
    {
    case 0:
        newState = AMOV_GIMBAL_VIDEO_OFF;
        break;
    case 1:
        newState = AMOV_GIMBAL_VIDEO_TAKE;
        break;
    default:
        newState = AMOV_GIMBAL_VIDEO_OFF;
        break;
    }

    if (pdevTemp->setVideo(newState) > 0)
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
    AMOV_GIMBAL_STATE_T state;
    state = pdevTemp->getGimabalState();
    if (state.video == AMOV_GIMBAL_VIDEO_TAKE)
    {
        ret = 1;
    }
    else if (state.video == AMOV_GIMBAL_VIDEO_OFF)
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

    AMOV_GIMBAL_POS_T temp;

    if (pitch_rate == 0.0f)
        pitch_rate = 360;
    if (roll_rate == 0.0f)
        roll_rate = 360;
    if (yaw_rate == 0.0f)
        yaw_rate = 360;

    temp.pitch = pitch_rate;
    temp.roll = roll_rate;
    temp.yaw = yaw_rate;
    pdevTemp->setGimabalFollowSpeed(temp);
    temp.pitch = pitch;
    temp.roll = roll;
    temp.yaw = yaw;
    pdevTemp->setGimabalPos(temp);
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

    AMOV_GIMBAL_POS_T temp;
    temp.pitch = pitch_rate;
    temp.roll = roll_rate;
    temp.yaw = yaw_rate;
    pdevTemp->setGimabalSpeed(temp);
}
void sv::Gimbal::attitudeCorrection(const GimbalQuaternionT &quaterion,
                                    const GimbalVelocityT &speed,
                                    const GimbalVelocityT &acc, void *extenData)
{
    amovGimbal::gimbal *pdevTemp = (amovGimbal::gimbal *)this->dev;
    AMOV_GIMBAL_QUATERNION_T temp_q;
    temp_q.q0 = quaterion.q0;
    temp_q.q1 = quaterion.q1;
    temp_q.q2 = quaterion.q2;
    temp_q.q3 = quaterion.q3;

    AMOV_GIMBAL_VELOCITY_T temp_v1, temp_v2;
    temp_v1.x = speed.x;
    temp_v1.y = speed.y;
    temp_v1.z = speed.z;

    temp_v2.x = acc.x;
    temp_v2.y = acc.y;
    temp_v2.z = acc.z;

    pdevTemp->attitudeCorrection(temp_q, temp_v1, temp_v2, extenData);
}

void sv::Gimbal::attitudeCorrection(const GimbalPosT &pos,
                                    const GimbalVelocityT &speed,
                                    const GimbalVelocityT &acc, void *extenData)
{
    amovGimbal::gimbal *pdevTemp = (amovGimbal::gimbal *)this->dev;
    AMOV_GIMBAL_VELOCITY_T temp_v1, temp_v2;
    temp_v1.x = speed.x;
    temp_v1.y = speed.y;
    temp_v1.z = speed.z;

    temp_v2.x = acc.x;
    temp_v2.y = acc.y;
    temp_v2.z = acc.z;

    AMOV_GIMBAL_POS_T temp_p;
    temp_p.pitch = pos.pitch;
    temp_p.yaw = pos.yaw;
    temp_p.roll = pos.roll;

    pdevTemp->attitudeCorrection(temp_p, temp_v1, temp_v2, extenData);
}

sv::Gimbal::~Gimbal()
{
    removeIO(this);
    delete (amovGimbal::gimbal *)this->dev;
    delete (amovGimbal::IOStreamBase *)this->IO;
}
