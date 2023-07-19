/**
 * \file            lwcell_input.c
 * \brief           Wrapper for passing input data to stack
 */

/*
 * Copyright (c) 2023 Tilen MAJERLE
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
 * This file is part of LwCELL - Lightweight GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.1
 */
#include "lwcell/lwcell_input.h"
#include "lwcell/lwcell_buff.h"
#include "lwcell/lwcell_private.h"

static uint32_t lwcell_recv_total_len;
static uint32_t lwcell_recv_calls;

#if !LWCELL_CFG_INPUT_USE_PROCESS || __DOXYGEN__

/**
 * \brief           Write data to input buffer
 * \note            \ref LWCELL_CFG_INPUT_USE_PROCESS must be disabled to use this function
 * \param[in]       data: Pointer to data to write
 * \param[in]       len: Number of data elements in units of bytes
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_input(const void* data, size_t len) {
    if (!lwcell.status.f.initialized || lwcell.buff.buff == NULL) {
        return lwcellERR;
    }
    lwcell_buff_write(&lwcell.buff, data, len);         /* Write data to buffer */
    lwcell_sys_mbox_putnow(&lwcell.mbox_process, NULL); /* Write empty box, don't care if write fails */
    lwcell_recv_total_len += len;                      /* Update total number of received bytes */
    ++lwcell_recv_calls;                               /* Update number of calls */
    return lwcellOK;
}

#endif /* !LWCELL_CFG_INPUT_USE_PROCESS || __DOXYGEN__ */

#if LWCELL_CFG_INPUT_USE_PROCESS || __DOXYGEN__

/**
 * \brief           Process input data directly without writing it to input buffer
 * \note            This function may only be used when in OS mode,
 *                  where single thread is dedicated for input read of AT receive
 *
 * \note            \ref LWCELL_CFG_INPUT_USE_PROCESS must be enabled to use this function
 *
 * \param[in]       data: Pointer to received data to be processed
 * \param[in]       len: Length of data to process in units of bytes
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_input_process(const void* data, size_t len) {
    lwcellr_t res;

    if (!lwcell.status.f.initialized) {
        return lwcellERR;
    }

    lwcell_recv_total_len += len; /* Update total number of received bytes */
    ++lwcell_recv_calls;          /* Update number of calls */

    lwcell_core_lock();
    res = lwcelli_process(data, len); /* Process input data */
    lwcell_core_unlock();
    return res;
}

#endif /* LWCELL_CFG_INPUT_USE_PROCESS || __DOXYGEN__ */
