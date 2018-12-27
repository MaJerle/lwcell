/**
 * \file            gsm_conn.h
 * \brief           Connection API
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
#ifndef __GSM_CONN_H
#define __GSM_CONN_H

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif

#include "gsm/gsm.h"

/**
 * \ingroup         GSM
 * \defgroup        GSM_CONN Connection API
 * \brief           Connection API functions
 * \{
 */

gsmr_t      gsm_conn_start(gsm_conn_p* conn, gsm_conn_type_t type, const char* const host, gsm_port_t port, void* const arg, gsm_evt_fn cb_func, const uint32_t blocking);
gsmr_t      gsm_conn_close(gsm_conn_p conn, const uint32_t blocking);
gsmr_t      gsm_conn_send(gsm_conn_p conn, const void* data, size_t btw, size_t* const bw, const uint32_t blocking);
gsmr_t      gsm_conn_sendto(gsm_conn_p conn, const gsm_ip_t* const ip, gsm_port_t port, const void* data, size_t btw, size_t* bw, const uint32_t blocking);
gsmr_t      gsm_conn_set_arg(gsm_conn_p conn, void* const arg);
void *      gsm_conn_get_arg(gsm_conn_p conn);
uint8_t     gsm_conn_is_client(gsm_conn_p conn);
uint8_t     gsm_conn_is_active(gsm_conn_p conn);
uint8_t     gsm_conn_is_closed(gsm_conn_p conn);
int8_t      gsm_conn_getnum(gsm_conn_p conn);
gsmr_t      gsm_get_conns_status(const uint32_t blocking);
gsm_conn_p  gsm_conn_get_from_evt(gsm_evt_t* evt);
gsmr_t      gsm_conn_write(gsm_conn_p conn, const void* data, size_t btw, uint8_t flush, size_t* const mem_available);
gsmr_t      gsm_conn_recved(gsm_conn_p conn, gsm_pbuf_p pbuf);
size_t      gsm_conn_get_total_recved_count(gsm_conn_p conn);

uint8_t     gsm_conn_get_remote_ip(gsm_conn_p conn, gsm_ip_t* ip);
gsm_port_t  gsm_conn_get_remote_port(gsm_conn_p conn);
gsm_port_t  gsm_conn_get_local_port(gsm_conn_p conn);

/**
 * \}
 */

/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif /* __GSM_CONN_H */
