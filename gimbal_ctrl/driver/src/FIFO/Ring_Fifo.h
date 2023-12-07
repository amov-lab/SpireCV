/*
 * @Description    :
 * @Author         : Aiyangsky
 * @Date           : 2022-08-26 21:42:02
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-11-28 11:47:39
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/FIFO/Ring_Fifo.h
 */

#ifndef RING_FIFO_H
#define RING_FIFO_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>

class fifoRing
{
private:
    unsigned char *start;
    unsigned char *in;
    unsigned char *out;
    unsigned char *end;

    unsigned short currNumber;
    unsigned short maxNumber;
    unsigned short cellSize;

    std::mutex fifoMutex;
    std::condition_variable_any notEmpty;

public:
    fifoRing(unsigned short _cellSize, unsigned int _cellNum);
    ~fifoRing()
    {
        if (start != nullptr)
        {
            free(start);
        }
    }

    bool inCell(void *data);
    unsigned short inCells(void *data, unsigned short number);
    bool outCell(void *data);
    unsigned short outCells(void *data, unsigned short number);
};

#endif