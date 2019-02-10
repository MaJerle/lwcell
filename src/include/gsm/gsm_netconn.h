/**
 * \file            gsm_netconn.h
 * \brief           API functions for sequential calls
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
#ifndef GSM_HDR_NETCONN_H
#define GSM_HDR_NETCONN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "gsm/gsm.h"

/**
 * \ingroup         GSM_API
 * \defgroup        GSM_NETCONN Network connection
 * \brief           Network connection
 * \{
 */

struct gsm_netconn;

/**
 * \brief           Netconn object structure
 */
typedef struct gsm_netconn* gsm_netconn_p;

/**
 * \brief           Netconn connection type
 */
typedef enum {
    GSM_NETCONN_TYPE_TCP = GSM_CONN_TYPE_TCP,   /*!< TCP connection */
    GSM_NETCONN_TYPE_UDP = GSM_CONN_TYPE_UDP,   /*!< UDP connection */
    GSM_NETCONN_TYPE_SSL = GSM_CONN_TYPE_SSL,   /*!< TCP connection over SSL */
} gsm_netconn_type_t;

gsm_netconn_p   gsm_netconn_new(gsm_netconn_type_t type);
gsmr_t          gsm_netconn_delete(gsm_netconn_p nc);
gsmr_t          gsm_netconn_connect(gsm_netconn_p nc, const char* host, gsm_port_t port);
gsmr_t          gsm_netconn_receive(gsm_netconn_p nc, gsm_pbuf_p* pbuf);
gsmr_t          gsm_netconn_close(gsm_netconn_p nc);
int8_t          gsm_netconn_getconnnum(gsm_netconn_p nc);
void            gsm_netconn_set_receive_timeout(gsm_netconn_p nc, uint32_t timeout);
uint32_t        gsm_netconn_get_receive_timeout(gsm_netconn_p nc);

/* TCP only */
gsmr_t          gsm_netconn_write(gsm_netconn_p nc, const void* data, size_t btw);
gsmr_t          gsm_netconn_flush(gsm_netconn_p nc);

/* UDP only */
gsmr_t          gsm_netconn_send(gsm_netconn_p nc, const void* data, size_t btw);
gsmr_t          gsm_netconn_sendto(gsm_netconn_p nc, const gsm_ip_t* ip, gsm_port_t port, const void* data, size_t btw);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GSM_HDR_NETCONN_H */
