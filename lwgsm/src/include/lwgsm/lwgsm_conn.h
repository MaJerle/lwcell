/**
 * \file            lwgsm_conn.h
 * \brief           Connection API
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
#ifndef LWGSM_HDR_CONN_H
#define LWGSM_HDR_CONN_H

#include "lwgsm/lwgsm.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \ingroup         LWGSM
 * \defgroup        LWGSM_CONN Connection API
 * \brief           Connection API functions
 * \{
 */

lwgsmr_t      lwgsm_conn_start(lwgsm_conn_p* conn, lwgsm_conn_type_t type, const char* const host, lwgsm_port_t port, void* const arg, lwgsm_evt_fn conn_evt_fn, const uint32_t blocking);
lwgsmr_t      lwgsm_conn_close(lwgsm_conn_p conn, const uint32_t blocking);
lwgsmr_t      lwgsm_conn_send(lwgsm_conn_p conn, const void* data, size_t btw, size_t* const bw, const uint32_t blocking);
lwgsmr_t      lwgsm_conn_sendto(lwgsm_conn_p conn, const lwgsm_ip_t* const ip, lwgsm_port_t port, const void* data, size_t btw, size_t* bw, const uint32_t blocking);
lwgsmr_t      lwgsm_conn_set_arg(lwgsm_conn_p conn, void* const arg);
void*       lwgsm_conn_get_arg(lwgsm_conn_p conn);
uint8_t     lwgsm_conn_is_client(lwgsm_conn_p conn);
uint8_t     lwgsm_conn_is_active(lwgsm_conn_p conn);
uint8_t     lwgsm_conn_is_closed(lwgsm_conn_p conn);
int8_t      lwgsm_conn_getnum(lwgsm_conn_p conn);
lwgsmr_t      lwgsm_get_conns_status(const uint32_t blocking);
lwgsm_conn_p  lwgsm_conn_get_from_evt(lwgsm_evt_t* evt);
lwgsmr_t      lwgsm_conn_write(lwgsm_conn_p conn, const void* data, size_t btw, uint8_t flush, size_t* const mem_available);
lwgsmr_t      lwgsm_conn_recved(lwgsm_conn_p conn, lwgsm_pbuf_p pbuf);
size_t      lwgsm_conn_get_total_recved_count(lwgsm_conn_p conn);

uint8_t     lwgsm_conn_get_remote_ip(lwgsm_conn_p conn, lwgsm_ip_t* ip);
lwgsm_port_t  lwgsm_conn_get_remote_port(lwgsm_conn_p conn);
lwgsm_port_t  lwgsm_conn_get_local_port(lwgsm_conn_p conn);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWGSM_HDR_CONN_H */
