/**
 * \file            lwgsm_mqtt_client_api.h
 * \brief           MQTT client API
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
#ifndef LWGSM_HDR_APP_MQTT_CLIENT_API_H
#define LWGSM_HDR_APP_MQTT_CLIENT_API_H

#include "lwgsm/lwgsm.h"
#include "lwgsm/apps/lwgsm_mqtt_client.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \ingroup         LWGSM_APPS
 * \defgroup        LWGSM_APP_MQTT_CLIENT_API MQTT client API
 * \brief           Sequential, single thread MQTT client API
 * \{
 */

/**
 * \brief           MQTT API client structure
 */
struct lwgsm_mqtt_client_api;

/**
 * \brief           MQTT API RX buffer
 */
typedef struct lwgsm_mqtt_client_api_buf {
    char* topic;                                /*!< Topic data */
    size_t topic_len;                           /*!< Topic length */
    uint8_t* payload;                           /*!< Payload data */
    size_t payload_len;                         /*!< Payload length */
    lwgsm_mqtt_qos_t qos;                       /*!< Quality of service */
} lwgsm_mqtt_client_api_buf_t;

/**
 * \brief           Pointer to \ref lwgsm_mqtt_client_api structure
 */
typedef struct lwgsm_mqtt_client_api* lwgsm_mqtt_client_api_p;

/**
 * \brief           Pointer to \ref lwgsm_mqtt_client_api_buf_t structure
 */
typedef struct lwgsm_mqtt_client_api_buf* lwgsm_mqtt_client_api_buf_p;

lwgsm_mqtt_client_api_p lwgsm_mqtt_client_api_new(size_t tx_buff_len, size_t rx_buff_len);
void                    lwgsm_mqtt_client_api_delete(lwgsm_mqtt_client_api_p client);
lwgsm_mqtt_conn_status_t    lwgsm_mqtt_client_api_connect(lwgsm_mqtt_client_api_p client, const char* host, lwgsm_port_t port, const lwgsm_mqtt_client_info_t* info);
lwgsmr_t                lwgsm_mqtt_client_api_close(lwgsm_mqtt_client_api_p client);
lwgsmr_t                lwgsm_mqtt_client_api_subscribe(lwgsm_mqtt_client_api_p client, const char* topic, lwgsm_mqtt_qos_t qos);
lwgsmr_t                lwgsm_mqtt_client_api_unsubscribe(lwgsm_mqtt_client_api_p client, const char* topic);
lwgsmr_t                lwgsm_mqtt_client_api_publish(lwgsm_mqtt_client_api_p client, const char* topic, const void* data, size_t btw, lwgsm_mqtt_qos_t qos, uint8_t retain);
uint8_t                 lwgsm_mqtt_client_api_is_connected(lwgsm_mqtt_client_api_p client);
lwgsmr_t                lwgsm_mqtt_client_api_receive(lwgsm_mqtt_client_api_p client, lwgsm_mqtt_client_api_buf_p* p, uint32_t timeout);
void                    lwgsm_mqtt_client_api_buf_free(lwgsm_mqtt_client_api_buf_p p);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWGSM_HDR_APP_MQTT_CLIENT_H */
