/**
 * \file            lwcell_device_info.c
 * \brief           Basic device information
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
#include "lwcell/lwcell_device_info.h"
#include "lwcell/lwcell_private.h"

/**
 * \brief           Get device manufacturer
 * \param[in]       manuf: Pointer to output string array to save manufacturer info
 * \param[in]       len: Length of string array including `NULL` termination
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_device_get_manufacturer(char* manuf, size_t len, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                               const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_ASSERT(manuf != NULL);
    LWCELL_ASSERT(len > 0);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CGMI_GET;
    LWCELL_MSG_VAR_REF(msg).msg.device_info.str = manuf;
    LWCELL_MSG_VAR_REF(msg).msg.device_info.len = len;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 10000);
}

/**
 * \brief           Get device model name
 * \param[in]       model: Pointer to output string array to save model info
 * \param[in]       len: Length of string array including `NULL` termination
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_device_get_model(char* model, size_t len, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                        const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_ASSERT(model != NULL);
    LWCELL_ASSERT(len > 0);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CGMM_GET;
    LWCELL_MSG_VAR_REF(msg).msg.device_info.str = model;
    LWCELL_MSG_VAR_REF(msg).msg.device_info.len = len;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 10000);
}

/**
 * \brief           Get device revision
 * \param[in]       rev: Pointer to output string array to save revision info
 * \param[in]       len: Length of string array including `NULL` termination
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_device_get_revision(char* rev, size_t len, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                           const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_ASSERT(rev != NULL);
    LWCELL_ASSERT(len > 0);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CGMR_GET;
    LWCELL_MSG_VAR_REF(msg).msg.device_info.str = rev;
    LWCELL_MSG_VAR_REF(msg).msg.device_info.len = len;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 10000);
}

/**
 * \brief           Get device serial number
 * \param[in]       serial: Pointer to output string array to save serial number info
 * \param[in]       len: Length of string array including `NULL` termination
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_device_get_serial_number(char* serial, size_t len, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                                const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_ASSERT(serial != NULL);
    LWCELL_ASSERT(len > 0);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CGSN_GET;
    LWCELL_MSG_VAR_REF(msg).msg.device_info.str = serial;
    LWCELL_MSG_VAR_REF(msg).msg.device_info.len = len;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 10000);
}
