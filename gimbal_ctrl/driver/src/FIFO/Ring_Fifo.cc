/*
 * @Description    :
 * @Author         : Aiyangsky
 * @Date           : 2022-08-26 21:42:10
 * @LastEditors    : Aiyangsky
 * @LastEditTime   : 2022-08-27 03:43:49
 * @FilePath       : \mavlink\src\route\Ring_Fifo.c
 */

#include <string.h>

#include "Ring_Fifo.h"

/**
 * @description:
 * @param       {RING_FIFO_CB_T} *fifo                  fifo struct pointer
 * @param       {unsigned short} cell_size              sizeof(cell)
 * @param       {unsigned char} *buffer                 fifo buffer address
 * @param       {unsigned int} buffer_lenght            sizeof(buffer)
 * @return      {*}
 * @note       :
 */
void Ring_Fifo_init(RING_FIFO_CB_T *fifo, unsigned short cell_size,
                    unsigned char *buffer, unsigned int buffer_lenght)
{
    fifo->cell_size = cell_size;

    fifo->start = buffer;
    // Remainder is taken to avoid splicing in the output so as to improve the efficiency
    fifo->end = buffer + buffer_lenght - (buffer_lenght % cell_size);
    fifo->in = buffer;
    fifo->out = buffer;
    fifo->curr_number = 0;
    fifo->max_number = buffer_lenght / cell_size;
}

/**
 * @description:                                        add a cell to fifo
 * @param       {RING_FIFO_CB_T} *fifo                  fifo struct pointer
 * @param       {void} *data                            cell data [in]
 * @return      {*}                                     Success or fail
 * @note       :                                        failed if without space
 */
bool Ring_Fifo_in_cell(RING_FIFO_CB_T *fifo, void *data)
{
    unsigned char *next;
    unsigned char *ptemp = fifo->in;
    bool ret = false;

    LOCK();

    if (fifo->curr_number < fifo->max_number)
    {
        next = fifo->in + fifo->cell_size;
        if (next >= fifo->end)
        {
            next = fifo->start;
        }
        fifo->in = next;
        fifo->curr_number++;
        memcpy(ptemp, data, fifo->cell_size);
        ret = true;
    }

    UNLOCK();

    return ret;
}

/**
 * @description:                                        add a series of cells to fifo
 * @param       {RING_FIFO_CB_T} *fifo
 * @param       {void} *data                            cells data [in]
 * @param       {unsigned short} number                 expect add number of cells
 * @return      {*}                                     number of successful add
 * @note       :
 */
unsigned short Ring_Fifo_in_cells(RING_FIFO_CB_T *fifo, void *data, unsigned short number)
{
    // Number of remaining storable cells is described to simplify the calculation in the copying process.
    unsigned short diff = fifo->max_number - fifo->curr_number;
    unsigned short count_temp, count_temp_r;
    unsigned char *next;
    unsigned char *ptemp = fifo->in;
    unsigned short ret;

    LOCK();

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

    count_temp = fifo->cell_size * ret;
    next = fifo->in + count_temp;
    // Moving the write pointer and the number of stored cells before
    // copying data reduces the likelihood of multithreaded write conflicts.
    fifo->curr_number += ret;

    if (next < fifo->end)
    {
        fifo->in = next;
        memcpy(ptemp, data, count_temp);
    }
    else
    {
        count_temp_r = fifo->end - fifo->in;
        next = fifo->start + count_temp - count_temp_r;
        fifo->in = next;
        memcpy(ptemp, data, count_temp_r);
        memcpy(fifo->start, ((unsigned char *)data) + count_temp_r, count_temp - count_temp_r);
    }

    UNLOCK();

    return ret;
}

/**
 * @description:                                        output a cell
 * @param       {RING_FIFO_CB_T} *fifo
 * @param       {void} *data                            cell data [out]
 * @return      {*}                                     Success or fail
 * @note       :                                        fail if without cell
 */
bool Ring_Fifo_out_cell(RING_FIFO_CB_T *fifo, void *data)
{
    unsigned char *next;
    unsigned char *ptemp = fifo->out;
    bool ret = false;

    LOCK();

    if (fifo->curr_number > 0)
    {
        next = fifo->out + fifo->cell_size;
        if (next >= fifo->end)
        {
            next = fifo->start;
        }
        fifo->out = next;
        fifo->curr_number--;
        memcpy(data, ptemp, fifo->cell_size);
        ret = true;
    }

    UNLOCK();

    return ret;
}

/**
 * @description:                                        output a series of cells in fifo
 * @param       {RING_FIFO_CB_T} *fifo
 * @param       {void} *data                            cells data [out]
 * @param       {unsigned short} number                 expect out number of cells
 * @return      {*}                                     number of successful output
 * @note       :
 */
unsigned short Ring_Fifo_out_cells(RING_FIFO_CB_T *fifo, void *data, unsigned short number)
{
    unsigned char *next;
    unsigned char *ptemp = fifo->out;
    unsigned short count_temp, count_temp_r;
    unsigned short ret;

    LOCK();

    if (fifo->curr_number > number)
    {
        ret = number;
    }
    else if (fifo->curr_number < number && fifo->curr_number > 0)
    {
        ret = fifo->curr_number;
    }
    else
    {
        ret = 0;
    }

    count_temp = fifo->cell_size * ret;
    next = fifo->out + count_temp;

    fifo->curr_number -= ret;

    if (next < fifo->end)
    {
        fifo->out = next;
        memcpy(data, ptemp, count_temp);
    }
    else
    {
        count_temp_r = fifo->end - fifo->in;
        next = fifo->start + count_temp - count_temp_r;
        fifo->out = next;
        memcpy(data, ptemp, count_temp_r);
        memcpy(((unsigned char *)data) + count_temp_r, fifo->start, count_temp - count_temp_r);
    }

    UNLOCK();

    return ret;
}
