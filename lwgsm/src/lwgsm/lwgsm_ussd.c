/**
 * \file            lwgsm_ussd.c
 * \brief           Unstructured Supplementary Service Data
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
#include "lwgsm/lwgsm_ussd.h"
#include "lwgsm/lwgsm_mem.h"

#if LWGSM_CFG_USSD || __DOXYGEN__

/**
 * \brief           Run USSD command, such as `*123#` to get balance on SIM card
 * \param[in]       code: Code to run, such as `*123#`
 * \param[out]      resp: Pointer to array to save response
 * \param[in]       resp_len: Length of array, including string `NULL` termination
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_ussd_run(const char* code, char* resp, size_t resp_len,
             const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_ASSERT("code != NULL && strlen(code) > 0", code != NULL && strlen(code) > 0);
    LWGSM_ASSERT("resp != NULL", resp != NULL);
    LWGSM_ASSERT("resp_len != NULL", resp_len > 0);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_CUSD;
    LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CUSD_GET;
    LWGSM_MSG_VAR_REF(msg).msg.ussd.code = code;
    LWGSM_MSG_VAR_REF(msg).msg.ussd.resp = resp;
    LWGSM_MSG_VAR_REF(msg).msg.ussd.resp_len = resp_len;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 10000);
}

#endif /* LWGSM_CFG_USSD || __DOXYGEN__ */
