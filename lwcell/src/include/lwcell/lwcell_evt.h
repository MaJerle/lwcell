/**
 * \file            lwcell_evt.h
 * \brief           Event helper functions
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
 * This file is part of LwCELL - Lightweight GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.1
 */
#ifndef LWCELL_EVT_HDR_H
#define LWCELL_EVT_HDR_H

#include "lwcell/lwcell_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \ingroup         LWCELL
 * \defgroup        LWCELL_EVT Events management
 * \brief           Event helper functions
 * \{
 */

lwcellr_t lwcell_evt_register(lwcell_evt_fn fn);
lwcellr_t lwcell_evt_unregister(lwcell_evt_fn fn);
lwcell_evt_type_t lwcell_evt_get_type(lwcell_evt_t* cc);

/**
 * \anchor          LWCELL_EVT_RESET
 * \name            Reset event
 * \brief           Event helper functions for \ref LWCELL_EVT_RESET event
 */

lwcellr_t lwcell_evt_reset_get_result(lwcell_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWCELL_EVT_RESTORE
 * \name            Restore event
 * \brief           Event helper functions for \ref LWCELL_EVT_RESTORE event
 */

lwcellr_t lwcell_evt_restore_get_result(lwcell_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWCELL_EVT_NETWORK_OPERATOR_CURRENT
 * \name            Current network operator
 * \brief           Event helper functions for \ref LWCELL_EVT_NETWORK_OPERATOR_CURRENT event
 */

const lwcell_operator_curr_t* lwcell_evt_network_operator_get_current(lwcell_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWCELL_EVT_CONN_RECV
 * \name            Connection data received
 * \brief           Event helper functions for \ref LWCELL_EVT_CONN_RECV event
 */

lwcell_pbuf_p lwcell_evt_conn_recv_get_buff(lwcell_evt_t* cc);
lwcell_conn_p lwcell_evt_conn_recv_get_conn(lwcell_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWCELL_EVT_CONN_SEND
 * \name            Connection data send
 * \brief           Event helper functions for \ref LWCELL_EVT_CONN_SEND event
 */

lwcell_conn_p lwcell_evt_conn_send_get_conn(lwcell_evt_t* cc);
size_t lwcell_evt_conn_send_get_length(lwcell_evt_t* cc);
lwcellr_t lwcell_evt_conn_send_get_result(lwcell_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWCELL_EVT_CONN_ACTIVE
 * \name            Connection active
 * \brief           Event helper functions for \ref LWCELL_EVT_CONN_ACTIVE event
 */

lwcell_conn_p lwcell_evt_conn_active_get_conn(lwcell_evt_t* cc);
uint8_t lwcell_evt_conn_active_is_client(lwcell_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWCELL_EVT_CONN_CLOSE
 * \name            Connection close event
 * \brief           Event helper functions for \ref LWCELL_EVT_CONN_CLOSE event
 */

lwcell_conn_p lwcell_evt_conn_close_get_conn(lwcell_evt_t* cc);
uint8_t lwcell_evt_conn_close_is_client(lwcell_evt_t* cc);
uint8_t lwcell_evt_conn_close_is_forced(lwcell_evt_t* cc);
lwcellr_t lwcell_evt_conn_close_get_result(lwcell_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWCELL_EVT_CONN_POLL
 * \name            Connection poll
 * \brief           Event helper functions for \ref LWCELL_EVT_CONN_POLL event
 */

lwcell_conn_p lwcell_evt_conn_poll_get_conn(lwcell_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWCELL_EVT_CONN_ERROR
 * \name            Connection error
 * \brief           Event helper functions for \ref LWCELL_EVT_CONN_ERROR event
 */

lwcellr_t lwcell_evt_conn_error_get_error(lwcell_evt_t* cc);
lwcell_conn_type_t lwcell_evt_conn_error_get_type(lwcell_evt_t* cc);
const char* lwcell_evt_conn_error_get_host(lwcell_evt_t* cc);
lwcell_port_t lwcell_evt_conn_error_get_port(lwcell_evt_t* cc);
void* lwcell_evt_conn_error_get_arg(lwcell_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWCELL_EVT_SIGNAL_STRENGTH
 * \name            Signal strength
 * \brief           Event helper functions for \ref LWCELL_EVT_CONN_RECV event
 */

int16_t lwcell_evt_signal_strength_get_rssi(lwcell_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWCELL_EVT_SMS_RECV
 * \name            SMS received
 * \brief           Event helper functions for \ref LWCELL_EVT_SMS_RECV event
 */

size_t lwcell_evt_sms_recv_get_pos(lwcell_evt_t* cc);
lwcell_mem_t lwcell_evt_sms_recv_get_mem(lwcell_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWCELL_EVT_SMS_READ
 * \name            SMS content read
 * \brief           Event helper functions for \ref LWCELL_EVT_SMS_READ event
 */

lwcell_sms_entry_t* lwcell_evt_sms_read_get_entry(lwcell_evt_t* cc);
lwcellr_t lwcell_evt_sms_read_get_result(lwcell_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWCELL_EVT_SMS_SEND
 * \name            SMS send
 * \brief           Event helper functions for \ref LWCELL_EVT_SMS_SEND event
 */

lwcellr_t lwcell_evt_sms_send_get_result(lwcell_evt_t* cc);
size_t lwcell_evt_sms_send_get_pos(lwcell_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWCELL_EVT_SMS_DELETE
 * \name            SMS delete
 * \brief           Event helper functions for \ref LWCELL_EVT_SMS_DELETE event
 */

lwcellr_t lwcell_evt_sms_delete_get_result(lwcell_evt_t* cc);
size_t lwcell_evt_sms_delete_get_pos(lwcell_evt_t* cc);
lwcell_mem_t lwcell_evt_sms_delete_get_mem(lwcell_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWCELL_EVT_CALL_CHANGED
 * \name            Call status changed
 * \brief           Event helper functions for \ref LWCELL_EVT_CALL_CHANGED event
 */

const lwcell_call_t* lwcell_evt_call_changed_get_call(lwcell_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWCELL_EVT_OPERATOR_SCAN
 * \name            Operator scan
 * \brief           Event helper functions for \ref LWCELL_EVT_OPERATOR_SCAN event
 */

lwcellr_t lwcell_evt_operator_scan_get_result(lwcell_evt_t* cc);
lwcell_operator_t* lwcell_evt_operator_scan_get_entries(lwcell_evt_t* cc);
size_t lwcell_evt_operator_scan_get_length(lwcell_evt_t* cc);

/**
 * \}
 */

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWCELL_EVT_HDR_H */
