/**	
 * \file            gsm_evt.c
 * \brief           Event helper functions
 */
 
/*
 * Copyright (c) 2018 Tilen Majerle
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
 */
#include "gsm/gsm_private.h"
#include "gsm/gsm_evt.h"
#include "gsm/gsm_mem.h"

/**
 * \brief           Get event type
 * \param[in]       cc: Event handle
 * \return          Event type. Member of \ref gsm_evt_type_t enumeration
 */
gsm_evt_type_t
gsm_evt_get_type(gsm_evt_t* cc) {
    return cc->type;
}

/**
 * \brief           Checks if reset was forced by user or not
 * \note            This function may only be used when event type is \ref GSM_EVT_RESET
 * \param[in]       cc: Event data
 * \return          `1` if forced by user, `0` otherwise
 */
uint8_t
gsm_evt_reset_is_forced(gsm_evt_t* cc) {
    return cc->evt.reset.forced;                /* Return forced reset status */
}

/**
 * \brief           Get current operator data from event
 * \note            This function may only be used when event type is \ref GSM_EVT_OPERATOR_CURRENT
 * \param[in]       cc: Event data
 * \return          Current operator handle
 */
const gsm_operator_curr_t *
gsm_evt_operator_current_get_operator(gsm_evt_t* cc) {
    return cc->evt.operator_current.operator_current;
}

#if GSM_CFG_CONN || __DOXYGEN__

/**
 * \brief           Get buffer from received data
 * \param[in]       cc: Event handle
 * \return          Buffer handle
 */
gsm_pbuf_p
gsm_evt_conn_data_recv_get_buff(gsm_evt_t* cc) {
    return cc->evt.conn_data_recv.buff;
}

/**
 * \brief           Get connection handle for receive
 * \param[in]       cc: Event handle
 * \return          Connection handle
 */
gsm_conn_p
gsm_evt_conn_data_recv_get_conn(gsm_evt_t* cc) {
    return cc->evt.conn_data_recv.conn;
}

/**
 * \brief           Get connection handle for data sent event
 * \param[in]       cc: Event handle
 * \return          Connection handle
 */
gsm_conn_p
gsm_evt_conn_data_send_get_conn(gsm_evt_t* cc) {
    return cc->evt.conn_data_send.conn;
}

/**
 * \brief           Get number of bytes sent on connection
 * \param[in]       cc: Event handle
 * \return          Number of bytes sent
 */
size_t
gsm_evt_conn_data_send_get_length(gsm_evt_t* cc) {
    return cc->evt.conn_data_send.sent;
}

/**
 * \brief           Check if connection send was successful
 * \param[in]       cc: Event handle
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
gsmr_t
gsm_evt_conn_data_send_get_result(gsm_evt_t* cc) {
    return cc->evt.conn_data_send.res;
}

/**
 * \brief           Get connection handle
 * \param[in]       cc: Event handle
 * \return          Connection handle
 */
gsm_conn_p
gsm_evt_conn_active_get_conn(gsm_evt_t* cc) {
    return cc->evt.conn_active_closed.conn;
}

/**
 * \brief           Check if new connection is client
 * \param[in]       cc: Event handle
 * \return          `1` if client, `0` otherwise
 */
uint8_t
gsm_evt_conn_active_is_client(gsm_evt_t* cc) {
    return GSM_U8(!!cc->evt.conn_active_closed.client);
}

/**
 * \brief           Get connection handle
 * \param[in]       cc: Event handle
 * \return          Connection handle
 */
gsm_conn_p
gsm_evt_conn_closed_get_conn(gsm_evt_t* cc) {
    return cc->evt.conn_active_closed.conn;
}

/**
 * \brief           Check if just closed connection was client
 * \param[in]       cc: Event handle
 * \return          `1` if client, `0` otherwise
 */
uint8_t
gsm_evt_conn_closed_is_client(gsm_evt_t* cc) {
    return cc->evt.conn_active_closed.client;
}

/**
 * \brief           Check if connection close even was forced by user
 * \param[in]       cc: Event handle
 * \return          `1` if forced, `0` otherwise
 */
uint8_t
gsm_evt_conn_closed_is_forced(gsm_evt_t* cc) {
    return cc->evt.conn_active_closed.forced;
}

/**
 * \brief           Get connection handle
 * \param[in]       cc: Event handle
 * \return          Connection handle
 */
gsm_conn_p
gsm_evt_conn_poll_get_conn(gsm_evt_t* cc) {
    return cc->evt.conn_poll.conn;
}

/**
 * \brief           Get connection error type
 * \param[in]       cc: Event handle
 * \return          Member of \ref gsmr_t enumeration
 */
gsmr_t
gsm_evt_conn_error_get_error(gsm_evt_t* cc) {
    return cc->evt.conn_error.err;
}

/**
 * \brief           Get connection type
 * \param[in]       cc: Event handle
 * \return          Member of \ref gsmr_t enumeration
 */
gsm_conn_type_t
gsm_evt_conn_error_get_type(gsm_evt_t* cc) {
    return cc->evt.conn_error.type;
}

/**
 * \brief           Get connection host
 * \param[in]       cc: Event handle
 * \return          Host name for connection
 */
const char *
gsm_evt_conn_error_get_host(gsm_evt_t* cc) {
    return cc->evt.conn_error.host;
}

/**
 * \brief           Get connection port
 * \param[in]       cc: Event handle
 * \return          Host port number
 */
gsm_port_t
gsm_evt_conn_error_get_port(gsm_evt_t* cc) {
    return cc->evt.conn_error.port;
}

/**
 * \brief           Get user argument
 * \param[in]       cc: Event handle
 * \return          User argument
 */
void *
gsm_evt_conn_error_get_arg(gsm_evt_t* cc) {
    return cc->evt.conn_error.arg;
}

#endif /* GSM_CFG_CONN || __DOXYGEN__ */
