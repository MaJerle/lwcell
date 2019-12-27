/**
 * \file            gsm_ussd.c
 * \brief           Unstructured Supplementary Service Data
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
 * Version:         $_version_$
 */
#include "gsm/gsm_private.h"
#include "gsm/gsm_ussd.h"
#include "gsm/gsm_mem.h"

#if GSM_CFG_USSD || __DOXYGEN__

/**
 * \brief           Run USSD command, such as `*123#` to get balance on SIM card
 * \param[in]       code: Code to run, such as `*123#`
 * \param[out]      resp: Pointer to array to save response
 * \param[in]       resp_len: Length of array, including string `NULL` termination
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_ussd_run(const char* code, char* resp, size_t resp_len,
            const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);

    GSM_ASSERT("code != NULL && strlen(code) > 0", code != NULL && strlen(code) > 0);
    GSM_ASSERT("resp != NULL", resp != NULL);
    GSM_ASSERT("resp_len != NULL", resp_len > 0);

    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_CUSD;
    GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CUSD_GET;
    GSM_MSG_VAR_REF(msg).msg.ussd.code = code;
    GSM_MSG_VAR_REF(msg).msg.ussd.resp = resp;
    GSM_MSG_VAR_REF(msg).msg.ussd.resp_len = resp_len;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 10000);
}

#endif /* GSM_CFG_USSD || __DOXYGEN__ */
