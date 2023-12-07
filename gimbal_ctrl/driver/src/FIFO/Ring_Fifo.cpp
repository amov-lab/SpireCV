/*
 * @Description    :
 * @Author         : Aiyangsky
 * @Date           : 2022-08-26 21:42:10
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-11-28 11:47:34
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/FIFO/Ring_Fifo.cpp
 */

#include <string.h>

#include "Ring_Fifo.h"

/**
 * The `fifoRing` constructor initializes the `cellSize`, `maxNumber`, `currNumber`, `start`, `in`,
 * `out`, and `end` variables, and allocates memory for the `start` pointer.
 *
 * @param _cellSize The `_cellSize` parameter represents the size of each cell in the FIFO ring buffer.
 * It determines the amount of memory allocated for each element in the buffer.
 * @param _cellNum The parameter `_cellNum` represents the number of cells in the FIFO ring.
 */
fifoRing::fifoRing(unsigned short _cellSize, unsigned int _cellNum)
{
    cellSize = _cellSize;
    maxNumber = _cellNum;
    currNumber = 0;

    start = nullptr;
    start = (uint8_t *)malloc(_cellNum * _cellSize);
    if (start == nullptr)
    {
        std::cout << "fifo malloc failed! size :" << (_cellNum * _cellSize) << std::endl;
        exit(1);
    }

    memset(start, 0, _cellNum * _cellSize);

    in = start;
    out = start;
    end = start + _cellNum * _cellSize;
}

/**
 * The `inCell` function adds data to a FIFO ring buffer and returns true if successful.
 *
 * @param data A pointer to the data that needs to be stored in the FIFO ring.
 *
 * @return a boolean value.
 */
bool fifoRing::inCell(void *data)
{
    std::lock_guard<std::mutex> locker(fifoMutex);
    unsigned char *next;
    unsigned char *ptemp = in;
    bool ret = false;

    if (currNumber < maxNumber)
    {
        next = in + cellSize;
        if (next >= end)
        {
            next = start;
        }
        in = next;
        currNumber++;
        memcpy(ptemp, data, cellSize);
        ret = true;
        notEmpty.notify_all();
    }

    return ret;
}

/**
 * The `inCells` function is used to store data in a FIFO ring buffer, returning the number of cells
 * successfully stored.
 *
 * @param data A pointer to the data that needs to be stored in the FIFO ring.
 * @param number The parameter "number" represents the number of cells that the function should attempt
 * to store in the FIFO ring.
 *
 * @return the number of cells that were successfully stored in the FIFO ring buffer.
 */
unsigned short fifoRing::inCells(void *data, unsigned short number)
{
    std::lock_guard<std::mutex> locker(fifoMutex);
    // Number of remaining storable cells is described to simplify the calculation in the copying process.
    unsigned short diff = maxNumber - currNumber;
    unsigned short count_temp, count_temp_r;
    unsigned char *next;
    unsigned char *ptemp = in;
    unsigned short ret;

    if (diff > number)
    {
        ret = number;
    }
    else if (diff > 0 && diff < number)
    {
        ret = diff;
    }
    else
    {
        ret = 0;
    }

    count_temp = cellSize * ret;
    next = in + count_temp;
    // Moving the write pointer and the number of stored cells before
    // copying data reduces the likelihood of multithreaded write conflicts.
    currNumber += ret;

    if (next < end)
    {
        in = next;
        memcpy(ptemp, data, count_temp);
    }
    else
    {
        count_temp_r = end - in;
        next = start + count_temp - count_temp_r;
        in = next;
        memcpy(ptemp, data, count_temp_r);
        memcpy(start, ((unsigned char *)data) + count_temp_r, count_temp - count_temp_r);
    }
    if (ret > 0)
    {
        notEmpty.notify_all();
    }

    return ret;
}

/**
 * The `outCell` function removes a cell from the FIFO ring buffer and copies its data to the provided
 * memory location.
 *
 * @param data A pointer to the memory location where the data from the cell will be copied to.
 *
 * @return a boolean value. If a cell is successfully taken from the FIFO ring and the data is copied
 * into the provided pointer, the function returns true. Otherwise, if the FIFO ring is empty and no
 * cell can be taken, the function waits until a cell becomes available and then returns false.
 */
bool fifoRing::outCell(void *data)
{
    std::lock_guard<std::mutex> locker(fifoMutex);
    unsigned char *next;
    unsigned char *ptemp = out;
    bool ret = false;

    if (currNumber > 0)
    {
        next = out + cellSize;
        if (next >= end)
        {
            next = start;
        }
        out = next;
        currNumber--;
        memcpy(data, ptemp, cellSize);
        ret = true;
    }
    else
    {
        notEmpty.wait(fifoMutex);
    }

    return ret;
}

/**
 * The `outCells` function retrieves a specified number of cells from a FIFO ring buffer and copies the
 * data into a provided buffer.
 *
 * @param data A pointer to the memory location where the extracted data will be stored.
 * @param number The parameter "number" represents the number of cells that should be read from the
 * FIFO ring.
 *
 * @return the number of cells that were successfully read from the FIFO ring buffer.
 */
unsigned short fifoRing::outCells(void *data, unsigned short number)
{
    std::lock_guard<std::mutex> locker(fifoMutex);

    unsigned char *next;
    unsigned char *ptemp = out;
    unsigned short count_temp, count_temp_r;
    unsigned short ret;

    if (currNumber > number)
    {
        ret = number;
    }
    else if (currNumber < number && currNumber > 0)
    {
        ret = currNumber;
    }
    else
    {
        ret = 0;
    }

    count_temp = cellSize * ret;
    next = out + count_temp;

    currNumber -= ret;

    if (next < end)
    {
        out = next;
        memcpy(data, ptemp, count_temp);
    }
    else
    {
        count_temp_r = end - in;
        next = start + count_temp - count_temp_r;
        out = next;
        memcpy(data, ptemp, count_temp_r);
        memcpy(((unsigned char *)data) + count_temp_r, start, count_temp - count_temp_r);
    }

    if (ret == 0)
    {
        notEmpty.wait(fifoMutex);
    }

    return ret;
}
