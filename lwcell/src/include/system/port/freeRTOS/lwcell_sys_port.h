/**
 * \file            lwcell_sys_port.h
 * \brief           System dependent functions for FreeRTOS based operating system
 */

/*
 * Copyright (c) 2021 Tilen MAJERLE and Ilya Kargapolov
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
 * Authors:          Tilen MAJERLE <tilen@majerle.eu>,
 *                   Ilya Kargapolov <d3vil.st@gmail.com>
 * Version:         v0.1.1
 */
#ifndef LWCELL_SYSTEM_PORT_HDR_H
#define LWCELL_SYSTEM_PORT_HDR_H

#include <stdint.h>
#include <stdlib.h>
#include "lwcell/lwcell_opt.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \addtogroup      LWCELL_SYS
 * \{
 */

#if LWCELL_CFG_OS && !__DOXYGEN__

/* Include any OS specific features */
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

/**
 * \brief           System mutex type
 *
 * It is used by middleware as base type of mutex.
 */
typedef SemaphoreHandle_t lwcell_sys_mutex_t;

/**
 * \brief           System semaphore type
 *
 * It is used by middleware as base type of mutex.
 */
typedef SemaphoreHandle_t lwcell_sys_sem_t;

/**
 * \brief           System message queue type
 *
 * It is used by middleware as base type of mutex.
 */
typedef QueueHandle_t lwcell_sys_mbox_t;

/**
 * \brief           System thread ID type
 */
typedef TaskHandle_t lwcell_sys_thread_t;

/**
 * \brief           System thread priority type
 *
 * It is used as priority type for system function,
 * to start new threads by middleware.
 */
typedef UBaseType_t lwcell_sys_thread_prio_t;

/**
 * \brief           Mutex invalid value
 *
 * Value assigned to \ref lwcell_sys_mutex_t type when it is not valid.
 */
#define LWCELL_SYS_MUTEX_NULL  ((lwcell_sys_mutex_t)0)

/**
 * \brief           Semaphore invalid value
 *
 * Value assigned to \ref lwcell_sys_sem_t type when it is not valid.
 */
#define LWCELL_SYS_SEM_NULL    ((lwcell_sys_sem_t)0)

/**
 * \brief           Message box invalid value
 *
 * Value assigned to \ref lwcell_sys_mbox_t type when it is not valid.
 */
#define LWCELL_SYS_MBOX_NULL   ((lwcell_sys_mbox_t)0)

/**
 * \brief           OS timeout value
 *
 * Value returned by operating system functions (mutex wait, sem wait, mbox wait)
 * when it returns timeout and does not give valid value to application
 */
#define LWCELL_SYS_TIMEOUT     ((uint32_t)portMAX_DELAY)

/**
 * \brief           Default thread priority value used by middleware to start built-in threads
 *
 * Threads can well operate with normal (default) priority and do not require
 * any special feature in terms of priority for prioer operation.
 */
#define LWCELL_SYS_THREAD_PRIO (tskIDLE_PRIORITY + 1)

/**
 * \brief           Stack size in units of bytes for system threads
 *
 * It is used as default stack size for all built-in threads.
 */
#define LWCELL_SYS_THREAD_SS   (1024)

#endif /* LWCELL_CFG_OS && !__DOXYGEN__ */

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWCELL_SYSTEM_PORT_HDR_H */
