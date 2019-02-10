/**	
 * \file            gsm_evt.h
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
#ifndef GSM_HDR_EVT_H
#define GSM_HDR_EVT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gsm/gsm.h"

/**
 * \ingroup         GSM
 * \defgroup        GSM_EVT Event helpers
 * \brief           Event helper functions
 * \{
 */

gsm_evt_type_t  gsm_evt_get_type(gsm_evt_t* cc);

/**
 * \name            GSM_EVT_RESET
 * \anchor          GSM_EVT_RESET
 * \brief           Event helper functions for \ref GSM_EVT_RESET event
 */

gsmr_t      gsm_evt_reset_get_result(gsm_evt_t* cc);

/**
 * \}
 */

/**
 * \name            GSM_EVT_RESTORE
 * \anchor          GSM_EVT_RESTORE
 * \brief           Event helper functions for \ref GSM_EVT_RESTORE event
 */

gsmr_t      gsm_evt_restore_get_result(gsm_evt_t* cc);

/**
 * \}
 */

/**
 * \name            GSM_EVT_NETWORK_OPERATOR_CURRENT
 * \anchor          GSM_EVT_NETWORK_OPERATOR_CURRENT
 * \brief           Event helper functions for \ref GSM_EVT_NETWORK_OPERATOR_CURRENT event
 */

const gsm_operator_curr_t*  gsm_evt_network_operator_get_current(gsm_evt_t* cc);

/**
 * \}
 */

/**
 * \name            GSM_EVT_CONN_RECV
 * \anchor          GSM_EVT_CONN_RECV
 * \brief           Event helper functions for \ref GSM_EVT_CONN_RECV event
 */

gsm_pbuf_p  gsm_evt_conn_recv_get_buff(gsm_evt_t* cc);
gsm_conn_p  gsm_evt_conn_recv_get_conn(gsm_evt_t* cc);

/**
 * \}
 */

/**
 * \name            GSM_EVT_CONN_SEND
 * \anchor          GSM_EVT_CONN_SEND
 * \brief           Event helper functions for \ref GSM_EVT_CONN_SEND event
 */

gsm_conn_p  gsm_evt_conn_send_get_conn(gsm_evt_t* cc);
size_t      gsm_evt_conn_send_get_length(gsm_evt_t* cc);
gsmr_t      gsm_evt_conn_send_get_result(gsm_evt_t* cc);

/**
 * \}
 */

/**
 * \name            GSM_EVT_CONN_ACTIVE
 * \anchor          GSM_EVT_CONN_ACTIVE
 * \brief           Event helper functions for \ref GSM_EVT_CONN_ACTIVE event
 */

gsm_conn_p  gsm_evt_conn_active_get_conn(gsm_evt_t* cc);
uint8_t     gsm_evt_conn_active_is_client(gsm_evt_t* cc);

/**
 * \}
 */

/**
 * \name            GSM_EVT_CONN_CLOSED
 * \anchor          GSM_EVT_CONN_CLOSED
 * \brief           Event helper functions for \ref GSM_EVT_CONN_CLOSED event
 */

gsm_conn_p  gsm_evt_conn_closed_get_conn(gsm_evt_t* cc);
uint8_t     gsm_evt_conn_closed_is_client(gsm_evt_t* cc);
uint8_t     gsm_evt_conn_closed_is_forced(gsm_evt_t* cc);

/**
 * \}
 */

/**
 * \name            GSM_EVT_CONN_POLL
 * \anchor          GSM_EVT_CONN_POLL
 * \brief           Event helper functions for \ref GSM_EVT_CONN_POLL event
 */

gsm_conn_p  gsm_evt_conn_poll_get_conn(gsm_evt_t* cc);

/**
 * \}
 */

/**
 * \name            GSM_EVT_CONN_ERROR
 * \anchor          GSM_EVT_CONN_ERROR
 * \brief           Event helper functions for \ref GSM_EVT_CONN_ERROR event
 */

gsmr_t              gsm_evt_conn_error_get_error(gsm_evt_t* cc);
gsm_conn_type_t     gsm_evt_conn_error_get_type(gsm_evt_t* cc);
const char*         gsm_evt_conn_error_get_host(gsm_evt_t* cc);
gsm_port_t          gsm_evt_conn_error_get_port(gsm_evt_t* cc);
void*               gsm_evt_conn_error_get_arg(gsm_evt_t* cc);

/**
 * \}
 */

/**
 * \name            GSM_EVT_SIGNAL_STRENGTH
 * \anchor          GSM_EVT_SIGNAL_STRENGTH
 * \brief           Event helper functions for \ref GSM_EVT_CONN_RECV event
 */

int16_t gsm_evt_signal_strength_get_rssi(gsm_evt_t* cc);

/**
 * \}
 */

/**
 * \name            GSM_EVT_SMS_RECV
 * \anchor          GSM_EVT_SMS_RECV
 * \brief           Event helper functions for \ref GSM_EVT_SMS_RECV event
 */

size_t      gsm_evt_sms_recv_get_pos(gsm_evt_t* cc);
gsm_mem_t   gsm_evt_sms_recv_get_mem(gsm_evt_t* cc);

/**
 * \}
 */

/**
 * \name            GSM_EVT_SMS_READ
 * \anchor          GSM_EVT_SMS_READ
 * \brief           Event helper functions for \ref GSM_EVT_SMS_READ event
 */

gsm_sms_entry_t*    gsm_evt_sms_read_get_entry(gsm_evt_t* cc);
gsmr_t              gsm_evt_sms_read_get_result(gsm_evt_t* cc);

/**
 * \}
 */

/**
 * \name            GSM_EVT_SMS_SEND
 * \anchor          GSM_EVT_SMS_SEND
 * \brief           Event helper functions for \ref GSM_EVT_SMS_SEND event
 */

gsmr_t  gsm_evt_sms_send_get_result(gsm_evt_t* cc);
size_t  gsm_evt_sms_send_get_pos(gsm_evt_t* cc);

/**
 * \}
 */

/**
 * \name            GSM_EVT_SMS_DELETE
 * \anchor          GSM_EVT_SMS_DELETE
 * \brief           Event helper functions for \ref GSM_EVT_SMS_DELETE event
 */

gsmr_t      gsm_evt_sms_delete_get_result(gsm_evt_t* cc);
size_t      gsm_evt_sms_delete_get_pos(gsm_evt_t* cc);
gsm_mem_t   gsm_evt_sms_delete_get_mem(gsm_evt_t* cc);

/**
 * \}
 */

/**
 * \name            GSM_EVT_CALL_CHANGED
 * \anchor          GSM_EVT_CALL_CHANGED
 * \brief           Event helper functions for \ref GSM_EVT_CALL_CHANGED event
 */

const gsm_call_t*  gsm_evt_call_changed_get_call(gsm_evt_t* cc);

/**
 * \}
 */

/**
 * \}
 */

#ifdef __cplusplus
}
#endif

#endif /* GSM_HDR_EVT_H */
