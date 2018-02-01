/**
 * \file            gsm_sys.h
 * \brief           Main system include file which decides later include file
 */

/*
 * Copyright (c) 2018 Tilen Majerle
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
 * This file is part of GSM-AT.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 */
#ifndef __GSM_MAIN_SYS_H
#define __GSM_MAIN_SYS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "gsm_config.h"
    
/**
 * \addtogroup      GSM_SYS
 * \{
 */
 
/**
 * \brief           Thread function prototype
 */
typedef void (*gsm_sys_thread_fn)(void *);

/**
 * \name            GSM_SYS_PORTS System ports
 * \{
 *
 * List of already available system ports. 
 * Configure \ref GSM_CFG_SYS_PORT with one of these values to use preconfigured ports
 */
 
#define GSM_SYS_PORT_CMSIS_OS               1   /*!< CMSIS-OS based port for OS systems capable of ARM CMSIS standard */
#define GSM_SYS_PORT_WIN32                  2   /*!< WIN32 based port to use GSM library with Windows applications */

/**
 * \}
 */

/**
 * \}
 */

/* Decide which port to include */
#if GSM_CFG_SYS_PORT == GSM_SYS_PORT_CMSIS_OS
#include "system/gsm_sys_cmsis_os.h"
#elif GSM_CFG_SYS_PORT == GSM_SYS_PORT_WIN32
#include "system/gsm_sys_win32.h"
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GSM_MAIN_LL_H */
