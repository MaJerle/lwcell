/**
 * \file            lwcell_sys_port.h
 * \brief           System dependent functions for CMSIS-OS based operating system
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
 * Version:         v0.1.1
 */
#ifndef LWCELL_SYSTEM_PORT_HDR_H
#define LWCELL_SYSTEM_PORT_HDR_H

#include <stdint.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "lwcell/lwcell_opt.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if LWCELL_CFG_OS && !__DOXYGEN__

typedef osMutexId_t lwcell_sys_mutex_t;
typedef osSemaphoreId_t lwcell_sys_sem_t;
typedef osMessageQueueId_t lwcell_sys_mbox_t;
typedef osThreadId_t lwcell_sys_thread_t;
typedef osPriority_t lwcell_sys_thread_prio_t;

#define LWCELL_SYS_MUTEX_NULL  ((lwcell_sys_mutex_t)0)
#define LWCELL_SYS_SEM_NULL    ((lwcell_sys_sem_t)0)
#define LWCELL_SYS_MBOX_NULL   ((lwcell_sys_mbox_t)0)
#define LWCELL_SYS_TIMEOUT     ((uint32_t)osWaitForever)
#define LWCELL_SYS_THREAD_PRIO (osPriorityNormal)
#define LWCELL_SYS_THREAD_SS   (512)

#endif /* LWCELL_CFG_OS && !__DOXYGEN__ */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWCELL_SYSTEM_PORT_HDR_H */
