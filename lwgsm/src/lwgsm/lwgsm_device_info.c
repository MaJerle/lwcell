/**
 * \file            lwgsm_device_info.c
 * \brief           Basic device information
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
#include "lwgsm/lwgsm_device_info.h"
#include "lwgsm/lwgsm_mem.h"

/**
 * \brief           Get device manufacturer
 * \param[in]       manuf: Pointer to output string array to save manufacturer info
 * \param[in]       len: Length of string array including `NULL` termination
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_device_get_manufacturer(char* manuf, size_t len, const lwgsm_api_cmd_evt_fn evt_fn,
                            void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_ASSERT("manuf != NULL", manuf != NULL);
    LWGSM_ASSERT("len > 0", len > 0);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_CGMI_GET;
    LWGSM_MSG_VAR_REF(msg).msg.device_info.str = manuf;
    LWGSM_MSG_VAR_REF(msg).msg.device_info.len = len;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 10000);
}

/**
 * \brief           Get device model name
 * \param[in]       model: Pointer to output string array to save model info
 * \param[in]       len: Length of string array including `NULL` termination
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_device_get_model(char* model, size_t len, const lwgsm_api_cmd_evt_fn evt_fn,
                     void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_ASSERT("model != NULL", model != NULL);
    LWGSM_ASSERT("len > 0", len > 0);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_CGMM_GET;
    LWGSM_MSG_VAR_REF(msg).msg.device_info.str = model;
    LWGSM_MSG_VAR_REF(msg).msg.device_info.len = len;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 10000);
}

/**
 * \brief           Get device revision
 * \param[in]       rev: Pointer to output string array to save revision info
 * \param[in]       len: Length of string array including `NULL` termination
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_device_get_revision(char* rev, size_t len, const lwgsm_api_cmd_evt_fn evt_fn,
                        void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_ASSERT("rev != NULL", rev != NULL);
    LWGSM_ASSERT("len > 0", len > 0);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_CGMR_GET;
    LWGSM_MSG_VAR_REF(msg).msg.device_info.str = rev;
    LWGSM_MSG_VAR_REF(msg).msg.device_info.len = len;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 10000);
}

/**
 * \brief           Get device serial number
 * \param[in]       serial: Pointer to output string array to save serial number info
 * \param[in]       len: Length of string array including `NULL` termination
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_device_get_serial_number(char* serial, size_t len, const lwgsm_api_cmd_evt_fn evt_fn,
                             void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_ASSERT("serial != NULL", serial != NULL);
    LWGSM_ASSERT("len > 0", len > 0);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_CGSN_GET;
    LWGSM_MSG_VAR_REF(msg).msg.device_info.str = serial;
    LWGSM_MSG_VAR_REF(msg).msg.device_info.len = len;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 10000);
}
