/**
 * \file            lwcell_evt.c
 * \brief           Event helper functions
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
#include "lwcell/lwcell_evt.h"
#include "lwcell/lwcell_private.h"

/**
 * \brief           Register callback function for global (non-connection based) events
 * \param[in]       fn: Callback function to call on specific event
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_evt_register(lwcell_evt_fn fn) {
    lwcellr_t res = lwcellOK;
    lwcell_evt_func_t *func, *new_func;

    LWCELL_ASSERT(fn != NULL);

    lwcell_core_lock();

    /* Check if function already exists on list */
    for (func = lwcell.evt_func; func != NULL; func = func->next) {
        if (func->fn == fn) {
            res = lwcellERR;
            break;
        }
    }

    if (res == lwcellOK) {
        new_func = lwcell_mem_malloc(sizeof(*new_func));
        if (new_func != NULL) {
            LWCELL_MEMSET(new_func, 0x00, sizeof(*new_func));
            new_func->fn = fn; /* Set function pointer */
            for (func = lwcell.evt_func; func != NULL && func->next != NULL; func = func->next) {}
            if (func != NULL) {
                func->next = new_func; /* Set new function as next */
                res = lwcellOK;
            } else {
                lwcell_mem_free_s((void**)&new_func);
                res = lwcellERRMEM;
            }
        } else {
            res = lwcellERRMEM;
        }
    }
    lwcell_core_unlock();
    return res;
}

/**
 * \brief           Unregister callback function for global (non-connection based) events
 * \note            Function must be first registered using \ref lwcell_evt_register
 * \param[in]       fn: Callback function to remove from event list
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_evt_unregister(lwcell_evt_fn fn) {
    lwcell_evt_func_t *func, *prev;
    LWCELL_ASSERT(fn != NULL);

    lwcell_core_lock();
    for (prev = lwcell.evt_func, func = lwcell.evt_func->next; func != NULL; prev = func, func = func->next) {
        if (func->fn == fn) {
            prev->next = func->next;
            lwcell_mem_free_s((void**)&func);
            break;
        }
    }
    lwcell_core_unlock();
    return lwcellOK;
}

/**
 * \brief           Get event type
 * \param[in]       cc: Event handle
 * \return          Event type. Member of \ref lwcell_evt_type_t enumeration
 */
lwcell_evt_type_t
lwcell_evt_get_type(lwcell_evt_t* cc) {
    return cc->type;
}

/**
 * \brief           Get reset sequence operation status
 * \param[in]       cc: Event data
 * \return          Member of \ref lwcellr_t enumeration
 */
lwcellr_t
lwcell_evt_reset_get_result(lwcell_evt_t* cc) {
    return cc->evt.reset.res;
}

/**
 * \brief           Get restore sequence operation status
 * \param[in]       cc: Event data
 * \return          Member of \ref lwcellr_t enumeration
 */
lwcellr_t
lwcell_evt_restore_get_result(lwcell_evt_t* cc) {
    return cc->evt.restore.res;
}

/**
 * \brief           Get current operator data from event
 * \param[in]       cc: Event data
 * \return          Current operator handle
 */
const lwcell_operator_curr_t*
lwcell_evt_network_operator_get_current(lwcell_evt_t* cc) {
    return cc->evt.operator_current.operator_current;
}

/**
 * \brief           Get operator scan operation status
 * \param[in]       cc: Event data
 * \return          Member of \ref lwcellr_t enumeration
 */
lwcellr_t
lwcell_evt_operator_scan_get_result(lwcell_evt_t* cc) {
    return cc->evt.operator_scan.res;
}

/**
 * \brief           Get operator entries from scan
 * \param[in]       cc: Event data
 * \return          Pointer to array of operator entries
 */
lwcell_operator_t*
lwcell_evt_operator_scan_get_entries(lwcell_evt_t* cc) {
    return cc->evt.operator_scan.ops;
}

/**
 * \brief           Get length of operators scanned
 * \param[in]       cc: Event data
 * \return          Number of operators scanned
 */
size_t
lwcell_evt_operator_scan_get_length(lwcell_evt_t* cc) {
    return cc->evt.operator_scan.opf;
}

/**
 * \brief           Get RSSi from CSQ command
 * \param[in]       cc: Event data
 * \return          RSSI value in units of dBm
 */
int16_t
lwcell_evt_signal_strength_get_rssi(lwcell_evt_t* cc) {
    return cc->evt.rssi.rssi;
}

#if LWCELL_CFG_CONN || __DOXYGEN__

/**
 * \brief           Get buffer from received data
 * \param[in]       cc: Event handle
 * \return          Buffer handle
 */
lwcell_pbuf_p
lwcell_evt_conn_recv_get_buff(lwcell_evt_t* cc) {
    return cc->evt.conn_data_recv.buff;
}

/**
 * \brief           Get connection handle for receive
 * \param[in]       cc: Event handle
 * \return          Connection handle
 */
lwcell_conn_p
lwcell_evt_conn_recv_get_conn(lwcell_evt_t* cc) {
    return cc->evt.conn_data_recv.conn;
}

/**
 * \brief           Get connection handle for data sent event
 * \param[in]       cc: Event handle
 * \return          Connection handle
 */
lwcell_conn_p
lwcell_evt_conn_send_get_conn(lwcell_evt_t* cc) {
    return cc->evt.conn_data_send.conn;
}

/**
 * \brief           Get number of bytes sent on connection
 * \param[in]       cc: Event handle
 * \return          Number of bytes sent
 */
size_t
lwcell_evt_conn_send_get_length(lwcell_evt_t* cc) {
    return cc->evt.conn_data_send.sent;
}

/**
 * \brief           Check if connection send was successful
 * \param[in]       cc: Event handle
 * \return          Member of \ref lwcellr_t enumeration
 */
lwcellr_t
lwcell_evt_conn_send_get_result(lwcell_evt_t* cc) {
    return cc->evt.conn_data_send.res;
}

/**
 * \brief           Get connection handle
 * \param[in]       cc: Event handle
 * \return          Connection handle
 */
lwcell_conn_p
lwcell_evt_conn_active_get_conn(lwcell_evt_t* cc) {
    return cc->evt.conn_active_close.conn;
}

/**
 * \brief           Check if new connection is client
 * \param[in]       cc: Event handle
 * \return          `1` if client, `0` otherwise
 */
uint8_t
lwcell_evt_conn_active_is_client(lwcell_evt_t* cc) {
    return LWCELL_U8(cc->evt.conn_active_close.client > 0);
}

/**
 * \brief           Get connection handle
 * \param[in]       cc: Event handle
 * \return          Connection handle
 */
lwcell_conn_p
lwcell_evt_conn_close_get_conn(lwcell_evt_t* cc) {
    return cc->evt.conn_active_close.conn;
}

/**
 * \brief           Check if close connection was client
 * \param[in]       cc: Event handle
 * \return          `1` if client, `0` otherwise
 */
uint8_t
lwcell_evt_conn_close_is_client(lwcell_evt_t* cc) {
    return cc->evt.conn_active_close.client;
}

/**
 * \brief           Check if connection close even was forced by user
 * \param[in]       cc: Event handle
 * \return          `1` if forced, `0` otherwise
 */
uint8_t
lwcell_evt_conn_close_is_forced(lwcell_evt_t* cc) {
    return cc->evt.conn_active_close.forced;
}

/**
 * \brief           Get connection close event result
 * \param[in]       cc: Event handle
 * \return          Member of \ref lwcellr_t enumeration
 */
lwcellr_t
lwcell_evt_conn_close_get_result(lwcell_evt_t* cc) {
    return cc->evt.conn_active_close.res;
}

/**
 * \brief           Get connection handle
 * \param[in]       cc: Event handle
 * \return          Connection handle
 */
lwcell_conn_p
lwcell_evt_conn_poll_get_conn(lwcell_evt_t* cc) {
    return cc->evt.conn_poll.conn;
}

/**
 * \brief           Get connection error type
 * \param[in]       cc: Event handle
 * \return          Member of \ref lwcellr_t enumeration
 */
lwcellr_t
lwcell_evt_conn_error_get_error(lwcell_evt_t* cc) {
    return cc->evt.conn_error.err;
}

/**
 * \brief           Get connection type
 * \param[in]       cc: Event handle
 * \return          Member of \ref lwcellr_t enumeration
 */
lwcell_conn_type_t
lwcell_evt_conn_error_get_type(lwcell_evt_t* cc) {
    return cc->evt.conn_error.type;
}

/**
 * \brief           Get connection host
 * \param[in]       cc: Event handle
 * \return          Host name for connection
 */
const char*
lwcell_evt_conn_error_get_host(lwcell_evt_t* cc) {
    return cc->evt.conn_error.host;
}

/**
 * \brief           Get connection port
 * \param[in]       cc: Event handle
 * \return          Host port number
 */
lwcell_port_t
lwcell_evt_conn_error_get_port(lwcell_evt_t* cc) {
    return cc->evt.conn_error.port;
}

/**
 * \brief           Get user argument
 * \param[in]       cc: Event handle
 * \return          User argument
 */
void*
lwcell_evt_conn_error_get_arg(lwcell_evt_t* cc) {
    return cc->evt.conn_error.arg;
}

#endif /* LWCELL_CFG_CONN || __DOXYGEN__ */

#if LWCELL_CFG_SMS || __DOXYGEN__

/**
 * \brief           Get SMS position in memory which has been saved on receive
 * \param[in]       cc: Event handle
 * \return          SMS position in memory
 */
size_t
lwcell_evt_sms_recv_get_pos(lwcell_evt_t* cc) {
    return cc->evt.sms_recv.pos;
}

/**
 * \brief           Get SMS memory used to save SMS on receive
 * \param[in]       cc: Event handle
 * \return          SMS memory location
 */
lwcell_mem_t
lwcell_evt_sms_recv_get_mem(lwcell_evt_t* cc) {
    return cc->evt.sms_recv.mem;
}

/**
 * \brief           Get SMS entry after successful read
 * \param[in]       cc: Event handle
 * \return          SMS entry
 */
lwcell_sms_entry_t*
lwcell_evt_sms_read_get_entry(lwcell_evt_t* cc) {
    return cc->evt.sms_read.entry;
}

/**
 * \brief           Get SMS read operation result
 * \param[in]       cc: Event handle
 * \return          SMS entry
 */
lwcellr_t
lwcell_evt_sms_read_get_result(lwcell_evt_t* cc) {
    return cc->evt.sms_read.res;
}

/**
 * \brief           Get SMS send result status
 * \param[in]       cc: Event handle
 * \return          Member of \ref lwcellr_t enumeration
 */
lwcellr_t
lwcell_evt_sms_send_get_result(lwcell_evt_t* cc) {
    return cc->evt.sms_send.res;
}

/**
 * \brief           Get SMS send position in memory
 * \note            Use only if SMS sent successfully
 * \param[in]       cc: Event handle
 * \return          Position in memory
 */
size_t
lwcell_evt_sms_send_get_pos(lwcell_evt_t* cc) {
    return cc->evt.sms_send.pos;
}

/**
 * \brief           Get SMS delete result status
 * \param[in]       cc: Event handle
 * \return          Member of \ref lwcellr_t enumeration
 */
lwcellr_t
lwcell_evt_sms_delete_get_result(lwcell_evt_t* cc) {
    return cc->evt.sms_delete.res;
}

/**
 * \brief           Get SMS delete memory position
 * \param[in]       cc: Event handle
 * \return          Deleted position in memory
 */
size_t
lwcell_evt_sms_delete_get_pos(lwcell_evt_t* cc) {
    return cc->evt.sms_delete.pos;
}

/**
 * \brief           Get SMS delete memory
 * \param[in]       cc: Event handle
 * \return          SMS memory for delete operation
 */
lwcell_mem_t
lwcell_evt_sms_delete_get_mem(lwcell_evt_t* cc) {
    return cc->evt.sms_delete.mem;
}

#endif /* LWCELL_CFG_SMS || __DOXYGEN__ */

#if LWCELL_CFG_CALL || __DOXYGEN__

/**
 * \brief           Get call information from changed event
 * \param[in]       cc: Event handle
 * \return          Position in memory
 */
const lwcell_call_t*
lwcell_evt_call_changed_get_call(lwcell_evt_t* cc) {
    LWCELL_UNUSED(cc);
    return lwcell.evt.evt.call_changed.call;
}

#endif /* LWCELL_CFG_CALL || __DOXYGEN__ */
