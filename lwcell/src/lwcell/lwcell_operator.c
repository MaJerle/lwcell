/**
 * \file            lwcell_operator.c
 * \brief           Operator API functions
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
#include "lwcell/lwcell_operator.h"
#include "lwcell/lwcell_private.h"

/**
 * \brief           Get current operator
 * \param[out]      curr: Pointer to output variable to save info about current operator
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_operator_get(lwcell_operator_curr_t* curr, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                    const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_COPS_GET;
    LWCELL_MSG_VAR_REF(msg).msg.cops_get.curr = curr;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 2000);
}

/**
 * \brief           Set current operator
 * \param[in]       mode: Operator mode. This parameter can be a value of \ref lwcell_operator_mode_t enumeration
 * \param[in]       format: Operator data format. This parameter can be a value of \ref lwcell_operator_format_t enumeration
 * \param[in]       name: Operator name. This parameter must be valid according to `format` parameter
 * \param[in]       num: Operator number. This parameter must be valid according to `format` parameter
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_operator_set(lwcell_operator_mode_t mode, lwcell_operator_format_t format, const char* name, uint32_t num,
                    const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    if (mode != LWCELL_OPERATOR_MODE_AUTO) { /* Check parameters only if non-auto mode */
        LWCELL_ASSERT(format < LWCELL_OPERATOR_FORMAT_INVALID);
        if (format != LWCELL_OPERATOR_FORMAT_NUMBER) {
            LWCELL_ASSERT(name != NULL);
        }
    }

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_COPS_SET;

    LWCELL_MSG_VAR_REF(msg).msg.cops_set.mode = mode;
    LWCELL_MSG_VAR_REF(msg).msg.cops_set.format = format;
    LWCELL_MSG_VAR_REF(msg).msg.cops_set.name = name;
    LWCELL_MSG_VAR_REF(msg).msg.cops_set.num = num;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 2000);
}

/**
 * \brief           Scan for available operators
 * \param[in]       ops: Pointer to array to write found operators
 * \param[in]       opsl: Length of input array in units of elements
 * \param[out]      opf: Pointer to ouput variable to save number of operators found
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_operator_scan(lwcell_operator_t* ops, size_t opsl, size_t* opf, const lwcell_api_cmd_evt_fn evt_fn,
                     void* const evt_arg, const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    if (opf != NULL) {
        *opf = 0;
    }

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_COPS_GET_OPT;
    LWCELL_MSG_VAR_REF(msg).msg.cops_scan.ops = ops;
    LWCELL_MSG_VAR_REF(msg).msg.cops_scan.opsl = opsl;
    LWCELL_MSG_VAR_REF(msg).msg.cops_scan.opf = opf;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 120000);
}
