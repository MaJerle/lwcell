/**
 * \file            lwcell_netconn.h
 * \brief           API functions for sequential calls
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
#ifndef LWCELL_NETCONN_HDR_H
#define LWCELL_NETCONN_HDR_H

#include "lwcell/lwcell_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \ingroup         LWCELL_API
 * \defgroup        LWCELL_NETCONN Network connection
 * \brief           Network connection
 * \{
 */

struct lwcell_netconn;

/**
 * \brief           Netconn object structure
 */
typedef struct lwcell_netconn* lwcell_netconn_p;

/**
 * \brief           Receive data with no timeout
 * \note            Used with \ref lwcell_netconn_set_receive_timeout function
 */
#define LWCELL_NETCONN_RECEIVE_NO_WAIT 0xFFFFFFFF

/* Immediate flush for TCP write. Used with \ref lwcell_netconn_write_ex*/
#define LWCELL_NETCONN_FLAG_FLUSH      ((uint16_t)0x0001) /*!< Immediate flush after netconn write */

/**
 * \brief           Netconn connection type
 */
typedef enum {
    LWCELL_NETCONN_TYPE_TCP = LWCELL_CONN_TYPE_TCP, /*!< TCP connection */
    LWCELL_NETCONN_TYPE_UDP = LWCELL_CONN_TYPE_UDP, /*!< UDP connection */
    LWCELL_NETCONN_TYPE_SSL = LWCELL_CONN_TYPE_SSL, /*!< TCP connection over SSL 
                                                         Note: This option is unstable on SIM868 devices due to firmware issues */
} lwcell_netconn_type_t;

lwcell_netconn_p lwcell_netconn_new(lwcell_netconn_type_t type);
lwcellr_t lwcell_netconn_delete(lwcell_netconn_p nc);
lwcellr_t lwcell_netconn_connect(lwcell_netconn_p nc, const char* host, lwcell_port_t port);
lwcellr_t lwcell_netconn_receive(lwcell_netconn_p nc, lwcell_pbuf_p* pbuf);
lwcellr_t lwcell_netconn_close(lwcell_netconn_p nc);
int8_t lwcell_netconn_getconnnum(lwcell_netconn_p nc);
void lwcell_netconn_set_receive_timeout(lwcell_netconn_p nc, uint32_t timeout);
uint32_t lwcell_netconn_get_receive_timeout(lwcell_netconn_p nc);

/* TCP only */
lwcellr_t lwcell_netconn_write(lwcell_netconn_p nc, const void* data, size_t btw);
lwcellr_t lwcell_netconn_write_ex(lwcell_netconn_p nc, const void* data, size_t btw, uint16_t flags);
lwcellr_t lwcell_netconn_flush(lwcell_netconn_p nc);

/* UDP only */
lwcellr_t lwcell_netconn_send(lwcell_netconn_p nc, const void* data, size_t btw);
lwcellr_t lwcell_netconn_sendto(lwcell_netconn_p nc, const lwcell_ip_t* ip, lwcell_port_t port, const void* data,
                              size_t btw);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWCELL_NETCONN_HDR_H */
