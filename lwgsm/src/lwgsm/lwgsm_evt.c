/**
 * \file            lwgsm_evt.c
 * \brief           Event helper functions
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
#include "lwgsm/lwgsm_evt.h"
#include "lwgsm/lwgsm_mem.h"

/**
 * \brief           Register callback function for global (non-connection based) events
 * \param[in]       fn: Callback function to call on specific event
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_evt_register(lwgsm_evt_fn fn) {
    lwgsmr_t res = lwgsmOK;
    lwgsm_evt_func_t* func, *new_func;

    LWGSM_ASSERT("fn != NULL", fn != NULL);

    lwgsm_core_lock();

    /* Check if function already exists on list */
    for (func = lwgsm.evt_func; func != NULL; func = func->next) {
        if (func->fn == fn) {
            res = lwgsmERR;
            break;
        }
    }

    if (res == lwgsmOK) {
        new_func = lwgsm_mem_malloc(sizeof(*new_func));
        if (new_func != NULL) {
            LWGSM_MEMSET(new_func, 0x00, sizeof(*new_func));
            new_func->fn = fn;                  /* Set function pointer */
            for (func = lwgsm.evt_func; func != NULL && func->next != NULL; func = func->next) {}
            if (func != NULL) {
                func->next = new_func;          /* Set new function as next */
                res = lwgsmOK;
            } else {
                lwgsm_mem_free_s((void**)&new_func);
                res = lwgsmERRMEM;
            }
        } else {
            res = lwgsmERRMEM;
        }
    }
    lwgsm_core_unlock();
    return res;
}

/**
 * \brief           Unregister callback function for global (non-connection based) events
 * \note            Function must be first registered using \ref lwgsm_evt_register
 * \param[in]       fn: Callback function to remove from event list
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_evt_unregister(lwgsm_evt_fn fn) {
    lwgsm_evt_func_t* func, *prev;
    LWGSM_ASSERT("fn != NULL", fn != NULL);

    lwgsm_core_lock();
    for (prev = lwgsm.evt_func, func = lwgsm.evt_func->next; func != NULL; prev = func, func = func->next) {
        if (func->fn == fn) {
            prev->next = func->next;
            lwgsm_mem_free_s((void**)&func);
            break;
        }
    }
    lwgsm_core_unlock();
    return lwgsmOK;
}

/**
 * \brief           Get event type
 * \param[in]       cc: Event handle
 * \return          Event type. Member of \ref lwgsm_evt_type_t enumeration
 */
lwgsm_evt_type_t
lwgsm_evt_get_type(lwgsm_evt_t* cc) {
    return cc->type;
}

/**
 * \brief           Get reset sequence operation status
 * \param[in]       cc: Event data
 * \return          Member of \ref lwgsmr_t enumeration
 */
lwgsmr_t
lwgsm_evt_reset_get_result(lwgsm_evt_t* cc) {
    return cc->evt.reset.res;
}

/**
 * \brief           Get restore sequence operation status
 * \param[in]       cc: Event data
 * \return          Member of \ref lwgsmr_t enumeration
 */
lwgsmr_t
lwgsm_evt_restore_get_result(lwgsm_evt_t* cc) {
    return cc->evt.restore.res;
}

/**
 * \brief           Get current operator data from event
 * \param[in]       cc: Event data
 * \return          Current operator handle
 */
const lwgsm_operator_curr_t*
lwgsm_evt_network_operator_get_current(lwgsm_evt_t* cc) {
    return cc->evt.operator_current.operator_current;
}

/**
 * \brief           Get operator scan operation status
 * \param[in]       cc: Event data
 * \return          Member of \ref lwgsmr_t enumeration
 */
lwgsmr_t
lwgsm_evt_operator_scan_get_result(lwgsm_evt_t* cc) {
    return cc->evt.operator_scan.res;
}

/**
 * \brief           Get operator entries from scan
 * \param[in]       cc: Event data
 * \return          Pointer to array of operator entries
 */
lwgsm_operator_t*
lwgsm_evt_operator_scan_get_entries(lwgsm_evt_t* cc) {
    return cc->evt.operator_scan.ops;
}

/**
 * \brief           Get length of operators scanned
 * \param[in]       cc: Event data
 * \return          Number of operators scanned
 */
size_t
lwgsm_evt_operator_scan_get_length(lwgsm_evt_t* cc) {
    return cc->evt.operator_scan.opf;
}

/**
 * \brief           Get RSSi from CSQ command
 * \param[in]       cc: Event data
 * \return          RSSI value in units of dBm
 */
int16_t
lwgsm_evt_signal_strength_get_rssi(lwgsm_evt_t* cc) {
    return cc->evt.rssi.rssi;
}

#if LWGSM_CFG_CONN || __DOXYGEN__

/**
 * \brief           Get buffer from received data
 * \param[in]       cc: Event handle
 * \return          Buffer handle
 */
lwgsm_pbuf_p
lwgsm_evt_conn_recv_get_buff(lwgsm_evt_t* cc) {
    return cc->evt.conn_data_recv.buff;
}

/**
 * \brief           Get connection handle for receive
 * \param[in]       cc: Event handle
 * \return          Connection handle
 */
lwgsm_conn_p
lwgsm_evt_conn_recv_get_conn(lwgsm_evt_t* cc) {
    return cc->evt.conn_data_recv.conn;
}

/**
 * \brief           Get connection handle for data sent event
 * \param[in]       cc: Event handle
 * \return          Connection handle
 */
lwgsm_conn_p
lwgsm_evt_conn_send_get_conn(lwgsm_evt_t* cc) {
    return cc->evt.conn_data_send.conn;
}

/**
 * \brief           Get number of bytes sent on connection
 * \param[in]       cc: Event handle
 * \return          Number of bytes sent
 */
size_t
lwgsm_evt_conn_send_get_length(lwgsm_evt_t* cc) {
    return cc->evt.conn_data_send.sent;
}

/**
 * \brief           Check if connection send was successful
 * \param[in]       cc: Event handle
 * \return          Member of \ref lwgsmr_t enumeration
 */
lwgsmr_t
lwgsm_evt_conn_send_get_result(lwgsm_evt_t* cc) {
    return cc->evt.conn_data_send.res;
}

/**
 * \brief           Get connection handle
 * \param[in]       cc: Event handle
 * \return          Connection handle
 */
lwgsm_conn_p
lwgsm_evt_conn_active_get_conn(lwgsm_evt_t* cc) {
    return cc->evt.conn_active_close.conn;
}

/**
 * \brief           Check if new connection is client
 * \param[in]       cc: Event handle
 * \return          `1` if client, `0` otherwise
 */
uint8_t
lwgsm_evt_conn_active_is_client(lwgsm_evt_t* cc) {
    return LWGSM_U8(cc->evt.conn_active_close.client > 0);
}

/**
 * \brief           Get connection handle
 * \param[in]       cc: Event handle
 * \return          Connection handle
 */
lwgsm_conn_p
lwgsm_evt_conn_close_get_conn(lwgsm_evt_t* cc) {
    return cc->evt.conn_active_close.conn;
}

/**
 * \brief           Check if close connection was client
 * \param[in]       cc: Event handle
 * \return          `1` if client, `0` otherwise
 */
uint8_t
lwgsm_evt_conn_close_is_client(lwgsm_evt_t* cc) {
    return cc->evt.conn_active_close.client;
}

/**
 * \brief           Check if connection close even was forced by user
 * \param[in]       cc: Event handle
 * \return          `1` if forced, `0` otherwise
 */
uint8_t
lwgsm_evt_conn_close_is_forced(lwgsm_evt_t* cc) {
    return cc->evt.conn_active_close.forced;
}

/**
 * \brief           Get connection close event result
 * \param[in]       cc: Event handle
 * \return          Member of \ref lwgsmr_t enumeration
 */
lwgsmr_t
lwgsm_evt_conn_close_get_result(lwgsm_evt_t* cc) {
    return cc->evt.conn_active_close.res;
}

/**
 * \brief           Get connection handle
 * \param[in]       cc: Event handle
 * \return          Connection handle
 */
lwgsm_conn_p
lwgsm_evt_conn_poll_get_conn(lwgsm_evt_t* cc) {
    return cc->evt.conn_poll.conn;
}

/**
 * \brief           Get connection error type
 * \param[in]       cc: Event handle
 * \return          Member of \ref lwgsmr_t enumeration
 */
lwgsmr_t
lwgsm_evt_conn_error_get_error(lwgsm_evt_t* cc) {
    return cc->evt.conn_error.err;
}

/**
 * \brief           Get connection type
 * \param[in]       cc: Event handle
 * \return          Member of \ref lwgsmr_t enumeration
 */
lwgsm_conn_type_t
lwgsm_evt_conn_error_get_type(lwgsm_evt_t* cc) {
    return cc->evt.conn_error.type;
}

/**
 * \brief           Get connection host
 * \param[in]       cc: Event handle
 * \return          Host name for connection
 */
const char*
lwgsm_evt_conn_error_get_host(lwgsm_evt_t* cc) {
    return cc->evt.conn_error.host;
}

/**
 * \brief           Get connection port
 * \param[in]       cc: Event handle
 * \return          Host port number
 */
lwgsm_port_t
lwgsm_evt_conn_error_get_port(lwgsm_evt_t* cc) {
    return cc->evt.conn_error.port;
}

/**
 * \brief           Get user argument
 * \param[in]       cc: Event handle
 * \return          User argument
 */
void*
lwgsm_evt_conn_error_get_arg(lwgsm_evt_t* cc) {
    return cc->evt.conn_error.arg;
}

#endif /* LWGSM_CFG_CONN || __DOXYGEN__ */

#if LWGSM_CFG_SMS || __DOXYGEN__

/**
 * \brief           Get SMS position in memory which has been saved on receive
 * \param[in]       cc: Event handle
 * \return          SMS position in memory
 */
size_t
lwgsm_evt_sms_recv_get_pos(lwgsm_evt_t* cc) {
    return cc->evt.sms_recv.pos;
}

/**
 * \brief           Get SMS memory used to save SMS on receive
 * \param[in]       cc: Event handle
 * \return          SMS memory location
 */
lwgsm_mem_t
lwgsm_evt_sms_recv_get_mem(lwgsm_evt_t* cc) {
    return cc->evt.sms_recv.mem;
}

/**
 * \brief           Get SMS entry after successful read
 * \param[in]       cc: Event handle
 * \return          SMS entry
 */
lwgsm_sms_entry_t*
lwgsm_evt_sms_read_get_entry(lwgsm_evt_t* cc) {
    return cc->evt.sms_read.entry;
}

/**
 * \brief           Get SMS read operation result
 * \param[in]       cc: Event handle
 * \return          SMS entry
 */
lwgsmr_t
lwgsm_evt_sms_read_get_result(lwgsm_evt_t* cc) {
    return cc->evt.sms_read.res;
}

/**
 * \brief           Get SMS send result status
 * \param[in]       cc: Event handle
 * \return          Member of \ref lwgsmr_t enumeration
 */
lwgsmr_t
lwgsm_evt_sms_send_get_result(lwgsm_evt_t* cc) {
    return cc->evt.sms_send.res;
}

/**
 * \brief           Get SMS send position in memory
 * \note            Use only if SMS sent successfully
 * \param[in]       cc: Event handle
 * \return          Position in memory
 */
size_t
lwgsm_evt_sms_send_get_pos(lwgsm_evt_t* cc) {
    return cc->evt.sms_send.pos;
}

/**
 * \brief           Get SMS delete result status
 * \param[in]       cc: Event handle
 * \return          Member of \ref lwgsmr_t enumeration
 */
lwgsmr_t
lwgsm_evt_sms_delete_get_result(lwgsm_evt_t* cc) {
    return cc->evt.sms_delete.res;
}

/**
 * \brief           Get SMS delete memory position
 * \param[in]       cc: Event handle
 * \return          Deleted position in memory
 */
size_t
lwgsm_evt_sms_delete_get_pos(lwgsm_evt_t* cc) {
    return cc->evt.sms_delete.pos;
}

/**
 * \brief           Get SMS delete memory
 * \param[in]       cc: Event handle
 * \return          SMS memory for delete operation
 */
lwgsm_mem_t
lwgsm_evt_sms_delete_get_mem(lwgsm_evt_t* cc) {
    return cc->evt.sms_delete.mem;
}

#endif /* LWGSM_CFG_SMS || __DOXYGEN__ */

#if LWGSM_CFG_CALL || __DOXYGEN__

/**
 * \brief           Get call information from changed event
 * \param[in]       cc: Event handle
 * \return          Position in memory
 */
const lwgsm_call_t*
lwgsm_evt_call_changed_get_call(lwgsm_evt_t* cc) {
    return lwgsm.evt.evt.call_changed.call;
}

#endif /* LWGSM_CFG_CALL || __DOXYGEN__ */
