/**
 * \file            lwgsm_input.c
 * \brief           Wrapper for passing input data to stack
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
#include "lwgsm/lwgsm_private.h"
#include "lwgsm/lwgsm.h"
#include "lwgsm/lwgsm_input.h"
#include "lwgsm/lwgsm_buff.h"

static uint32_t lwgsm_recv_total_len;
static uint32_t lwgsm_recv_calls;

#if !LWGSM_CFG_INPUT_USE_PROCESS || __DOXYGEN__

/**
 * \brief           Write data to input buffer
 * \note            \ref LWGSM_CFG_INPUT_USE_PROCESS must be disabled to use this function
 * \param[in]       data: Pointer to data to write
 * \param[in]       len: Number of data elements in units of bytes
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_input(const void* data, size_t len) {
    if (!lwgsm.status.f.initialized || lwgsm.buff.buff == NULL) {
        return lwgsmERR;
    }
    lwgsm_buff_write(&lwgsm.buff, data, len);   /* Write data to buffer */
    lwgsm_sys_mbox_putnow(&lwgsm.mbox_process, NULL);   /* Write empty box, don't care if write fails */
    lwgsm_recv_total_len += len;                /* Update total number of received bytes */
    ++lwgsm_recv_calls;                         /* Update number of calls */
    return lwgsmOK;
}

#endif /* !LWGSM_CFG_INPUT_USE_PROCESS || __DOXYGEN__ */

#if LWGSM_CFG_INPUT_USE_PROCESS || __DOXYGEN__

/**
 * \brief           Process input data directly without writing it to input buffer
 * \note            This function may only be used when in OS mode,
 *                  where single thread is dedicated for input read of AT receive
 *
 * \note            \ref LWGSM_CFG_INPUT_USE_PROCESS must be enabled to use this function
 *
 * \param[in]       data: Pointer to received data to be processed
 * \param[in]       len: Length of data to process in units of bytes
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_input_process(const void* data, size_t len) {
    lwgsmr_t res;

    if (!lwgsm.status.f.initialized) {
        return lwgsmERR;
    }

    lwgsm_recv_total_len += len;                /* Update total number of received bytes */
    ++lwgsm_recv_calls;                         /* Update number of calls */

    lwgsm_core_lock();
    res = lwgsmi_process(data, len);            /* Process input data */
    lwgsm_core_unlock();
    return res;
}

#endif /* LWGSM_CFG_INPUT_USE_PROCESS || __DOXYGEN__ */
