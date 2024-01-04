/**
 * \file            lwcell_ussd.c
 * \brief           Unstructured Supplementary Service Data
 */

/*
 * Copyright (c) 2024 Tilen MAJERLE
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
 * Version:         v0.1.1
 */
#include "lwcell/lwcell_ussd.h"
#include "lwcell/lwcell_private.h"

#if LWCELL_CFG_USSD || __DOXYGEN__

/**
 * \brief           Run USSD command, such as `*123#` to get balance on SIM card
 * \param[in]       code: Code to run, such as `*123#`
 * \param[out]      resp: Pointer to array to save response
 * \param[in]       resp_len: Length of array, including string `NULL` termination
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_ussd_run(const char* code, char* resp, size_t resp_len, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_ASSERT(code != NULL && strlen(code) > 0);
    LWCELL_ASSERT(resp != NULL);
    LWCELL_ASSERT(resp_len > 0);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CUSD;
    LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CUSD_GET;
    LWCELL_MSG_VAR_REF(msg).msg.ussd.code = code;
    LWCELL_MSG_VAR_REF(msg).msg.ussd.resp = resp;
    LWCELL_MSG_VAR_REF(msg).msg.ussd.resp_len = resp_len;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 10000);
}

#endif /* LWCELL_CFG_USSD || __DOXYGEN__ */
