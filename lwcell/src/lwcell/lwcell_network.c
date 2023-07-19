/**
 * \file            lwcell_network.c
 * \brief           Network API
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
 * This file is part of LwCELL - Lightweight cellular modem AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.1
 */
#include "lwcell/lwcell_network.h"
#include "lwcell/lwcell_private.h"

#if LWCELL_CFG_NETWORK || __DOXYGEN__

/**
 * \brief           Attach to network and active PDP context
 * \param[in]       apn: APN name
 * \param[in]       user: User name to attach. Set to `NULL` if not used
 * \param[in]       pass: User password to attach. Set to `NULL` if not used
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_network_attach(const char* apn, const char* user, const char* pass, const lwcell_api_cmd_evt_fn evt_fn,
                     void* const evt_arg, const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_NETWORK_ATTACH;
#if LWCELL_CFG_CONN
    LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CIPSTATUS;
#endif /* LWCELL_CFG_CONN */
    LWCELL_MSG_VAR_REF(msg).msg.network_attach.apn = apn;
    LWCELL_MSG_VAR_REF(msg).msg.network_attach.user = user;
    LWCELL_MSG_VAR_REF(msg).msg.network_attach.pass = pass;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 200000);
}

/**
 * \brief           Detach from network
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_network_detach(const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_NETWORK_DETACH;
#if LWCELL_CFG_CONN
    /* LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CIPSTATUS; */
#endif /* LWCELL_CFG_CONN */

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 60000);
}

/**
 * \brief           Check network PDP status
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_network_check_status(const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CIPSTATUS;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 60000);
}

/**
 * \brief           Copy IP address from internal value to user variable
 * \param[out]      ip: Pointer to output IP variable
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_network_copy_ip(lwcell_ip_t* ip) {
    if (lwcell_network_is_attached()) {
        lwcell_core_lock();
        LWCELL_MEMCPY(ip, &lwcell.m.network.ip_addr, sizeof(*ip));
        lwcell_core_unlock();
        return lwcellOK;
    }
    return lwcellERR;
}

/**
 * \brief           Check if device is attached to network and PDP context is active
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwcell_network_is_attached(void) {
    uint8_t res;
    lwcell_core_lock();
    res = LWCELL_U8(lwcell.m.network.is_attached);
    lwcell_core_unlock();
    return res;
}

#endif /* LWCELL_CFG_NETWORK || __DOXYGEN__ */

/**
 * \brief           Read RSSI signal from network operator
 * \param[out]      rssi: RSSI output variable. When set to `0`, RSSI is not valid
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_network_rssi(int16_t* rssi, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CSQ_GET;
    LWCELL_MSG_VAR_REF(msg).msg.csq.rssi = rssi;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 120000);
}

/**
 * \brief           Get network registration status
 * \return          Member of \ref lwcell_network_reg_status_t enumeration
 */
lwcell_network_reg_status_t
lwcell_network_get_reg_status(void) {
    lwcell_network_reg_status_t ret;
    lwcell_core_lock();
    ret = lwcell.m.network.status;
    lwcell_core_unlock();
    return ret;
}
