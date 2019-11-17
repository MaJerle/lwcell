/**
 * \file            gsm_sys.h
 * \brief           Main system include file which decides later include file
 */

/*
 * Copyright (c) 2019 Tilen MAJERLE
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
 * This file is part of GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.6.0
 */
#ifndef GSM_HDR_MAIN_SYS_H
#define GSM_HDR_MAIN_SYS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "gsm_config.h"

/**
 * \ingroup         GSM_PORT
 * \defgroup        GSM_SYS System functions
 * \brief           System based function for OS management, timings, etc
 * \{
 */

/**
 * \brief           Thread function prototype
 */
typedef void (*gsm_sys_thread_fn)(void *);

/**
 * \anchor          GSM_SYS_PORTS
 * \name            System ports
 * \{
 *
 * List of already available system ports.
 * Configure \ref GSM_CFG_SYS_PORT with one of these values to use preconfigured ports
 */

#define GSM_SYS_PORT_CMSIS_OS               1   /*!< CMSIS-OS based port for OS systems capable of ARM CMSIS standard */
#define GSM_SYS_PORT_WIN32                  2   /*!< WIN32 based port to use GSM library with Windows applications */
#define GSM_SYS_PORT_CMSIS_OS2              3   /*!< CMSIS-OS v2 based port for OS systems capable of ARM CMSIS standard */
#define GSM_SYS_PORT_USER                   99  /*!< User custom implementation.
                                                    When port is selected to user mode, user must provide "gsm_sys_user.h" file,
                                                    which is not provided with library. Refer to `system/gsm_sys_template.h` file for more information
                                                    */

/**
 * \}
 */

/* Decide which port to include */
#if GSM_CFG_SYS_PORT == GSM_SYS_PORT_CMSIS_OS
#include "system/gsm_sys_cmsis_os.h"
#elif GSM_CFG_SYS_PORT == GSM_SYS_PORT_CMSIS_OS2
#include "system/esp_sys_cmsis_os2.h"
#elif GSM_CFG_SYS_PORT == GSM_SYS_PORT_WIN32
#include "system/gsm_sys_win32.h"
#elif GSM_CFG_SYS_PORT == GSM_SYS_PORT_USER
#include "gsm_sys_user.h"
#endif

uint8_t     gsm_sys_init(void);
uint32_t    gsm_sys_now(void);

uint8_t     gsm_sys_protect(void);
uint8_t     gsm_sys_unprotect(void);

uint8_t     gsm_sys_mutex_create(gsm_sys_mutex_t* p);
uint8_t     gsm_sys_mutex_delete(gsm_sys_mutex_t* p);
uint8_t     gsm_sys_mutex_lock(gsm_sys_mutex_t* p);
uint8_t     gsm_sys_mutex_unlock(gsm_sys_mutex_t* p);
uint8_t     gsm_sys_mutex_isvalid(gsm_sys_mutex_t* p);
uint8_t     gsm_sys_mutex_invalid(gsm_sys_mutex_t* p);

uint8_t     gsm_sys_sem_create(gsm_sys_sem_t* p, uint8_t cnt);
uint8_t     gsm_sys_sem_delete(gsm_sys_sem_t* p);
uint32_t    gsm_sys_sem_wait(gsm_sys_sem_t* p, uint32_t timeout);
uint8_t     gsm_sys_sem_release(gsm_sys_sem_t* p);
uint8_t     gsm_sys_sem_isvalid(gsm_sys_sem_t* p);
uint8_t     gsm_sys_sem_invalid(gsm_sys_sem_t* p);

uint8_t     gsm_sys_mbox_create(gsm_sys_mbox_t* b, size_t size);
uint8_t     gsm_sys_mbox_delete(gsm_sys_mbox_t* b);
uint32_t    gsm_sys_mbox_put(gsm_sys_mbox_t* b, void* m);
uint32_t    gsm_sys_mbox_get(gsm_sys_mbox_t* b, void** m, uint32_t timeout);
uint8_t     gsm_sys_mbox_putnow(gsm_sys_mbox_t* b, void* m);
uint8_t     gsm_sys_mbox_getnow(gsm_sys_mbox_t* b, void** m);
uint8_t     gsm_sys_mbox_isvalid(gsm_sys_mbox_t* b);
uint8_t     gsm_sys_mbox_invalid(gsm_sys_mbox_t* b);

uint8_t     gsm_sys_thread_create(gsm_sys_thread_t* t, const char* name, gsm_sys_thread_fn thread_func, void* const arg, size_t stack_size, gsm_sys_thread_prio_t prio);
uint8_t     gsm_sys_thread_terminate(gsm_sys_thread_t* t);
uint8_t     gsm_sys_thread_yield(void);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GSM_HDR_MAIN_LL_H */
