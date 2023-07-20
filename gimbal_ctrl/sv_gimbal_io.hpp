/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-04-12 12:22:09
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-04-13 10:17:21
 * @FilePath: /spirecv-gimbal-sdk/gimbal_ctrl/sv_gimbal_io.hpp
 */
#ifndef __SV_GIMABL_IO_H
#define __SV_GIMABL_IO_H

#include "amov_gimbal.h"
#include "serial/serial.h"

class UART : public amovGimbal::IOStreamBase
{
private:
    serial::Serial *ser;

public:
    virtual bool open()
    {
        ser->open();
        return true;
    }
    virtual bool close()
    {
        ser->close();
        return true;
    }
    virtual bool isOpen()
    {
        return ser->isOpen();
    }
    virtual bool isBusy()
    {
        return false;
    }

    virtual bool inPutByte(IN uint8_t *byte)
    {
        if (ser->read(byte, 1))
        {
            return true;
        }
        return false;
    }

    virtual uint32_t outPutBytes(IN uint8_t *byte, uint32_t lenght)
    {
        return ser->write(byte, lenght);
    }

    UART(const std::string &port, uint32_t baudrate, serial::Timeout timeout,
         serial::bytesize_t bytesize, serial::parity_t parity, serial::stopbits_t stopbits,
         serial::flowcontrol_t flowcontrol)
    {
        ser = new serial::Serial(port, baudrate, timeout, bytesize, parity, stopbits, flowcontrol);
    }
    ~UART()
    {
        ser->close();
        delete ser;
    }
};

#endif