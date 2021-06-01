/**
 * \file            lwgsm_sys_cmsis_os.c
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
#include "system/lwgsm_sys.h"
#include "cmsis_os.h"

#if !__DOXYGEN__

static osMutexId_t sys_mutex;

uint8_t
lwgsm_sys_init(void) {
    lwgsm_sys_mutex_create(&sys_mutex);
    return 1;
}

uint32_t
lwgsm_sys_now(void) {
    return osKernelSysTick();
}

uint8_t
lwgsm_sys_protect(void) {
    lwgsm_sys_mutex_lock(&sys_mutex);
    return 1;
}

uint8_t
lwgsm_sys_unprotect(void) {
    lwgsm_sys_mutex_unlock(&sys_mutex);
    return 1;
}

uint8_t
lwgsm_sys_mutex_create(lwgsm_sys_mutex_t* p) {
    const osMutexAttr_t attr = {
        .attr_bits = osMutexRecursive,
        .name = "lwgsm_mutex",
    };
    return (*p = osMutexNew(&attr)) != NULL;
}

uint8_t
lwgsm_sys_mutex_delete(lwgsm_sys_mutex_t* p) {
    return osMutexDelete(*p) == osOK;
}

uint8_t
lwgsm_sys_mutex_lock(lwgsm_sys_mutex_t* p) {
    return osMutexAcquire(*p, osWaitForever) == osOK;
}

uint8_t
lwgsm_sys_mutex_unlock(lwgsm_sys_mutex_t* p) {
    return osMutexRelease(*p) == osOK;
}

uint8_t
lwgsm_sys_mutex_isvalid(lwgsm_sys_mutex_t* p) {
    return p != NULL && *p != NULL;
}

uint8_t
lwgsm_sys_mutex_invalid(lwgsm_sys_mutex_t* p) {
    *p = LWGSM_SYS_MUTEX_NULL;
    return 1;
}

uint8_t
lwgsm_sys_sem_create(lwgsm_sys_sem_t* p, uint8_t cnt) {
    const osSemaphoreAttr_t attr = {
        .name = "lwgsm_sem",
    };
    return (*p = osSemaphoreNew(1, cnt > 0 ? 1 : 0, &attr)) != NULL;
}

uint8_t
lwgsm_sys_sem_delete(lwgsm_sys_sem_t* p) {
    return osSemaphoreDelete(*p) == osOK;
}

uint32_t
lwgsm_sys_sem_wait(lwgsm_sys_sem_t* p, uint32_t timeout) {
    uint32_t tick = osKernelSysTick();
    return (osSemaphoreAcquire(*p, timeout == 0 ? osWaitForever : timeout) == osOK) ? (osKernelSysTick() - tick) : LWGSM_SYS_TIMEOUT;
}

uint8_t
lwgsm_sys_sem_release(lwgsm_sys_sem_t* p) {
    return osSemaphoreRelease(*p) == osOK;
}

uint8_t
lwgsm_sys_sem_isvalid(lwgsm_sys_sem_t* p) {
    return p != NULL && *p != NULL;
}

uint8_t
lwgsm_sys_sem_invalid(lwgsm_sys_sem_t* p) {
    *p = LWGSM_SYS_SEM_NULL;
    return 1;
}

uint8_t
lwgsm_sys_mbox_create(lwgsm_sys_mbox_t* b, size_t size) {
    const osMessageQueueAttr_t attr = {
        .name = "lwgsm_mbox",
    };
    return (*b = osMessageQueueNew(size, sizeof(void*), &attr)) != NULL;
}

uint8_t
lwgsm_sys_mbox_delete(lwgsm_sys_mbox_t* b) {
    if (osMessageQueueGetCount(*b) > 0) {
        return 0;
    }
    return osMessageQueueDelete(*b) == osOK;
}

uint32_t
lwgsm_sys_mbox_put(lwgsm_sys_mbox_t* b, void* m) {
    uint32_t tick = osKernelSysTick();
    return osMessageQueuePut(*b, &m, 0, osWaitForever) == osOK ? (osKernelSysTick() - tick) : LWGSM_SYS_TIMEOUT;
}

uint32_t
lwgsm_sys_mbox_get(lwgsm_sys_mbox_t* b, void** m, uint32_t timeout) {
    uint32_t tick = osKernelSysTick();
    return osMessageQueueGet(*b, m, NULL, timeout == 0 ? osWaitForever : timeout) == osOK ? (osKernelSysTick() - tick) : LWGSM_SYS_TIMEOUT;
}

uint8_t
lwgsm_sys_mbox_putnow(lwgsm_sys_mbox_t* b, void* m) {
    return osMessageQueuePut(*b, &m, 0, 0) == osOK;
}

uint8_t
lwgsm_sys_mbox_getnow(lwgsm_sys_mbox_t* b, void** m) {
    return osMessageQueueGet(*b, m, NULL, 0) == osOK;
}

uint8_t
lwgsm_sys_mbox_isvalid(lwgsm_sys_mbox_t* b) {
    return b != NULL && *b != NULL;
}

uint8_t
lwgsm_sys_mbox_invalid(lwgsm_sys_mbox_t* b) {
    *b = LWGSM_SYS_MBOX_NULL;
    return 1;
}

uint8_t
lwgsm_sys_thread_create(lwgsm_sys_thread_t* t, const char* name, lwgsm_sys_thread_fn thread_func, void* const arg, size_t stack_size, lwgsm_sys_thread_prio_t prio) {
    lwgsm_sys_thread_t id;
    const osThreadAttr_t thread_attr = {
        .name = (char*)name,
        .priority = (osPriority)prio,
        .stack_size = stack_size > 0 ? stack_size : LWGSM_SYS_THREAD_SS
    };

    id = osThreadNew(thread_func, arg, &thread_attr);
    if (t != NULL) {
        *t = id;
    }
    return id != NULL;
}

uint8_t
lwgsm_sys_thread_terminate(lwgsm_sys_thread_t* t) {
    if (t != NULL) {
        osThreadTerminate(*t);
    } else {
        osThreadExit();
    }
    return 1;
}

uint8_t
lwgsm_sys_thread_yield(void) {
    osThreadYield();
    return 1;
}

#endif /* !__DOXYGEN__ */
