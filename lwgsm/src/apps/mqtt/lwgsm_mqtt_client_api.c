/**
 * \file            lwgsm_mqtt_client_api.c
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
#include "lwgsm/apps/lwgsm_mqtt_client_api.h"
#include "lwgsm/lwgsm_mem.h"

/* Tracing debug message */
#define LWGSM_CFG_DBG_MQTT_API_TRACE              (LWGSM_CFG_DBG_MQTT_API | LWGSM_DBG_TYPE_TRACE)
#define LWGSM_CFG_DBG_MQTT_API_STATE              (LWGSM_CFG_DBG_MQTT_API | LWGSM_DBG_TYPE_STATE)
#define LWGSM_CFG_DBG_MQTT_API_TRACE_WARNING      (LWGSM_CFG_DBG_MQTT_API | LWGSM_DBG_TYPE_TRACE | LWGSM_DBG_LVL_WARNING)
#define LWGSM_CFG_DBG_MQTT_API_TRACE_SEVERE       (LWGSM_CFG_DBG_MQTT_API | LWGSM_DBG_TYPE_TRACE | LWGSM_DBG_LVL_SEVERE)

/**
 * \brief           MQTT API client structure
 */
struct lwgsm_mqtt_client_api {
    lwgsm_mqtt_client_p mc;                     /*!< MQTT client handle */
    lwgsm_sys_mbox_t rcv_mbox;                  /*!< Received data mbox */
    lwgsm_sys_sem_t sync_sem;                   /*!< Synchronization semaphore */
    lwgsm_sys_mutex_t mutex;                    /*!< Mutex handle */
    uint8_t release_sem;                        /*!< Set to `1` to release semaphore */
    lwgsm_mqtt_conn_status_t connect_resp;      /*!< Response when connecting to server */
    lwgsmr_t sub_pub_resp;                      /*!< Subscribe/Unsubscribe/Publish response */
} lwgsm_mqtt_client_api_t;

/**
 * \brief           Variable used as pointer for message queue when MQTT connection is closed
 */
static uint8_t
mqtt_closed = 0xFF;

/**
 * \brief           Release user semaphore
 * \param[in]       client: Client handle
 */
static void
release_sem(lwgsm_mqtt_client_api_p client) {
    if (client->release_sem) {
        client->release_sem = 0;
        lwgsm_sys_sem_release(&client->sync_sem);
    }
}

/**
 * \brief           MQTT event callback function
 */
static void
mqtt_evt(lwgsm_mqtt_client_p client, lwgsm_mqtt_evt_t* evt) {
    lwgsm_mqtt_client_api_p api_client = lwgsm_mqtt_client_get_arg(client);
    if (api_client == NULL) {
        return;
    }
    switch (lwgsm_mqtt_client_evt_get_type(client, evt)) {
        case LWGSM_MQTT_EVT_CONNECT: {
            lwgsm_mqtt_conn_status_t status = lwgsm_mqtt_client_evt_connect_get_status(client, evt);

            LWGSM_DEBUGF(LWGSM_CFG_DBG_MQTT_API_TRACE,
                       "[MQTT API] Connect event with status: %d\r\n", (int)status);

            api_client->connect_resp = status;

            /*
             * By MQTT 3.1.1 specification, broker must close connection
             * if client CONNECT packet was not accepted.
             *
             * If client is accepted or connection did not even start,
             * release semaphore, otherwise wait CLOSED event
             * and release semaphore from there,
             * to make sure we are fully ready for next connection
             */
            if (status == LWGSM_MQTT_CONN_STATUS_TCP_FAILED
                || status == LWGSM_MQTT_CONN_STATUS_ACCEPTED) {
                release_sem(api_client);        /* Release semaphore */
            }

            break;
        }
        case LWGSM_MQTT_EVT_PUBLISH_RECV: {
            /* Check valid receive mbox */
            if (lwgsm_sys_mbox_isvalid(&api_client->rcv_mbox)) {
                lwgsm_mqtt_client_api_buf_p buf;
                size_t size, buf_size, topic_size, payload_size;

                /* Get event data */
                const char* topic = lwgsm_mqtt_client_evt_publish_recv_get_topic(client, evt);
                size_t topic_len = lwgsm_mqtt_client_evt_publish_recv_get_topic_len(client, evt);
                const uint8_t* payload = lwgsm_mqtt_client_evt_publish_recv_get_payload(client, evt);
                size_t payload_len = lwgsm_mqtt_client_evt_publish_recv_get_payload_len(client, evt);
                lwgsm_mqtt_qos_t qos = lwgsm_mqtt_client_evt_publish_recv_get_qos(client, evt);

                /* Print debug message */
                LWGSM_DEBUGF(LWGSM_CFG_DBG_MQTT_API_TRACE,
                           "[MQTT API] New publish received on topic %.*s\r\n", (int)topic_len, topic);

                /* Calculate memory sizes */
                buf_size = LWGSM_MEM_ALIGN(sizeof(*buf));
                topic_size = LWGSM_MEM_ALIGN(sizeof(*topic) * (topic_len + 1));
                payload_size = LWGSM_MEM_ALIGN(sizeof(*payload) * (payload_len + 1));

                size = buf_size + topic_size + payload_size;
                buf = lwgsm_mem_malloc(size);
                if (buf != NULL) {
                    LWGSM_MEMSET(buf, 0x00, size);
                    buf->topic = (void*)((uint8_t*)buf + buf_size);
                    buf->payload = (void*)((uint8_t*)buf + buf_size + topic_size);
                    buf->topic_len = topic_len;
                    buf->payload_len = payload_len;
                    buf->qos = qos;

                    /* Copy content to new memory */
                    LWGSM_MEMCPY(buf->topic, topic, sizeof(*topic) * topic_len);
                    LWGSM_MEMCPY(buf->payload, payload, sizeof(*payload) * payload_len);

                    /* Write to receive queue */
                    if (!lwgsm_sys_mbox_putnow(&api_client->rcv_mbox, buf)) {
                        LWGSM_DEBUGF(LWGSM_CFG_DBG_MQTT_API_TRACE_WARNING,
                                   "[MQTT API] Cannot put new received MQTT publish to queue\r\n");
                        lwgsm_mem_free_s((void**)&buf);
                    }
                } else {
                    LWGSM_DEBUGF(LWGSM_CFG_DBG_MQTT_API_TRACE_WARNING,
                               "[MQTT API] Cannot allocate memory for packet buffer of size %d bytes\r\n",
                               (int)size);
                }
            }
            break;
        }
        case LWGSM_MQTT_EVT_PUBLISH: {
            api_client->sub_pub_resp = lwgsm_mqtt_client_evt_publish_get_result(client, evt);

            /* Print debug message */
            LWGSM_DEBUGF(LWGSM_CFG_DBG_MQTT_API_TRACE,
                       "[MQTT API] Publish event with response: %d\r\n",
                       (int)api_client->sub_pub_resp);

            release_sem(api_client);            /* Release semaphore */
            break;
        }
        case LWGSM_MQTT_EVT_SUBSCRIBE: {
            api_client->sub_pub_resp = lwgsm_mqtt_client_evt_subscribe_get_result(client, evt);

            /* Print debug message */
            LWGSM_DEBUGF(LWGSM_CFG_DBG_MQTT_API_TRACE,
                       "[MQTT API] Subscribe event with response: %d\r\n",
                       (int)api_client->sub_pub_resp);

            release_sem(api_client);            /* Release semaphore */
            break;
        }
        case LWGSM_MQTT_EVT_UNSUBSCRIBE: {
            api_client->sub_pub_resp = lwgsm_mqtt_client_evt_unsubscribe_get_result(client, evt);

            /* Print debug message */
            LWGSM_DEBUGF(LWGSM_CFG_DBG_MQTT_API_TRACE,
                       "[MQTT API] Unsubscribe event with response: %d\r\n",
                       (int)api_client->sub_pub_resp);

            release_sem(api_client);            /* Release semaphore */
            break;
        }
        case LWGSM_MQTT_EVT_DISCONNECT: {
            uint8_t is_accepted = lwgsm_mqtt_client_evt_disconnect_is_accepted(client, evt);
            /* Disconnect event happened */
            //api_client->connect_resp = MQTT_CONN_STATUS_TCP_FAILED;

            /* Print debug message */
            LWGSM_DEBUGF(LWGSM_CFG_DBG_MQTT_API_TRACE,
                       "[MQTT API] Disconnect event\r\n");

            /* Write to receive mbox to wakeup receive thread */
            if (is_accepted && lwgsm_sys_mbox_isvalid(&api_client->rcv_mbox)) {
                lwgsm_sys_mbox_putnow(&api_client->rcv_mbox, &mqtt_closed);
            }

            release_sem(api_client);            /* Release semaphore */
            break;
        }
        default:
            break;
    }
}

/**
 * \brief           Create new MQTT client API
 * \param[in]       tx_buff_len: Maximal TX buffer for maximal packet length
 * \param[in]       rx_buff_len: Maximal RX buffer
 * \return          Client handle on success, `NULL` otherwise
 */
lwgsm_mqtt_client_api_p
lwgsm_mqtt_client_api_new(size_t tx_buff_len, size_t rx_buff_len) {
    lwgsm_mqtt_client_api_p client;
    size_t size;

    size = LWGSM_MEM_ALIGN(sizeof(*client));    /* Get size of client itself */

    /* Create client APi structure */
    client = lwgsm_mem_calloc(1, size);         /* Allocate client memory */
    if (client != NULL) {
        /* Create MQTT raw client structure */
        client->mc = lwgsm_mqtt_client_new(tx_buff_len, rx_buff_len);
        if (client->mc != NULL) {
            /* Create receive mbox queue */
            if (lwgsm_sys_mbox_create(&client->rcv_mbox, 5)) {
                /* Create synchronization semaphore */
                if (lwgsm_sys_sem_create(&client->sync_sem, 1)) {
                    /* Create mutex */
                    if (lwgsm_sys_mutex_create(&client->mutex)) {
                        lwgsm_mqtt_client_set_arg(client->mc, client);  /* Set client to mqtt client argument */
                        return client;
                    } else {
                        LWGSM_DEBUGF(LWGSM_CFG_DBG_MQTT_API,
                                   "[MQTT API] Cannot allocate mutex\r\n");
                    }
                } else {
                    LWGSM_DEBUGF(LWGSM_CFG_DBG_MQTT_API,
                               "[MQTT API] Cannot allocate sync semaphore\r\n");
                }
            } else {
                LWGSM_DEBUGF(LWGSM_CFG_DBG_MQTT_API,
                           "[MQTT API] Cannot allocate receive queue\r\n");
            }
        } else {
            LWGSM_DEBUGF(LWGSM_CFG_DBG_MQTT_API,
                       "[MQTT API] Cannot allocate MQTT client\r\n");
        }
    } else {
        LWGSM_DEBUGF(LWGSM_CFG_DBG_MQTT_API,
                   "[MQTT API] Cannot allocate memory for client\r\n");
    }

    lwgsm_mqtt_client_api_delete(client);
    client = NULL;
    return NULL;
}

/**
 * \brief           Delete client from memory
 * \param[in]       client: MQTT API client handle
 */
void
lwgsm_mqtt_client_api_delete(lwgsm_mqtt_client_api_p client) {
    if (client == NULL) {
        return;
    }
    if (lwgsm_sys_sem_isvalid(&client->sync_sem)) {
        lwgsm_sys_sem_delete(&client->sync_sem);
        lwgsm_sys_sem_invalid(&client->sync_sem);
    }
    if (lwgsm_sys_mutex_isvalid(&client->mutex)) {
        lwgsm_sys_mutex_delete(&client->mutex);
        lwgsm_sys_mutex_invalid(&client->mutex);
    }
    if (lwgsm_sys_mbox_isvalid(&client->rcv_mbox)) {
        void* d;
        while (lwgsm_sys_mbox_getnow(&client->rcv_mbox, &d)) {
            if ((uint8_t*)d != (uint8_t*)&mqtt_closed) {
                lwgsm_mqtt_client_api_buf_free(d);
            }
        }
        lwgsm_sys_mbox_delete(&client->rcv_mbox);
        lwgsm_sys_mbox_invalid(&client->rcv_mbox);
    }
    if (client->mc != NULL) {
        lwgsm_mqtt_client_delete(client->mc);
        client->mc = NULL;
    }
    lwgsm_mem_free_s((void**)&client);
}

/**
 * \brief           Connect to MQTT broker
 * \param[in]       client: MQTT API client handle
 * \param[in]       host: TCP host
 * \param[in]       port: TCP port
 * \param[in]       info: MQTT client info
 * \return          \ref LWGSM_MQTT_CONN_STATUS_ACCEPTED on success, member of \ref lwgsm_mqtt_conn_status_t otherwise
 */
lwgsm_mqtt_conn_status_t
lwgsm_mqtt_client_api_connect(lwgsm_mqtt_client_api_p client, const char* host,
                            lwgsm_port_t port, const lwgsm_mqtt_client_info_t* info) {
    if (client == NULL || host == NULL
        || !port || info == NULL) {
        LWGSM_DEBUGF(LWGSM_CFG_DBG_MQTT_API_TRACE_WARNING,
                   "[MQTT API] Invalid parameters in function\r\n");
        return LWGSM_MQTT_CONN_STATUS_TCP_FAILED;
    }

    lwgsm_sys_mutex_lock(&client->mutex);
    client->connect_resp = LWGSM_MQTT_CONN_STATUS_TCP_FAILED;
    lwgsm_sys_sem_wait(&client->sync_sem, 0);
    client->release_sem = 1;
    if (lwgsm_mqtt_client_connect(client->mc, host, port, mqtt_evt, info) == lwgsmOK) {
        lwgsm_sys_sem_wait(&client->sync_sem, 0);
    } else {
        LWGSM_DEBUGF(LWGSM_CFG_DBG_MQTT_API_TRACE_WARNING,
                   "[MQTT API] Cannot connect to %s\r\n", host);
    }
    client->release_sem = 0;
    lwgsm_sys_sem_release(&client->sync_sem);
    lwgsm_sys_mutex_unlock(&client->mutex);
    return client->connect_resp;
}

/**
 * \brief           Close MQTT connection
 * \param[in]       client: MQTT API client handle
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_mqtt_client_api_close(lwgsm_mqtt_client_api_p client) {
    lwgsmr_t res = lwgsmERR;

    LWGSM_ASSERT("client != NULL", client != NULL);

    lwgsm_sys_mutex_lock(&client->mutex);
    lwgsm_sys_sem_wait(&client->sync_sem, 0);
    client->release_sem = 1;
    if (lwgsm_mqtt_client_disconnect(client->mc) == lwgsmOK) {
        res = lwgsmOK;
        lwgsm_sys_sem_wait(&client->sync_sem, 0);
    } else {
        LWGSM_DEBUGF(LWGSM_CFG_DBG_MQTT_API_TRACE_WARNING,
                   "[MQTT API] Cannot close API connection\r\n");
    }
    client->release_sem = 0;
    lwgsm_sys_sem_release(&client->sync_sem);
    lwgsm_sys_mutex_unlock(&client->mutex);
    return res;
}

/**
 * \brief           Subscribe to topic
 * \param[in]       client: MQTT API client handle
 * \param[in]       topic: Topic to subscribe on
 * \param[in]       qos: Quality of service. This parameter can be a value of \ref lwgsm_mqtt_qos_t
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_mqtt_client_api_subscribe(lwgsm_mqtt_client_api_p client, const char* topic,
                              lwgsm_mqtt_qos_t qos) {
    lwgsmr_t res = lwgsmERR;

    LWGSM_ASSERT("client != NULL", client != NULL);
    LWGSM_ASSERT("topic != NULL", topic != NULL);

    lwgsm_sys_mutex_lock(&client->mutex);
    lwgsm_sys_sem_wait(&client->sync_sem, 0);
    client->release_sem = 1;
    if (lwgsm_mqtt_client_subscribe(client->mc, topic, qos, NULL) == lwgsmOK) {
        lwgsm_sys_sem_wait(&client->sync_sem, 0);
        res = client->sub_pub_resp;
    } else {
        LWGSM_DEBUGF(LWGSM_CFG_DBG_MQTT_API_TRACE_WARNING,
                   "[MQTT API] Cannot subscribe to topic %s\r\n", topic);
    }
    client->release_sem = 0;
    lwgsm_sys_sem_release(&client->sync_sem);
    lwgsm_sys_mutex_unlock(&client->mutex);

    return res;
}

/**
 * \brief           Unsubscribe from topic
 * \param[in]       client: MQTT API client handle
 * \param[in]       topic: Topic to unsubscribe from
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_mqtt_client_api_unsubscribe(lwgsm_mqtt_client_api_p client, const char* topic) {
    lwgsmr_t res = lwgsmERR;

    LWGSM_ASSERT("client != NULL", client != NULL);
    LWGSM_ASSERT("topic != NULL", topic != NULL);

    lwgsm_sys_mutex_lock(&client->mutex);
    lwgsm_sys_sem_wait(&client->sync_sem, 0);
    client->release_sem = 1;
    if (lwgsm_mqtt_client_unsubscribe(client->mc, topic, NULL) == lwgsmOK) {
        lwgsm_sys_sem_wait(&client->sync_sem, 0);
        res = client->sub_pub_resp;
    } else {
        LWGSM_DEBUGF(LWGSM_CFG_DBG_MQTT_API_TRACE_WARNING,
                   "[MQTT API] Cannot unsubscribe from topic %s\r\n", topic);
    }
    client->release_sem = 0;
    lwgsm_sys_sem_release(&client->sync_sem);
    lwgsm_sys_mutex_unlock(&client->mutex);

    return res;
}

/**
 * \brief           Publish new packet to MQTT network
 * \param[in]       client: MQTT API client handle
 * \param[in]       topic: Topic to publish on
 * \param[in]       data: Data to send
 * \param[in]       btw: Number of bytes to send for data parameter
 * \param[in]       qos: Quality of service. This parameter can be a value of \ref lwgsm_mqtt_qos_t
 * \param[in]       retain: Set to `1` for retain flag, `0` otherwise
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_mqtt_client_api_publish(lwgsm_mqtt_client_api_p client, const char* topic, const void* data,
                            size_t btw, lwgsm_mqtt_qos_t qos, uint8_t retain) {
    lwgsmr_t res = lwgsmERR;

    LWGSM_ASSERT("client != NULL", client != NULL);
    LWGSM_ASSERT("topic != NULL", topic != NULL);
    LWGSM_ASSERT("data != NULL", data != NULL);
    LWGSM_ASSERT("btw > 0", btw > 0);

    lwgsm_sys_mutex_lock(&client->mutex);
    lwgsm_sys_sem_wait(&client->sync_sem, 0);
    client->release_sem = 1;
    if (lwgsm_mqtt_client_publish(client->mc, topic, data, LWGSM_U16(btw), qos, retain, NULL) == lwgsmOK) {
        lwgsm_sys_sem_wait(&client->sync_sem, 0);
        res = client->sub_pub_resp;
    } else {
        LWGSM_DEBUGF(LWGSM_CFG_DBG_MQTT_API_TRACE_WARNING,
                   "[MQTT API] Cannot publish new packet\r\n");
    }
    client->release_sem = 0;
    lwgsm_sys_sem_release(&client->sync_sem);
    lwgsm_sys_mutex_unlock(&client->mutex);

    return res;
}

/**
 * \brief           Check if client MQTT connection is active
 * \param[in]       client: MQTT API client handle
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_mqtt_client_api_is_connected(lwgsm_mqtt_client_api_p client) {
    uint8_t ret;

    if (client == NULL) {
        return 0;
    }

    lwgsm_sys_mutex_lock(&client->mutex);
    ret = lwgsm_mqtt_client_is_connected(client->mc);
    lwgsm_sys_mutex_unlock(&client->mutex);
    return ret;
}

/**
 * \brief           Receive next packet in specific timeout time
 * \note            This function can be called from separate thread
 *                      than the rest of API function, which allows you to
 *                      handle receive data separated with custom timeout
 * \param[in]       client: MQTT API client handle
 * \param[in]       p: Pointer to output buffer
 * \param[in]       timeout: Maximal time to wait before function returns timeout
 * \return          \ref lwgsmOK on success, \ref lwgsmCLOSED if MQTT is closed, \ref lwgsmTIMEOUT on timeout
 */
lwgsmr_t
lwgsm_mqtt_client_api_receive(lwgsm_mqtt_client_api_p client, lwgsm_mqtt_client_api_buf_p* p,
                            uint32_t timeout) {
    LWGSM_ASSERT("client != NULL", client != NULL);
    LWGSM_ASSERT("p != NULL", p != NULL);

    *p = NULL;

    /* Get new entry from mbox */
    if (timeout == 0) {
        if (!lwgsm_sys_mbox_getnow(&client->rcv_mbox, (void**)p)) {
            return lwgsmTIMEOUT;
        }
    } else if (lwgsm_sys_mbox_get(&client->rcv_mbox, (void**)p, timeout) == LWGSM_SYS_TIMEOUT) {
        return lwgsmTIMEOUT;
    }

    /* Check for MQTT closed event */
    if ((uint8_t*)(*p) == (uint8_t*)&mqtt_closed) {
        LWGSM_DEBUGF(LWGSM_CFG_DBG_MQTT_API_TRACE,
                   "[MQTT API] Closed event received from queue\r\n");

        *p = NULL;
        return lwgsmCLOSED;
    }
    return lwgsmOK;
}

/**
 * \brief           Free buffer memory after usage
 * \param[in]       p: Buffer to free
 */
void
lwgsm_mqtt_client_api_buf_free(lwgsm_mqtt_client_api_buf_p p) {
    lwgsm_mem_free_s((void**)&p);
}
