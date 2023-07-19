/**
 * \file            lwcell_conn.h
 * \brief           Connection API
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
#ifndef LWCELL_CONN_HDR_H
#define LWCELL_CONN_HDR_H

#include "lwcell/lwcell_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \ingroup         LWCELL
 * \defgroup        LWCELL_CONN Connection API
 * \brief           Connection API functions
 * \{
 */

lwcellr_t lwcell_conn_start(lwcell_conn_p* conn, lwcell_conn_type_t type, const char* const host, lwcell_port_t port,
                          void* const arg, lwcell_evt_fn conn_evt_fn, const uint32_t blocking);
lwcellr_t lwcell_conn_close(lwcell_conn_p conn, const uint32_t blocking);
lwcellr_t lwcell_conn_send(lwcell_conn_p conn, const void* data, size_t btw, size_t* const bw, const uint32_t blocking);
lwcellr_t lwcell_conn_sendto(lwcell_conn_p conn, const lwcell_ip_t* const ip, lwcell_port_t port, const void* data,
                           size_t btw, size_t* bw, const uint32_t blocking);
lwcellr_t lwcell_conn_set_arg(lwcell_conn_p conn, void* const arg);
void* lwcell_conn_get_arg(lwcell_conn_p conn);
uint8_t lwcell_conn_is_client(lwcell_conn_p conn);
uint8_t lwcell_conn_is_active(lwcell_conn_p conn);
uint8_t lwcell_conn_is_closed(lwcell_conn_p conn);
int8_t lwcell_conn_getnum(lwcell_conn_p conn);
lwcellr_t lwcell_get_conns_status(const uint32_t blocking);
lwcell_conn_p lwcell_conn_get_from_evt(lwcell_evt_t* evt);
lwcellr_t lwcell_conn_write(lwcell_conn_p conn, const void* data, size_t btw, uint8_t flush, size_t* const mem_available);
lwcellr_t lwcell_conn_recved(lwcell_conn_p conn, lwcell_pbuf_p pbuf);
size_t lwcell_conn_get_total_recved_count(lwcell_conn_p conn);

uint8_t lwcell_conn_get_remote_ip(lwcell_conn_p conn, lwcell_ip_t* ip);
lwcell_port_t lwcell_conn_get_remote_port(lwcell_conn_p conn);
lwcell_port_t lwcell_conn_get_local_port(lwcell_conn_p conn);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWCELL_CONN_HDR_H */
