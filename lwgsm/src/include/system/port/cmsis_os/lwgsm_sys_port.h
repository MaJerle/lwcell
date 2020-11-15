/**
 * \file            lwgsm_sys_port.h
 * \brief           System dependent functions for CMSIS-OS based operating system
 */

/*
 * Copyright (c) 2020 Tilen MAJERLE
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
 * This file is part of LwGSM - Lightweight GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.0
 */
#ifndef LWGSM_HDR_SYSTEM_PORT_H
#define LWGSM_HDR_SYSTEM_PORT_H

#include <stdint.h>
#include <stdlib.h>
#include "lwgsm/lwgsm_opt.h"
#include "cmsis_os.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if LWGSM_CFG_OS && !__DOXYGEN__

typedef osMutexId_t                 lwgsm_sys_mutex_t;
typedef osSemaphoreId_t             lwgsm_sys_sem_t;
typedef osMessageQueueId_t          lwgsm_sys_mbox_t;
typedef osThreadId_t                lwgsm_sys_thread_t;
typedef osPriority_t                lwgsm_sys_thread_prio_t;

#define LWGSM_SYS_MUTEX_NULL          ((lwgsm_sys_mutex_t)0)
#define LWGSM_SYS_SEM_NULL            ((lwgsm_sys_sem_t)0)
#define LWGSM_SYS_MBOX_NULL           ((lwgsm_sys_mbox_t)0)
#define LWGSM_SYS_TIMEOUT             ((uint32_t)osWaitForever)
#define LWGSM_SYS_THREAD_PRIO         (osPriorityNormal)
#define LWGSM_SYS_THREAD_SS           (512)

#endif /* LWGSM_CFG_OS && !__DOXYGEN__ */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWGSM_HDR_SYSTEM_PORT_H */
