/**
 * \file            lwgsm_sys.h
 * \brief           Main system include file which decides later include file
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
#ifndef LWGSM_HDR_MAIN_SYS_H
#define LWGSM_HDR_MAIN_SYS_H

#include <stdlib.h>
#include "lwgsm/lwgsm_opt.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \defgroup        LWGSM_SYS System functions
 * \brief           System based function for OS management, timings, etc
 * \{
 */

/**
 * \brief           Thread function prototype
 */
typedef void (*lwgsm_sys_thread_fn)(void*);

/* Include system port file from portable folder */
#include "lwgsm_sys_port.h"

/**
 * \anchor          LWGSM_SYS_CORE
 * \name            Main
 */

uint8_t     lwgsm_sys_init(void);
uint32_t    lwgsm_sys_now(void);

uint8_t     lwgsm_sys_protect(void);
uint8_t     lwgsm_sys_unprotect(void);

/**
 * \}
 */

/**
 * \anchor          LWGSM_SYS_MUTEX
 * \name            Mutex
 */

uint8_t     lwgsm_sys_mutex_create(lwgsm_sys_mutex_t* p);
uint8_t     lwgsm_sys_mutex_delete(lwgsm_sys_mutex_t* p);
uint8_t     lwgsm_sys_mutex_lock(lwgsm_sys_mutex_t* p);
uint8_t     lwgsm_sys_mutex_unlock(lwgsm_sys_mutex_t* p);
uint8_t     lwgsm_sys_mutex_isvalid(lwgsm_sys_mutex_t* p);
uint8_t     lwgsm_sys_mutex_invalid(lwgsm_sys_mutex_t* p);

/**
 * \}
 */

/**
 * \anchor          LWGSM_SYS_SEM
 * \name            Semaphores
 */

uint8_t     lwgsm_sys_sem_create(lwgsm_sys_sem_t* p, uint8_t cnt);
uint8_t     lwgsm_sys_sem_delete(lwgsm_sys_sem_t* p);
uint32_t    lwgsm_sys_sem_wait(lwgsm_sys_sem_t* p, uint32_t timeout);
uint8_t     lwgsm_sys_sem_release(lwgsm_sys_sem_t* p);
uint8_t     lwgsm_sys_sem_isvalid(lwgsm_sys_sem_t* p);
uint8_t     lwgsm_sys_sem_invalid(lwgsm_sys_sem_t* p);

/**
 * \}
 */

/**
 * \anchor          LWGSM_SYS_MBOX
 * \name            Message queues
 */

uint8_t     lwgsm_sys_mbox_create(lwgsm_sys_mbox_t* b, size_t size);
uint8_t     lwgsm_sys_mbox_delete(lwgsm_sys_mbox_t* b);
uint32_t    lwgsm_sys_mbox_put(lwgsm_sys_mbox_t* b, void* m);
uint32_t    lwgsm_sys_mbox_get(lwgsm_sys_mbox_t* b, void** m, uint32_t timeout);
uint8_t     lwgsm_sys_mbox_putnow(lwgsm_sys_mbox_t* b, void* m);
uint8_t     lwgsm_sys_mbox_getnow(lwgsm_sys_mbox_t* b, void** m);
uint8_t     lwgsm_sys_mbox_isvalid(lwgsm_sys_mbox_t* b);
uint8_t     lwgsm_sys_mbox_invalid(lwgsm_sys_mbox_t* b);

/**
 * \}
 */

/**
 * \anchor          LWGSM_SYS_THREAD
 * \name            Threads
 */

uint8_t     lwgsm_sys_thread_create(lwgsm_sys_thread_t* t, const char* name, lwgsm_sys_thread_fn thread_func, void* const arg, size_t stack_size, lwgsm_sys_thread_prio_t prio);
uint8_t     lwgsm_sys_thread_terminate(lwgsm_sys_thread_t* t);
uint8_t     lwgsm_sys_thread_yield(void);

/**
 * \}
 */

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWGSM_HDR_MAIN_LL_H */
