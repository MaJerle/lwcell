/**
 * \file            lwcell_mqtt_client_api.h
 * \brief           MQTT client API
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
#ifndef LWCELL_APP_MQTT_CLIENT_API_HDR_H
#define LWCELL_APP_MQTT_CLIENT_API_HDR_H

#include "lwcell/apps/lwcell_mqtt_client.h"
#include "lwcell/lwcell_includes.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \ingroup         LWCELL_APPS
 * \defgroup        LWCELL_APP_MQTT_CLIENT_API MQTT client API
 * \brief           Sequential, single thread MQTT client API
 * \{
 */

/**
 * \brief           MQTT API client structure
 */
struct lwcell_mqtt_client_api;

/**
 * \brief           MQTT API RX buffer
 */
typedef struct lwcell_mqtt_client_api_buf {
    char* topic;          /*!< Topic data */
    size_t topic_len;     /*!< Topic length */
    uint8_t* payload;     /*!< Payload data */
    size_t payload_len;   /*!< Payload length */
    lwcell_mqtt_qos_t qos; /*!< Quality of service */
} lwcell_mqtt_client_api_buf_t;

/**
 * \brief           Pointer to \ref lwcell_mqtt_client_api structure
 */
typedef struct lwcell_mqtt_client_api* lwcell_mqtt_client_api_p;

/**
 * \brief           Pointer to \ref lwcell_mqtt_client_api_buf_t structure
 */
typedef struct lwcell_mqtt_client_api_buf* lwcell_mqtt_client_api_buf_p;

lwcell_mqtt_client_api_p lwcell_mqtt_client_api_new(size_t tx_buff_len, size_t rx_buff_len);
void lwcell_mqtt_client_api_delete(lwcell_mqtt_client_api_p client);
lwcell_mqtt_conn_status_t lwcell_mqtt_client_api_connect(lwcell_mqtt_client_api_p client, const char* host,
                                                       lwcell_port_t port, const lwcell_mqtt_client_info_t* info);
lwcellr_t lwcell_mqtt_client_api_close(lwcell_mqtt_client_api_p client);
lwcellr_t lwcell_mqtt_client_api_subscribe(lwcell_mqtt_client_api_p client, const char* topic, lwcell_mqtt_qos_t qos);
lwcellr_t lwcell_mqtt_client_api_unsubscribe(lwcell_mqtt_client_api_p client, const char* topic);
lwcellr_t lwcell_mqtt_client_api_publish(lwcell_mqtt_client_api_p client, const char* topic, const void* data, size_t btw,
                                       lwcell_mqtt_qos_t qos, uint8_t retain);
uint8_t lwcell_mqtt_client_api_is_connected(lwcell_mqtt_client_api_p client);
lwcellr_t lwcell_mqtt_client_api_receive(lwcell_mqtt_client_api_p client, lwcell_mqtt_client_api_buf_p* p,
                                       uint32_t timeout);
void lwcell_mqtt_client_api_buf_free(lwcell_mqtt_client_api_buf_p p);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWCELL_HDR_APP_MQTT_CLIENT_H */
