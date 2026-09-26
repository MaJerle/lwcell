/**
 * \file            lwcell_ll.h
 * \brief           Low-level communication implementation
 */

/*
 * Copyright (c) 2026 Tilen MAJERLE
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
 * Version:         v0.1.2
 */
#ifndef LWCELL_LL_HDR_H
#define LWCELL_LL_HDR_H

#include "lwcell/lwcell_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \defgroup        LWCELL_LL Low-level functions
 * \brief           Low-level communication functions
 * \{
 */

/**
 * \brief           Callback function called from initialization process
 *
 * \note            This function may be called multiple times if AT baudrate is changed from application.
 *                  It is important that every configuration except AT baudrate is configured only once!
 *
 * \note            This function may be called from different threads in GSM stack when using OS.
 *                  When \ref LWCELL_CFG_INPUT_USE_PROCESS is set to 1, this function may be called from user UART
 * thread.
 *
 * \param[in,out]   ll: Pointer to \ref lwcell_ll_t structure to fill data for communication functions
 * \return          lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t lwcell_ll_init(lwcell_ll_t* ll);

/**
 * \brief           Callback function to de-init low-level communication part
 * \param[in,out]   ll: Pointer to \ref lwcell_ll_t structure to fill data for communication functions
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t lwcell_ll_deinit(lwcell_ll_t* ll);

/**
 * \brief           Wait until modem responds to AT autobaud sync attempts,
 *                  or until timeout expires.
 *
 * Implementation is platform-specific and must be provided by the
 * low-level driver (see \ref lwcell_ll_init).
 *
 * \param[in]       max_wait_ms: Maximum time to wait for first response, in ms
 * \param[in]       settle_ms: Extra time to wait after first response is seen
 * \return          `1` if module responded within max_wait_ms, `0` on timeout
 */
uint8_t lwcell_ll_wait_module_ready(uint32_t max_wait_ms, uint32_t settle_ms);

/**
 * \brief           Discard any pending/unprocessed data in the platform's
 *                  RX buffer and reset RX position tracking.
 *
 * Used after autobaud sync (cold power-up or warm reset via AT+CFUN=1,1)
 * to make sure lwcell starts parsing from a clean state, without any
 * leftover "OK" responses from the sync "AT\r" pings still pending.
 *
 * Implementation is platform-specific and must be provided by the
 * low-level driver (see \ref lwcell_ll_init).
 *
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t lwcell_ll_rx_clear(void);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWCELL_LL_HDR_H */
