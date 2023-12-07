/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-04-12 12:22:09
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-05 17:38:59
 * @FilePath: /SpireCV/gimbal_ctrl/sv_gimbal_io.hpp
 */
#ifndef __SV_GIMABL_IO_H
#define __SV_GIMABL_IO_H

#include "amovGimbal/amov_gimbal.h"
#include "serial/serial.h"

#include <string.h>
#include <stdio.h>
#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#if defined(__linux__)
#include <arpa/inet.h>
#endif

#include <unistd.h>
namespace sv
{
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
        virtual uint32_t inPutBytes(IN uint8_t *byte)
        {
            if (ser->read(byte, 1))
            {
                return 1;
            }
            return 0;
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
        }
    };

    int scoketClose(int scoket)
    {
#if defined(_WIN32)
        return closesocket(scoket);
#endif
#if defined(__linux__)
        return close(scoket);
#endif
    }
    class TCPClient : public amovGimbal::IOStreamBase
    {
    private:
        int scoketFd;
        sockaddr_in scoketAddr;

    public:
        virtual bool open()
        {
            return true;
        }
        virtual bool close()
        {
            sv::scoketClose(scoketFd);
            return true;
        }
        virtual bool isOpen()
        {
            return true;
        }
        virtual bool isBusy()
        {
            return false;
        }
        virtual uint32_t inPutBytes(IN uint8_t *byte)
        {
            return recv(scoketFd, (char *)byte, 65536, 0);
        }
        virtual uint32_t outPutBytes(IN uint8_t *byte, uint32_t lenght)
        {
            return send(scoketFd, (const char *)byte, lenght, 0);
        }

        TCPClient(const std::string &addr, const uint16_t port)
        {
            if ((scoketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            {
                throw std::runtime_error("scoket creat failed");
            }
            memset(&scoketAddr, 0, sizeof(scoketAddr));
            scoketAddr.sin_family = AF_INET;
            scoketAddr.sin_addr.s_addr = inet_addr(addr.c_str());

            if (scoketAddr.sin_addr.s_addr == INADDR_NONE ||
                scoketAddr.sin_addr.s_addr == INADDR_ANY)
            {
                sv::scoketClose(scoketFd);

                throw std::runtime_error("scoket addr errr");
            }
            scoketAddr.sin_port = htons(port);

            if (connect(scoketFd, (struct sockaddr *)&scoketAddr, sizeof(scoketAddr)) != 0)
            {
                sv::scoketClose(scoketFd);

                throw std::runtime_error("scoket connect failed !");
            }
        }
        ~TCPClient()
        {
            close();
        }
    };

    class UDP : public amovGimbal::IOStreamBase
    {
    private:
        int rcvScoketFd, sendScoketFd;
        sockaddr_in rcvScoketAddr, sendScoketAddr;

    public:
        virtual bool open()
        {
            return true;
        }
        virtual bool close()
        {
            sv::scoketClose(rcvScoketFd);
            sv::scoketClose(sendScoketFd);
            return true;
        }
        virtual bool isOpen()
        {
            return true;
        }
        virtual bool isBusy()
        {
            return false;
        }
        virtual uint32_t inPutBytes(IN uint8_t *byte)
        {
            sockaddr_in remoteAddr;
            int len = sizeof(remoteAddr);

            return recvfrom(rcvScoketFd, (char *)byte, 65536, 0,
                            (struct sockaddr *)&remoteAddr, reinterpret_cast<socklen_t *>(&len));
        }
        virtual uint32_t outPutBytes(IN uint8_t *byte, uint32_t lenght)
        {
            return sendto(sendScoketFd, (const char *)byte, lenght, 0,
                          (struct sockaddr *)&sendScoketAddr, sizeof(sendScoketAddr));
        }

        UDP(const std::string &remoteAddr, const uint16_t rcvPort, uint16_t sendPort)
        {
            if ((rcvScoketFd = socket(AF_INET, SOCK_DGRAM, 0)) == -1 ||
                (sendScoketFd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
            {
                sv::scoketClose(rcvScoketFd);
                sv::scoketClose(sendScoketFd);
                throw std::runtime_error("scoket creat failed");
            }
            memset(&rcvScoketAddr, 0, sizeof(rcvScoketAddr));
            memset(&sendScoketAddr, 0, sizeof(sendScoketAddr));
            sendScoketAddr.sin_family = AF_INET;
            sendScoketAddr.sin_addr.s_addr = inet_addr(remoteAddr.c_str());
            sendScoketAddr.sin_port = htons(sendPort);

            if (sendScoketAddr.sin_addr.s_addr == INADDR_NONE ||
                sendScoketAddr.sin_addr.s_addr == INADDR_ANY)
            {
                sv::scoketClose(sendScoketFd);

                throw std::runtime_error("scoket addr errr");
            }

            rcvScoketAddr.sin_family = AF_INET;
            rcvScoketAddr.sin_addr.s_addr = INADDR_ANY;
            rcvScoketAddr.sin_port = htons(rcvPort);

            if (rcvScoketAddr.sin_addr.s_addr == INADDR_NONE)
            {
                sv::scoketClose(rcvScoketFd);

                throw std::runtime_error("scoket addr errr");
            }

            if (bind(rcvScoketFd, (struct sockaddr *)&rcvScoketAddr, sizeof(rcvScoketAddr)) == -1)
            {
                sv::scoketClose(rcvScoketFd);

                throw std::runtime_error("scoket bind failed !");
            }
        }
        ~UDP()
        {
            close();
        }
    };
}

#endif