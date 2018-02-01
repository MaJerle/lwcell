/**	
 * \file            gsm_timeout.h
 * \brief           Timeout manager
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
#ifndef __GSM_TIMEOUT_H
#define __GSM_TIMEOUT_H

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif

#include "gsm/gsm.h"

/**
 * \addtogroup      GSM
 * \{
 */
    
/**
 * \defgroup        GSM_TIMEOUT Timeout manager
 * \brief           Timeout manager
 * \{
 */

/**
 * \brief           Timeout callback function prototype
 */
typedef void (*gsm_timeout_fn_t)(void *);

/**
 * \brief           Timeout structure
 */
typedef struct gsm_timeout_t {
    struct gsm_timeout_t* next;                 /*!< Pointer to next timeout entry */
    uint32_t time;                              /*!< Time difference from previous entry */
    void* arg;                                  /*!< Argument to pass to callback function */
    gsm_timeout_fn_t fn;                        /*!< Callback function for timeout */
} gsm_timeout_t;

#ifdef GSM_INTERNAL
uint32_t        gsmi_get_from_mbox_with_timeout_checks(gsm_sys_mbox_t* b, void** m, uint32_t timeout);
#endif /* GSM_INTERNAL */

gsmr_t          gsm_timeout_add(uint32_t time, void (*cb)(void *), void* arg);
gsmr_t          gsm_timeout_remove(gsm_timeout_fn_t fn);
    
/**
 * \}
 */
    
/**
 * \}
 */

/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif /* __GSM_TIMEOUT_H */
