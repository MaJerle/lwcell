/**
 * \file            lwgsm_netconn.h
 * \brief           API functions for sequential calls
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
#ifndef LWGSM_HDR_NETCONN_H
#define LWGSM_HDR_NETCONN_H

#include "lwgsm/lwgsm.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \ingroup         LWGSM_API
 * \defgroup        LWGSM_NETCONN Network connection
 * \brief           Network connection
 * \{
 */

struct lwgsm_netconn;

/**
 * \brief           Netconn object structure
 */
typedef struct lwgsm_netconn* lwgsm_netconn_p;

/**
 * \brief           Netconn connection type
 */
typedef enum {
    LWGSM_NETCONN_TYPE_TCP = LWGSM_CONN_TYPE_TCP,   /*!< TCP connection */
    LWGSM_NETCONN_TYPE_UDP = LWGSM_CONN_TYPE_UDP,   /*!< UDP connection */
    LWGSM_NETCONN_TYPE_SSL = LWGSM_CONN_TYPE_SSL,   /*!< TCP connection over SSL */
} lwgsm_netconn_type_t;

lwgsm_netconn_p   lwgsm_netconn_new(lwgsm_netconn_type_t type);
lwgsmr_t          lwgsm_netconn_delete(lwgsm_netconn_p nc);
lwgsmr_t          lwgsm_netconn_connect(lwgsm_netconn_p nc, const char* host, lwgsm_port_t port);
lwgsmr_t          lwgsm_netconn_receive(lwgsm_netconn_p nc, lwgsm_pbuf_p* pbuf);
lwgsmr_t          lwgsm_netconn_close(lwgsm_netconn_p nc);
int8_t          lwgsm_netconn_getconnnum(lwgsm_netconn_p nc);
void            lwgsm_netconn_set_receive_timeout(lwgsm_netconn_p nc, uint32_t timeout);
uint32_t        lwgsm_netconn_get_receive_timeout(lwgsm_netconn_p nc);

/* TCP only */
lwgsmr_t          lwgsm_netconn_write(lwgsm_netconn_p nc, const void* data, size_t btw);
lwgsmr_t          lwgsm_netconn_flush(lwgsm_netconn_p nc);

/* UDP only */
lwgsmr_t          lwgsm_netconn_send(lwgsm_netconn_p nc, const void* data, size_t btw);
lwgsmr_t          lwgsm_netconn_sendto(lwgsm_netconn_p nc, const lwgsm_ip_t* ip, lwgsm_port_t port, const void* data, size_t btw);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWGSM_HDR_NETCONN_H */
