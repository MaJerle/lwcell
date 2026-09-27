/**
 * \file            lwcell_ll.h
 * \brief           Low-level communication implementation
 */

/*
 * Copyright (c) 2024 Tilen MAJERLE
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of LwCELL - Lightweight cellular modem AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 *                  Harald LESCHNER <picohari@googlemail.com>
 * Version:         v0.1.2
 */
#ifndef __LWCELL_LL_STM32_HAL_H__
#define __LWCELL_LL_STM32_HAL_H__

#include "cmsis_os2.h"

/* Message queue ID */
extern osMessageQueueId_t usart_rx_dma_queue_id;

/* Set once lwcell_init() succeeded - other tasks can wait on this
 * before calling any lwcell_*() API function */
extern osEventFlagsId_t lwcell_ready_evt_id;
#define LWCELL_READY_FLAG (1U << 0)

/* Thread function prototypes */
void LWCELL_thread(void* arg);

#endif /* __LWCELL_LL_STM32_HAL_H__ */