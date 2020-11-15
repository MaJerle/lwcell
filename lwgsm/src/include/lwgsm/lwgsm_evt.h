/**
 * \file            lwgsm_evt.h
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
#ifndef LWGSM_HDR_EVT_H
#define LWGSM_HDR_EVT_H

#include "lwgsm/lwgsm.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \ingroup         LWGSM
 * \defgroup        LWGSM_EVT Events management
 * \brief           Event helper functions
 * \{
 */

lwgsmr_t        lwgsm_evt_register(lwgsm_evt_fn fn);
lwgsmr_t        lwgsm_evt_unregister(lwgsm_evt_fn fn);
lwgsm_evt_type_t  lwgsm_evt_get_type(lwgsm_evt_t* cc);

/**
 * \anchor          LWGSM_EVT_RESET
 * \name            Reset event
 * \brief           Event helper functions for \ref LWGSM_EVT_RESET event
 */

lwgsmr_t        lwgsm_evt_reset_get_result(lwgsm_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWGSM_EVT_RESTORE
 * \name            Restore event
 * \brief           Event helper functions for \ref LWGSM_EVT_RESTORE event
 */

lwgsmr_t        lwgsm_evt_restore_get_result(lwgsm_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWGSM_EVT_NETWORK_OPERATOR_CURRENT
 * \name            Current network operator
 * \brief           Event helper functions for \ref LWGSM_EVT_NETWORK_OPERATOR_CURRENT event
 */

const lwgsm_operator_curr_t*  lwgsm_evt_network_operator_get_current(lwgsm_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWGSM_EVT_CONN_RECV
 * \name            Connection data received
 * \brief           Event helper functions for \ref LWGSM_EVT_CONN_RECV event
 */

lwgsm_pbuf_p    lwgsm_evt_conn_recv_get_buff(lwgsm_evt_t* cc);
lwgsm_conn_p    lwgsm_evt_conn_recv_get_conn(lwgsm_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWGSM_EVT_CONN_SEND
 * \name            Connection data send
 * \brief           Event helper functions for \ref LWGSM_EVT_CONN_SEND event
 */

lwgsm_conn_p    lwgsm_evt_conn_send_get_conn(lwgsm_evt_t* cc);
size_t          lwgsm_evt_conn_send_get_length(lwgsm_evt_t* cc);
lwgsmr_t        lwgsm_evt_conn_send_get_result(lwgsm_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWGSM_EVT_CONN_ACTIVE
 * \name            Connection active
 * \brief           Event helper functions for \ref LWGSM_EVT_CONN_ACTIVE event
 */

lwgsm_conn_p    lwgsm_evt_conn_active_get_conn(lwgsm_evt_t* cc);
uint8_t         lwgsm_evt_conn_active_is_client(lwgsm_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWGSM_EVT_CONN_CLOSE
 * \name            Connection close event
 * \brief           Event helper functions for \ref LWGSM_EVT_CONN_CLOSE event
 */

lwgsm_conn_p    lwgsm_evt_conn_close_get_conn(lwgsm_evt_t* cc);
uint8_t         lwgsm_evt_conn_close_is_client(lwgsm_evt_t* cc);
uint8_t         lwgsm_evt_conn_close_is_forced(lwgsm_evt_t* cc);
lwgsmr_t        lwgsm_evt_conn_close_get_result(lwgsm_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWGSM_EVT_CONN_POLL
 * \name            Connection poll
 * \brief           Event helper functions for \ref LWGSM_EVT_CONN_POLL event
 */

lwgsm_conn_p    lwgsm_evt_conn_poll_get_conn(lwgsm_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWGSM_EVT_CONN_ERROR
 * \name            Connection error
 * \brief           Event helper functions for \ref LWGSM_EVT_CONN_ERROR event
 */

lwgsmr_t            lwgsm_evt_conn_error_get_error(lwgsm_evt_t* cc);
lwgsm_conn_type_t   lwgsm_evt_conn_error_get_type(lwgsm_evt_t* cc);
const char*         lwgsm_evt_conn_error_get_host(lwgsm_evt_t* cc);
lwgsm_port_t        lwgsm_evt_conn_error_get_port(lwgsm_evt_t* cc);
void*               lwgsm_evt_conn_error_get_arg(lwgsm_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWGSM_EVT_SIGNAL_STRENGTH
 * \name            Signal strength
 * \brief           Event helper functions for \ref LWGSM_EVT_CONN_RECV event
 */

int16_t lwgsm_evt_signal_strength_get_rssi(lwgsm_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWGSM_EVT_SMS_RECV
 * \name            SMS received
 * \brief           Event helper functions for \ref LWGSM_EVT_SMS_RECV event
 */

size_t      lwgsm_evt_sms_recv_get_pos(lwgsm_evt_t* cc);
lwgsm_mem_t lwgsm_evt_sms_recv_get_mem(lwgsm_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWGSM_EVT_SMS_READ
 * \name            SMS content read
 * \brief           Event helper functions for \ref LWGSM_EVT_SMS_READ event
 */

lwgsm_sms_entry_t*  lwgsm_evt_sms_read_get_entry(lwgsm_evt_t* cc);
lwgsmr_t            lwgsm_evt_sms_read_get_result(lwgsm_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWGSM_EVT_SMS_SEND
 * \name            SMS send
 * \brief           Event helper functions for \ref LWGSM_EVT_SMS_SEND event
 */

lwgsmr_t    lwgsm_evt_sms_send_get_result(lwgsm_evt_t* cc);
size_t      lwgsm_evt_sms_send_get_pos(lwgsm_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWGSM_EVT_SMS_DELETE
 * \name            SMS delete
 * \brief           Event helper functions for \ref LWGSM_EVT_SMS_DELETE event
 */

lwgsmr_t    lwgsm_evt_sms_delete_get_result(lwgsm_evt_t* cc);
size_t      lwgsm_evt_sms_delete_get_pos(lwgsm_evt_t* cc);
lwgsm_mem_t lwgsm_evt_sms_delete_get_mem(lwgsm_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWGSM_EVT_CALL_CHANGED
 * \name            Call status changed
 * \brief           Event helper functions for \ref LWGSM_EVT_CALL_CHANGED event
 */

const lwgsm_call_t* lwgsm_evt_call_changed_get_call(lwgsm_evt_t* cc);

/**
 * \}
 */

/**
 * \anchor          LWGSM_EVT_OPERATOR_SCAN
 * \name            Operator scan
 * \brief           Event helper functions for \ref LWGSM_EVT_OPERATOR_SCAN event
 */

lwgsmr_t            lwgsm_evt_operator_scan_get_result(lwgsm_evt_t* cc);
lwgsm_operator_t*   lwgsm_evt_operator_scan_get_entries(lwgsm_evt_t* cc);
size_t              lwgsm_evt_operator_scan_get_length(lwgsm_evt_t* cc);

/**
 * \}
 */

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWGSM_HDR_EVT_H */
