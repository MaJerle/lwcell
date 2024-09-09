/**
 * \file            lwcell_mqtt_client_api.c
 * \brief           MQTT client API
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
#include "lwcell/apps/lwcell_mqtt_client_api.h"
#include "lwcell/lwcell.h"
#include "lwcell/lwcell_private.h"

/* Tracing debug message */
#define LWCELL_CFG_DBG_MQTT_API_TRACE         (LWCELL_CFG_DBG_MQTT_API | LWCELL_DBG_TYPE_TRACE)
#define LWCELL_CFG_DBG_MQTT_API_STATE         (LWCELL_CFG_DBG_MQTT_API | LWCELL_DBG_TYPE_STATE)
#define LWCELL_CFG_DBG_MQTT_API_TRACE_WARNING (LWCELL_CFG_DBG_MQTT_API | LWCELL_DBG_TYPE_TRACE | LWCELL_DBG_LVL_WARNING)
#define LWCELL_CFG_DBG_MQTT_API_TRACE_SEVERE  (LWCELL_CFG_DBG_MQTT_API | LWCELL_DBG_TYPE_TRACE | LWCELL_DBG_LVL_SEVERE)

/**
 * \brief           MQTT API client structure
 */
struct lwcell_mqtt_client_api {
    lwcell_mqtt_client_p mc;                /*!< MQTT client handle */
    lwcell_sys_mbox_t rcv_mbox;             /*!< Received data mbox */
    lwcell_sys_sem_t sync_sem;              /*!< Synchronization semaphore */
    lwcell_sys_mutex_t mutex;               /*!< Mutex handle */
    uint8_t release_sem;                   /*!< Set to `1` to release semaphore */
    lwcell_mqtt_conn_status_t connect_resp; /*!< Response when connecting to server */
    lwcellr_t sub_pub_resp;                 /*!< Subscribe/Unsubscribe/Publish response */
} lwcell_mqtt_client_api_t;

/**
 * \brief           Variable used as pointer for message queue when MQTT connection is closed
 */
static uint8_t mqtt_closed = 0xFF;

/**
 * \brief           Release user semaphore
 * \param[in]       client: Client handle
 */
static void
prv_release_sem(lwcell_mqtt_client_api_p client) {
    if (client->release_sem) {
        client->release_sem = 0;
        lwcell_sys_sem_release(&client->sync_sem);
    }
}

/**
 * \brief           MQTT event callback function
 */
static void
prv_mqtt_evt(lwcell_mqtt_client_p client, lwcell_mqtt_evt_t* evt) {
    lwcell_mqtt_client_api_p api_client = lwcell_mqtt_client_get_arg(client);
    if (api_client == NULL) {
        return;
    }
    switch (lwcell_mqtt_client_evt_get_type(client, evt)) {
        case LWCELL_MQTT_EVT_CONNECT: {
            lwcell_mqtt_conn_status_t status = lwcell_mqtt_client_evt_connect_get_status(client, evt);

            LWCELL_DEBUGF(LWCELL_CFG_DBG_MQTT_API_TRACE, "[MQTT API] Connect event with status: %d\r\n", (int)status);

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
            if (status == LWCELL_MQTT_CONN_STATUS_TCP_FAILED || status == LWCELL_MQTT_CONN_STATUS_ACCEPTED) {
                prv_release_sem(api_client); /* Release semaphore */
            }

            break;
        }
        case LWCELL_MQTT_EVT_PUBLISH_RECV: {
            /* Check valid receive mbox */
            if (!lwcell_sys_mbox_isvalid(&api_client->rcv_mbox)) {
                break;
            }
            lwcell_mqtt_client_api_buf_p buf;
            size_t size, buf_size, topic_size, payload_size;

            /* Get event data */
            const char* topic = lwcell_mqtt_client_evt_publish_recv_get_topic(client, evt);
            size_t topic_len = lwcell_mqtt_client_evt_publish_recv_get_topic_len(client, evt);
            const uint8_t* payload = lwcell_mqtt_client_evt_publish_recv_get_payload(client, evt);
            size_t payload_len = lwcell_mqtt_client_evt_publish_recv_get_payload_len(client, evt);
            lwcell_mqtt_qos_t qos = lwcell_mqtt_client_evt_publish_recv_get_qos(client, evt);
            uint8_t retain = lwcell_mqtt_client_evt_publish_recv_get_retain(client, evt);

            /* Print debug message */
            LWCELL_DEBUGF(LWCELL_CFG_DBG_MQTT_API_TRACE, "[MQTT API] New publish received on topic %.*s\r\n",
                          (int)topic_len, topic);

            /* Calculate memory sizes */
            buf_size = LWCELL_MEM_ALIGN(sizeof(*buf));
            topic_size = LWCELL_MEM_ALIGN(sizeof(*topic) * (topic_len + 1));
            payload_size = LWCELL_MEM_ALIGN(sizeof(*payload) * (payload_len + 1));

            size = buf_size + topic_size + payload_size;
            if ((buf = lwcell_mem_malloc(size)) != NULL) {
                LWCELL_MEMSET(buf, 0x00, size);
                buf->topic = (void*)((uint8_t*)buf + buf_size);
                buf->payload = (void*)((uint8_t*)buf + buf_size + topic_size);
                buf->topic_len = topic_len;
                buf->payload_len = payload_len;
                buf->qos = qos;
                buf->retain = retain;

                /* Copy content to new memory */
                LWCELL_MEMCPY(buf->topic, topic, sizeof(*topic) * topic_len);
                LWCELL_MEMCPY(buf->payload, payload, sizeof(*payload) * payload_len);

                /* Write to receive queue */
                if (!lwcell_sys_mbox_putnow(&api_client->rcv_mbox, buf)) {
                    LWCELL_DEBUGF(LWCELL_CFG_DBG_MQTT_API_TRACE_WARNING,
                                  "[MQTT API] Cannot put new received MQTT publish to queue\r\n");
                    lwcell_mem_free_s((void**)&buf);
                }
            } else {
                LWCELL_DEBUGF(LWCELL_CFG_DBG_MQTT_API_TRACE_WARNING,
                              "[MQTT API] Cannot allocate memory for packet buffer of size %d bytes\r\n", (int)size);
            }
            break;
        }
        case LWCELL_MQTT_EVT_PUBLISH: {
            api_client->sub_pub_resp = lwcell_mqtt_client_evt_publish_get_result(client, evt);
            prv_release_sem(api_client); /* Release semaphore */
            LWCELL_DEBUGF(LWCELL_CFG_DBG_MQTT_API_TRACE, "[MQTT API] Publish event with response: %d\r\n",
                          (int)api_client->sub_pub_resp);
            break;
        }
        case LWCELL_MQTT_EVT_SUBSCRIBE: {
            api_client->sub_pub_resp = lwcell_mqtt_client_evt_subscribe_get_result(client, evt);
            prv_release_sem(api_client); /* Release semaphore */
            LWCELL_DEBUGF(LWCELL_CFG_DBG_MQTT_API_TRACE, "[MQTT API] Subscribe event with response: %d\r\n",
                          (int)api_client->sub_pub_resp);
            break;
        }
        case LWCELL_MQTT_EVT_UNSUBSCRIBE: {
            api_client->sub_pub_resp = lwcell_mqtt_client_evt_unsubscribe_get_result(client, evt);
            prv_release_sem(api_client); /* Release semaphore */
            LWCELL_DEBUGF(LWCELL_CFG_DBG_MQTT_API_TRACE, "[MQTT API] Unsubscribe event with response: %d\r\n",
                          (int)api_client->sub_pub_resp);
            break;
        }
        case LWCELL_MQTT_EVT_DISCONNECT: {
            uint8_t is_accepted = lwcell_mqtt_client_evt_disconnect_is_accepted(client, evt);
            /* Disconnect event happened */
            //api_client->connect_resp = MQTT_CONN_STATUS_TCP_FAILED;

            /* Print debug message */
            LWCELL_DEBUGF(LWCELL_CFG_DBG_MQTT_API_TRACE, "[MQTT API] Disconnect event\r\n");

            /* Write to receive mbox to wakeup receive thread */
            if (is_accepted && lwcell_sys_mbox_isvalid(&api_client->rcv_mbox)) {
                lwcell_sys_mbox_putnow(&api_client->rcv_mbox, &mqtt_closed);
            }
            prv_release_sem(api_client); /* Release semaphore */
            break;
        }
        default: break;
    }
}

/**
 * \brief           Create new MQTT client API
 * \param[in]       tx_buff_len: Maximal TX buffer for maximal packet length
 * \param[in]       rx_buff_len: Maximal RX buffer
 * \return          Client handle on success, `NULL` otherwise
 */
lwcell_mqtt_client_api_p
lwcell_mqtt_client_api_new(size_t tx_buff_len, size_t rx_buff_len) {
    lwcell_mqtt_client_api_p client;

    /* Allocate client memory */
    if ((client = lwcell_mem_calloc(1, LWCELL_MEM_ALIGN(sizeof(*client)))) != NULL) {
        /* Create MQTT raw client structure */
        if ((client->mc = lwcell_mqtt_client_new(tx_buff_len, rx_buff_len)) != NULL) {
            /* Create receive mbox queue */
            if (lwcell_sys_mbox_create(&client->rcv_mbox, LWCELL_CFG_MQTT_API_MBOX_SIZE)) {
                /* Create synchronization semaphore */
                if (lwcell_sys_sem_create(&client->sync_sem, 1)) {
                    /* Create mutex */
                    if (lwcell_sys_mutex_create(&client->mutex)) {
                        lwcell_mqtt_client_set_arg(client->mc, client); /* Set client to mqtt client argument */
                        return client;
                    } else {
                        LWCELL_DEBUGF(LWCELL_CFG_DBG_MQTT_API_TRACE_SEVERE, "[MQTT API] Cannot allocate mutex\r\n");
                    }
                } else {
                    LWCELL_DEBUGF(LWCELL_CFG_DBG_MQTT_API_TRACE_SEVERE, "[MQTT API] Cannot allocate sync semaphore\r\n");
                }
            } else {
                LWCELL_DEBUGF(LWCELL_CFG_DBG_MQTT_API_TRACE_SEVERE, "[MQTT API] Cannot allocate receive queue\r\n");
            }
        } else {
            LWCELL_DEBUGF(LWCELL_CFG_DBG_MQTT_API_TRACE_SEVERE, "[MQTT API] Cannot allocate MQTT client\r\n");
        }
    } else {
        LWCELL_DEBUGF(LWCELL_CFG_DBG_MQTT_API_TRACE_SEVERE, "[MQTT API] Cannot allocate memory for client\r\n");
    }
    lwcell_mqtt_client_api_delete(client);
    client = NULL;
    return NULL;
}

/**
 * \brief           Delete client from memory
 * \param[in]       client: MQTT API client handle
 */
void
lwcell_mqtt_client_api_delete(lwcell_mqtt_client_api_p client) {
    if (client == NULL) {
        return;
    }
    if (lwcell_sys_sem_isvalid(&client->sync_sem)) {
        lwcell_sys_sem_delete(&client->sync_sem);
        lwcell_sys_sem_invalid(&client->sync_sem);
    }
    if (lwcell_sys_mutex_isvalid(&client->mutex)) {
        lwcell_sys_mutex_delete(&client->mutex);
        lwcell_sys_mutex_invalid(&client->mutex);
    }
    if (lwcell_sys_mbox_isvalid(&client->rcv_mbox)) {
        void* d;
        while (lwcell_sys_mbox_getnow(&client->rcv_mbox, &d)) {
            if ((uint8_t*)d != (uint8_t*)&mqtt_closed) {
                lwcell_mqtt_client_api_buf_free(d);
            }
        }
        lwcell_sys_mbox_delete(&client->rcv_mbox);
        lwcell_sys_mbox_invalid(&client->rcv_mbox);
    }
    if (client->mc != NULL) {
        lwcell_mqtt_client_delete(client->mc);
        client->mc = NULL;
    }
    lwcell_mem_free_s((void**)&client);
}

/**
 * \brief           Connect to MQTT broker
 * \param[in]       client: MQTT API client handle
 * \param[in]       host: TCP host
 * \param[in]       port: TCP port
 * \param[in]       info: MQTT client info
 * \return          \ref LWCELL_MQTT_CONN_STATUS_ACCEPTED on success, member of \ref lwcell_mqtt_conn_status_t otherwise
 */
lwcell_mqtt_conn_status_t
lwcell_mqtt_client_api_connect(lwcell_mqtt_client_api_p client, const char* host, lwcell_port_t port,
                               const lwcell_mqtt_client_info_t* info) {
    if (client == NULL || host == NULL || !port || info == NULL) {
        LWCELL_DEBUGF(LWCELL_CFG_DBG_MQTT_API_TRACE_WARNING, "[MQTT API] Invalid parameters in function\r\n");
        return LWCELL_MQTT_CONN_STATUS_TCP_FAILED;
    }

    lwcell_sys_mutex_lock(&client->mutex);
    client->connect_resp = LWCELL_MQTT_CONN_STATUS_TCP_FAILED;
    lwcell_sys_sem_wait(&client->sync_sem, 0);
    client->release_sem = 1;
    if (lwcell_mqtt_client_connect(client->mc, host, port, prv_mqtt_evt, info) == lwcellOK) {
        lwcell_sys_sem_wait(&client->sync_sem, 0);
    } else {
        LWCELL_DEBUGF(LWCELL_CFG_DBG_MQTT_API_TRACE_WARNING, "[MQTT API] Cannot connect to %s\r\n", host);
    }
    client->release_sem = 0;
    lwcell_sys_sem_release(&client->sync_sem);
    lwcell_sys_mutex_unlock(&client->mutex);
    return client->connect_resp;
}

/**
 * \brief           Close MQTT connection
 * \param[in]       client: MQTT API client handle
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_mqtt_client_api_close(lwcell_mqtt_client_api_p client) {
    lwcellr_t res = lwcellERR;

    LWCELL_ASSERT(client != NULL);

    lwcell_sys_mutex_lock(&client->mutex);
    lwcell_sys_sem_wait(&client->sync_sem, 0);
    client->release_sem = 1;
    if (lwcell_mqtt_client_disconnect(client->mc) == lwcellOK) {
        res = lwcellOK;
        lwcell_sys_sem_wait(&client->sync_sem, 0);
    } else {
        LWCELL_DEBUGF(LWCELL_CFG_DBG_MQTT_API_TRACE_WARNING, "[MQTT API] Cannot close API connection\r\n");
    }
    client->release_sem = 0;
    lwcell_sys_sem_release(&client->sync_sem);
    lwcell_sys_mutex_unlock(&client->mutex);
    return res;
}

/**
 * \brief           Subscribe to topic
 * \param[in]       client: MQTT API client handle
 * \param[in]       topic: Topic to subscribe on
 * \param[in]       qos: Quality of service. This parameter can be a value of \ref lwcell_mqtt_qos_t
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_mqtt_client_api_subscribe(lwcell_mqtt_client_api_p client, const char* topic, lwcell_mqtt_qos_t qos) {
    lwcellr_t res = lwcellERR;

    LWCELL_ASSERT(client != NULL);
    LWCELL_ASSERT(topic != NULL);

    lwcell_sys_mutex_lock(&client->mutex);
    lwcell_sys_sem_wait(&client->sync_sem, 0);
    client->release_sem = 1;
    if (lwcell_mqtt_client_subscribe(client->mc, topic, qos, NULL) == lwcellOK) {
        lwcell_sys_sem_wait(&client->sync_sem, 0);
        res = client->sub_pub_resp;
    } else {
        LWCELL_DEBUGF(LWCELL_CFG_DBG_MQTT_API_TRACE_WARNING, "[MQTT API] Cannot subscribe to topic %s\r\n", topic);
    }
    client->release_sem = 0;
    lwcell_sys_sem_release(&client->sync_sem);
    lwcell_sys_mutex_unlock(&client->mutex);

    return res;
}

/**
 * \brief           Unsubscribe from topic
 * \param[in]       client: MQTT API client handle
 * \param[in]       topic: Topic to unsubscribe from
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_mqtt_client_api_unsubscribe(lwcell_mqtt_client_api_p client, const char* topic) {
    lwcellr_t res = lwcellERR;

    LWCELL_ASSERT(client != NULL);
    LWCELL_ASSERT(topic != NULL);

    lwcell_sys_mutex_lock(&client->mutex);
    lwcell_sys_sem_wait(&client->sync_sem, 0);
    client->release_sem = 1;
    if (lwcell_mqtt_client_unsubscribe(client->mc, topic, NULL) == lwcellOK) {
        lwcell_sys_sem_wait(&client->sync_sem, 0);
        res = client->sub_pub_resp;
    } else {
        LWCELL_DEBUGF(LWCELL_CFG_DBG_MQTT_API_TRACE_WARNING, "[MQTT API] Cannot unsubscribe from topic %s\r\n", topic);
    }
    client->release_sem = 0;
    lwcell_sys_sem_release(&client->sync_sem);
    lwcell_sys_mutex_unlock(&client->mutex);

    return res;
}

/**
 * \brief           Publish new packet to MQTT network
 * \param[in]       client: MQTT API client handle
 * \param[in]       topic: Topic to publish on
 * \param[in]       data: Data to send
 * \param[in]       btw: Number of bytes to send for data parameter
 * \param[in]       qos: Quality of service. This parameter can be a value of \ref lwcell_mqtt_qos_t
 * \param[in]       retain: Set to `1` for retain flag, `0` otherwise
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_mqtt_client_api_publish(lwcell_mqtt_client_api_p client, const char* topic, const void* data, size_t btw,
                               lwcell_mqtt_qos_t qos, uint8_t retain) {
    lwcellr_t res = lwcellERR;

    LWCELL_ASSERT(client != NULL);
    LWCELL_ASSERT(topic != NULL);
    LWCELL_ASSERT(data != NULL);
    LWCELL_ASSERT(btw > 0);

    lwcell_sys_mutex_lock(&client->mutex);
    lwcell_sys_sem_wait(&client->sync_sem, 0);
    client->release_sem = 1;
    if (lwcell_mqtt_client_publish(client->mc, topic, data, LWCELL_U16(btw), qos, retain, NULL) == lwcellOK) {
        lwcell_sys_sem_wait(&client->sync_sem, 0);
        res = client->sub_pub_resp;
    } else {
        LWCELL_DEBUGF(LWCELL_CFG_DBG_MQTT_API_TRACE_WARNING, "[MQTT API] Cannot publish new packet\r\n");
    }
    client->release_sem = 0;
    lwcell_sys_sem_release(&client->sync_sem);
    lwcell_sys_mutex_unlock(&client->mutex);

    return res;
}

/**
 * \brief           Check if client MQTT connection is active
 * \param[in]       client: MQTT API client handle
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwcell_mqtt_client_api_is_connected(lwcell_mqtt_client_api_p client) {
    uint8_t ret;

    if (client == NULL) {
        return 0;
    }

    lwcell_sys_mutex_lock(&client->mutex);
    ret = lwcell_mqtt_client_is_connected(client->mc);
    lwcell_sys_mutex_unlock(&client->mutex);
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
 * \return          \ref lwcellOK on success, \ref lwcellCLOSED if MQTT is closed, \ref lwcellTIMEOUT on timeout
 */
lwcellr_t
lwcell_mqtt_client_api_receive(lwcell_mqtt_client_api_p client, lwcell_mqtt_client_api_buf_p* p, uint32_t timeout) {
    LWCELL_ASSERT(client != NULL);
    LWCELL_ASSERT(p != NULL);

    *p = NULL;

    /* Get new entry from mbox */
    if (timeout == 0) {
        if (!lwcell_sys_mbox_getnow(&client->rcv_mbox, (void**)p)) {
            return lwcellTIMEOUT;
        }
    } else if (lwcell_sys_mbox_get(&client->rcv_mbox, (void**)p, timeout) == LWCELL_SYS_TIMEOUT) {
        return lwcellTIMEOUT;
    }

    /* Check for MQTT closed event */
    if ((uint8_t*)(*p) == (uint8_t*)&mqtt_closed) {
        LWCELL_DEBUGF(LWCELL_CFG_DBG_MQTT_API_TRACE, "[MQTT API] Closed event received from queue\r\n");

        *p = NULL;
        return lwcellCLOSED;
    }
    return lwcellOK;
}

/**
 * \brief           Free buffer memory after usage
 * \param[in]       p: Buffer to free
 */
void
lwcell_mqtt_client_api_buf_free(lwcell_mqtt_client_api_buf_p p) {
    lwcell_mem_free_s((void**)&p);
}
