/**
 * \file            gsm_mqtt_client_api.h
 * \brief           MQTT client API
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
#ifndef __GSM_APP_MQTT_CLIENT_API_H
#define __GSM_APP_MQTT_CLIENT_API_H

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif

#include "gsm/gsm.h"
#include "gsm/apps/gsm_mqtt_client.h"
    
/**
 * \ingroup         GSM_APPS
 * \defgroup        GSM_APP_MQTT_CLIENT_API MQTT client API
 * \brief           Sequential, single thread MQTT client API
 * \{
 */

/**
 * \brief           MQTT API client structure
 */
struct gsm_mqtt_client_api;

/**
 * \brief           MQTT API RX buffer
 */
typedef struct gsm_mqtt_client_api_buf {
    const char* topic;                          /*!< Topic data */
    size_t topic_len;                           /*!< Topic length */
    const uint8_t* payload;                     /*!< Payload data */
    size_t payload_len;                         /*!< Payload length */
    gsm_mqtt_qos_t qos;                         /*!< Quality of service */
} gsm_mqtt_client_api_buf_t;

/**
 * \brief           Pointer to \ref gsm_mqtt_client_api_t structure
 */
typedef struct gsm_mqtt_client_api* gsm_mqtt_client_api_p;

/**
 * \brief           Pointer to \ref gsm_mqtt_client_api_buf_t structure 
 */
typedef struct gsm_mqtt_client_api_buf* gsm_mqtt_client_api_buf_p;

gsm_mqtt_client_api_p   gsm_mqtt_client_api_new(size_t tx_buff_len, size_t rx_buff_len);
void                    gsm_mqtt_client_api_delete(gsm_mqtt_client_api_p client);
gsm_mqtt_conn_status_t  gsm_mqtt_client_api_connect(gsm_mqtt_client_api_p client, const char* host, gsm_port_t port, const gsm_mqtt_client_info_t* info);
gsmr_t                  gsm_mqtt_client_api_close(gsm_mqtt_client_api_p client);
gsmr_t                  gsm_mqtt_client_api_subscribe(gsm_mqtt_client_api_p client, const char* topic, gsm_mqtt_qos_t qos);
gsmr_t                  gsm_mqtt_client_api_unsubscribe(gsm_mqtt_client_api_p client, const char* topic);
gsmr_t                  gsm_mqtt_client_api_publish(gsm_mqtt_client_api_p client, const char* topic, const void* data, size_t btw, gsm_mqtt_qos_t qos, uint8_t retain);
gsmr_t                  gsm_mqtt_client_api_receive(gsm_mqtt_client_api_p client, gsm_mqtt_client_api_buf_p* p, uint32_t timeout);
void                    gsm_mqtt_client_api_buf_free(gsm_mqtt_client_api_buf_p p);
    
/**
 * \}
 */

/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif /* __GSM_APP_MQTT_CLIENT_H */
