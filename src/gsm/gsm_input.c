/**
 * \file            gsm_input.c
 * \brief           Wrapper for passing input data to stack
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
 * This file is part of GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 */
#include "gsm/gsm_private.h"
#include "gsm/gsm.h"
#include "gsm/gsm_input.h"
#include "gsm/gsm_buff.h"

static uint32_t gsm_recv_total_len;
static uint32_t gsm_recv_calls;

#if !GSM_CFG_INPUT_USE_PROCESS || __DOXYGEN__

/**
 * \brief           Write data to input buffer
 * \note            \ref GSM_CFG_INPUT_USE_PROCESS must be disabled to use this function
 * \param[in]       data: Pointer to data to write
 * \param[in]       len: Number of data elements in units of bytes
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_input(const void* data, size_t len) {
    if (!gsm.status.f.initialized || gsm.buff.buff == NULL) {
        return gsmERR;
    }
    gsm_buff_write(&gsm.buff, data, len);       /* Write data to buffer */
    gsm_sys_mbox_putnow(&gsm.mbox_process, NULL);   /* Write empty box, don't care if write fails */
    gsm_recv_total_len += len;                  /* Update total number of received bytes */
    gsm_recv_calls++;                           /* Update number of calls */
    return gsmOK;
}

#endif /* !GSM_CFG_INPUT_USE_PROCESS || __DOXYGEN__ */

#if GSM_CFG_INPUT_USE_PROCESS || __DOXYGEN__

/**
 * \brief           Process input data directly without writing it to input buffer
 * \note            This function may only be used when in OS mode,
 *                  where single thread is dedicated for input read of AT receive
 *
 * \note            \ref GSM_CFG_INPUT_USE_PROCESS must be enabled to use this function
 *
 * \param[in]       data: Pointer to received data to be processed
 * \param[in]       len: Length of data to process in units of bytes
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_input_process(const void* data, size_t len) {
    gsmr_t res;

    if (!gsm.status.f.initialized) {
        return gsmERR;
    }

    gsm_recv_total_len += len;                  /* Update total number of received bytes */
    gsm_recv_calls++;                           /* Update number of calls */

    gsm_core_lock();
    res = gsmi_process(data, len);              /* Process input data */
    gsm_core_unlock();
    return res;
}

#endif /* GSM_CFG_INPUT_USE_PROCESS || __DOXYGEN__ */
