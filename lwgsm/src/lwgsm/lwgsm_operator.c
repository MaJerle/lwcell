/**
 * \file            lwgsm_operator.c
 * \brief           Operator API functions
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
#include "lwgsm/lwgsm_operator.h"
#include "lwgsm/lwgsm_mem.h"

/**
 * \brief           Get current operator
 * \param[out]      curr: Pointer to output variable to save info about current operator
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_operator_get(lwgsm_operator_curr_t* curr,
                 const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_COPS_GET;
    LWGSM_MSG_VAR_REF(msg).msg.cops_get.curr = curr;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 2000);
}

/**
 * \brief           Set current operator
 * \param[in]       mode: Operator mode. This parameter can be a value of \ref lwgsm_operator_mode_t enumeration
 * \param[in]       format: Operator data format. This parameter can be a value of \ref lwgsm_operator_format_t enumeration
 * \param[in]       name: Operator name. This parameter must be valid according to `format` parameter
 * \param[in]       num: Operator number. This parameter must be valid according to `format` parameter
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_operator_set(lwgsm_operator_mode_t mode, lwgsm_operator_format_t format, const char* name, uint32_t num,
                 const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    if (mode != LWGSM_OPERATOR_MODE_AUTO) {     /* Check parameters only if non-auto mode */
        LWGSM_ASSERT("format < LWGSM_OPERATOR_FORMAT_INVALID", format < LWGSM_OPERATOR_FORMAT_INVALID);
        if (format != LWGSM_OPERATOR_FORMAT_NUMBER) {
            LWGSM_ASSERT("name != NULL", name != NULL);
        }
    }

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_COPS_SET;

    LWGSM_MSG_VAR_REF(msg).msg.cops_set.mode = mode;
    LWGSM_MSG_VAR_REF(msg).msg.cops_set.format = format;
    LWGSM_MSG_VAR_REF(msg).msg.cops_set.name = name;
    LWGSM_MSG_VAR_REF(msg).msg.cops_set.num = num;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 2000);
}

/**
 * \brief           Scan for available operators
 * \param[in]       ops: Pointer to array to write found operators
 * \param[in]       opsl: Length of input array in units of elements
 * \param[out]      opf: Pointer to ouput variable to save number of operators found
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_operator_scan(lwgsm_operator_t* ops, size_t opsl, size_t* opf,
                  const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    if (opf != NULL) {
        *opf = 0;
    }

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_COPS_GET_OPT;
    LWGSM_MSG_VAR_REF(msg).msg.cops_scan.ops = ops;
    LWGSM_MSG_VAR_REF(msg).msg.cops_scan.opsl = opsl;
    LWGSM_MSG_VAR_REF(msg).msg.cops_scan.opf = opf;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 120000);
}
