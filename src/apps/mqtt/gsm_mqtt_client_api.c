/**
 * \file            gsm_mqtt_client_api.c
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
#include "gsm/apps/gsm_mqtt_client_api.h"
#include "gsm/gsm_mem.h"

/* Tracing debug message */
#define GSM_CFG_DBG_MQTT_API_TRACE              (GSM_CFG_DBG_MQTT_API | GSM_DBG_TYPE_TRACE)
#define GSM_CFG_DBG_MQTT_API_STATE              (GSM_CFG_DBG_MQTT_API | GSM_DBG_TYPE_STATE)
#define GSM_CFG_DBG_MQTT_API_TRACE_WARNING      (GSM_CFG_DBG_MQTT_API | GSM_DBG_TYPE_TRACE | GSM_DBG_LVL_WARNING)

/**
 * \brief           MQTT API client structure
 */
struct gsm_mqtt_client_api {
    gsm_mqtt_client_p mc;                       /*!< MQTT client handle */
    gsm_sys_mbox_t rcv_mbox;                    /*!< Received data mbox */
    gsm_sys_sem_t sync_sem;                     /*!< Synchronization semaphore */
    gsm_sys_mutex_t mutex;                      /*!< Mutex handle */
    uint8_t release_sem;                        /*!< Set to `1` to release semaphore */
    gsm_mqtt_conn_status_t connect_rgsm;        /*!< Rgsmonse when connecting to server */
    gsmr_t sub_pub_rgsm;                        /*!< Subscribe/Unsubscribe/Publish rgsmonse */
} gsm_mqtt_client_api_t;

static uint8_t mqtt_closed = 0xFF;

/**
 * \brief           Release user semaphore
 * \param[in]       client: Client handle
 */
static void
release_sem(gsm_mqtt_client_api_p client) {
    if (client->release_sem) {
        client->release_sem = 0;
        gsm_sys_sem_release(&client->sync_sem);
    }
}

/**
 * \brief           MQTT event callback function
 */
static void
mqtt_evt(gsm_mqtt_client_p client, gsm_mqtt_evt_t* evt) {
    gsm_mqtt_client_api_p api_client = gsm_mqtt_client_get_arg(client);
    if (api_client == NULL) {
        return;
    }
    switch (gsm_mqtt_client_evt_get_type(client, evt)) {
        case GSM_MQTT_EVT_CONNECT: {
            gsm_mqtt_conn_status_t status = gsm_mqtt_client_evt_connect_get_status(client, evt);

            GSM_DEBUGF(GSM_CFG_DBG_MQTT_API_TRACE,
                "[MQTT API] Connect event with status: %d\r\n", (int)status);

            api_client->connect_rgsm = status;
            
            /*
             * By MQTT 3.1.1 specification, broker must close connection
             * if client CONNECT packet was not accepted.
             *
             * If client is accepted or connection did not even start,
             * release semaphore, otherwise wait CLOSED event 
             * and release semaphore from there,
             * to make sure we are fully ready for next connection
             */
            if (status == GSM_MQTT_CONN_STATUS_TCP_FAILED
                || status == GSM_MQTT_CONN_STATUS_ACCEPTED) {
                release_sem(api_client);        /* Release semaphore */
            }

            break;
        }
        case GSM_MQTT_EVT_PUBLISH_RECV: {
            /* Check valid receive mbox */
            if (gsm_sys_mbox_isvalid(&api_client->rcv_mbox)) {
                gsm_mqtt_client_api_buf_p buf;
                size_t size, buf_size, topic_size, payload_size;

                /* Get event data */
                const char* topic = gsm_mqtt_client_evt_publish_recv_get_topic(client, evt);
                size_t topic_len = gsm_mqtt_client_evt_publish_recv_get_topic_len(client, evt);
                const uint8_t* payload = gsm_mqtt_client_evt_publish_recv_get_payload(client, evt);
                size_t payload_len = gsm_mqtt_client_evt_publish_recv_get_payload_len(client, evt);
                gsm_mqtt_qos_t qos = gsm_mqtt_client_evt_publish_recv_get_qos(client, evt);

                /* Print debug message */
                GSM_DEBUGF(GSM_CFG_DBG_MQTT_API_TRACE,
                    "[MQTT API] New publish received on topic %.*s\r\n", (int)topic_len, topic);

                /* Calculate sizes */
                buf_size = sizeof(*buf);
                topic_size = sizeof(*topic) * (topic_len + 1);
                payload_size = sizeof(*payload) * (payload_len + 1);

                size = sizeof(*buf) + topic_size + payload_size;
                buf = gsm_mem_alloc(size);
                if (buf != NULL) {
                    GSM_MEMSET(buf, 0x00, size);
                    buf->topic = (const void *)(buf + buf_size);
                    buf->payload = (const void *)(buf->topic + topic_size);
                    buf->topic_len = topic_len;
                    buf->payload_len = payload_len;
                    buf->qos = qos;

                    /* Copy content to new memory */
                    GSM_MEMCPY((void *)buf->topic, topic, sizeof(*topic) * topic_len);
                    GSM_MEMCPY((void *)buf->payload, payload, sizeof(*payload) * payload_len);

                    /* Write to receive queue */
                    if (!gsm_sys_mbox_putnow(&api_client->rcv_mbox, buf)) {
                        gsm_mem_free(buf);
                    }
                } else {
                    GSM_DEBUGF(GSM_CFG_DBG_MQTT_API_TRACE_WARNING,
                        "[MQTT API] Cannot allocate memory for packet buffer of size %d bytes\r\n",
                        (int)size);
                }
            }
            break;
        }
        case GSM_MQTT_EVT_PUBLISH: {
            api_client->sub_pub_rgsm = gsm_mqtt_client_evt_publish_get_result(client, evt);

            /* Print debug message */
            GSM_DEBUGF(GSM_CFG_DBG_MQTT_API_TRACE,
                "[MQTT API] Publish event with rgsmonse: %d\r\n",
                (int)api_client->sub_pub_rgsm);

            release_sem(api_client);            /* Release semaphore */
            break;
        }
        case GSM_MQTT_EVT_SUBSCRIBE: {
            api_client->sub_pub_rgsm = gsm_mqtt_client_evt_subscribe_get_result(client, evt);

            /* Print debug message */
            GSM_DEBUGF(GSM_CFG_DBG_MQTT_API_TRACE,
                "[MQTT API] Subscribe event with rgsmonse: %d\r\n",
                (int)api_client->sub_pub_rgsm);

            release_sem(api_client);            /* Release semaphore */
            break;
        }
        case GSM_MQTT_EVT_UNSUBSCRIBE: {
            api_client->sub_pub_rgsm = gsm_mqtt_client_evt_unsubscribe_get_result(client, evt);

            /* Print debug message */
            GSM_DEBUGF(GSM_CFG_DBG_MQTT_API_TRACE,
                "[MQTT API] Unsubscribe event with rgsmonse: %d\r\n",
                (int)api_client->sub_pub_rgsm);

            release_sem(api_client);            /* Release semaphore */
            break;
        }
        case GSM_MQTT_EVT_DISCONNECT: {
            uint8_t is_accepted = gsm_mqtt_client_evt_disconnect_is_accepted(client, evt);
            /* Disconnect event happened */
            //api_client->connect_rgsm = MQTT_CONN_STATUS_TCP_FAILED;

            /* Print debug message */
            GSM_DEBUGF(GSM_CFG_DBG_MQTT_API_TRACE,
                "[MQTT API] Disconnect event\r\n");

            /* Write to receive mbox to wakeup receive thread */
            if (is_accepted && gsm_sys_mbox_isvalid(&api_client->rcv_mbox)) {
                gsm_sys_mbox_putnow(&api_client->rcv_mbox, &mqtt_closed);
            }

            release_sem(api_client);            /* Release semaphore */
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
gsm_mqtt_client_api_p
gsm_mqtt_client_api_new(size_t tx_buff_len, size_t rx_buff_len) {
    gsm_mqtt_client_api_p client;
    size_t size;

    size = GSM_MEM_ALIGN(sizeof(*client));      /* Get size of client itself */

    /* Create client APi structure */
    client = gsm_mem_calloc(1, size);           /* Allocate client memory */
    if (client != NULL) {
        /* Create MQTT raw client structure */
        client->mc = gsm_mqtt_client_new(tx_buff_len, rx_buff_len);
        if (client->mc != NULL) {
            /* Create receive mbox queue */
            if (gsm_sys_mbox_create(&client->rcv_mbox, 5)) {
                /* Create synchronization semaphore */
                if (gsm_sys_sem_create(&client->sync_sem, 5)) {
                    /* Create mutex */
                    if (gsm_sys_mutex_create(&client->mutex)) {
                        gsm_mqtt_client_set_arg(client->mc, client);/* Set client to mqtt client argument */
                        return client;
                    } else {
                        GSM_DEBUGF(GSM_CFG_DBG_MQTT_API,
                            "[MQTT API] Cannot allocate mutex\r\n");
                    }
                } else {
                    GSM_DEBUGF(GSM_CFG_DBG_MQTT_API,
                        "[MQTT API] Cannot allocate sync semaphore\r\n");
                }
            } else {
                GSM_DEBUGF(GSM_CFG_DBG_MQTT_API,
                    "[MQTT API] Cannot allocate receive queue\r\n");
            }
        } else {
            GSM_DEBUGF(GSM_CFG_DBG_MQTT_API,
                "[MQTT API] Cannot allocate MQTT client\r\n");
        }
    } else {
        GSM_DEBUGF(GSM_CFG_DBG_MQTT_API,
            "[MQTT API] Cannot allocate memory for client\r\n");
    }

    gsm_mqtt_client_api_delete(client);
    client = NULL;
    return NULL;
}

/**
 * \brief           Delete client from memory
 * \param[in]       client: MQTT API client handle
 */
void
gsm_mqtt_client_api_delete(gsm_mqtt_client_api_p client) {
    if (client == NULL) {
        return;
    }
    if (gsm_sys_sem_isvalid(&client->sync_sem)) {
        gsm_sys_sem_delete(&client->sync_sem);
        gsm_sys_sem_invalid(&client->sync_sem);
    }
    if (gsm_sys_mutex_isvalid(&client->mutex)) {
        gsm_sys_mutex_delete(&client->mutex);
        gsm_sys_mutex_invalid(&client->mutex);
    }
    if (gsm_sys_mbox_isvalid(&client->rcv_mbox)) {
        void* d;
        while (gsm_sys_mbox_getnow(&client->rcv_mbox, &d)) {
            if ((uint8_t *)d != (uint8_t *)&mqtt_closed) {
                gsm_mqtt_client_api_buf_free(d);
            }
        }
        gsm_sys_mbox_delete(&client->rcv_mbox);
        gsm_sys_mbox_invalid(&client->rcv_mbox);
    }
    if (client->mc != NULL) {
        gsm_mqtt_client_delete(client->mc);
        client->mc = NULL;
    }
    gsm_mem_free(client);
}

/**
 * \brief           Connect to MQTT broker
 * \param[in]       client: MQTT API client handle
 * \param[in]       host: TCP host
 * \param[in]       port: TCP port
 * \param[in]       info: MQTT client info
 * \return          \ref GSM_MQTT_CONN_STATUS_ACCEPTED on success, member of \ref gsm_mqtt_conn_status_t otherwise
 */
gsm_mqtt_conn_status_t
gsm_mqtt_client_api_connect(gsm_mqtt_client_api_p client, const char* host,
                            gsm_port_t port, const gsm_mqtt_client_info_t* info) {
    if (client == NULL || host == NULL
        || port == 0 || info == NULL) {
        GSM_DEBUGF(GSM_CFG_DBG_MQTT_API_TRACE_WARNING,
            "[MQTT API] Invalid parameters in function\r\n");
        return GSM_MQTT_CONN_STATUS_TCP_FAILED;
    }

    gsm_sys_mutex_lock(&client->mutex);
    client->connect_rgsm = GSM_MQTT_CONN_STATUS_TCP_FAILED;
    gsm_sys_sem_wait(&client->sync_sem, 0);
    client->release_sem = 1;
    if (gsm_mqtt_client_connect(client->mc, host, port, mqtt_evt, info) == gsmOK) {
        gsm_sys_sem_wait(&client->sync_sem, 0);
    } else {
        GSM_DEBUGF(GSM_CFG_DBG_MQTT_API_TRACE_WARNING,
            "[MQTT API] Cannot connect to %s\r\n", host);
    }
    client->release_sem = 0;
    gsm_sys_sem_release(&client->sync_sem);
    gsm_sys_mutex_unlock(&client->mutex);
    return client->connect_rgsm;
}

/**
 * \brief           Close MQTT connection
 * \param[in]       client: MQTT API client handle
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
gsmr_t
gsm_mqtt_client_api_close(gsm_mqtt_client_api_p client) {
    gsmr_t res = gsmERR;

    GSM_ASSERT("client != NULL", client != NULL);

    gsm_sys_mutex_lock(&client->mutex);
    gsm_sys_sem_wait(&client->sync_sem, 0);
    client->release_sem = 1;
    if (gsm_mqtt_client_disconnect(client->mc) == gsmOK) {
        res = gsmOK;
        gsm_sys_sem_wait(&client->sync_sem, 0);
    } else {
         GSM_DEBUGF(GSM_CFG_DBG_MQTT_API_TRACE_WARNING,
             "[MQTT API] Cannot close API connection\r\n");
    }
    client->release_sem = 0;
    gsm_sys_sem_release(&client->sync_sem);
    gsm_sys_mutex_unlock(&client->mutex);
    return res;
}

/**
 * \brief           Subscribe to topic
 * \param[in]       client: MQTT API client handle
 * \param[in]       topic: Topic to subscribe on
 * \param[in]       qos: Quality of service. This parameter can be a value of \ref gsm_mqtt_qos_t
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
gsmr_t
gsm_mqtt_client_api_subscribe(gsm_mqtt_client_api_p client, const char* topic,
                                gsm_mqtt_qos_t qos) {
    gsmr_t res = gsmERR;

    GSM_ASSERT("client != NULL", client != NULL);
    GSM_ASSERT("topic != NULL", topic != NULL);

    gsm_sys_mutex_lock(&client->mutex);
    gsm_sys_sem_wait(&client->sync_sem, 0);
    client->release_sem = 1;
    if (gsm_mqtt_client_subscribe(client->mc, topic, qos, NULL) == gsmOK) {
        gsm_sys_sem_wait(&client->sync_sem, 0);
        res = client->sub_pub_rgsm;
    } else {
        GSM_DEBUGF(GSM_CFG_DBG_MQTT_API_TRACE_WARNING,
            "[MQTT API] Cannot subscribe to topic %s\r\n", topic);
    }
    client->release_sem = 0;
    gsm_sys_sem_release(&client->sync_sem);
    gsm_sys_mutex_unlock(&client->mutex);

    return res;
}

/**
 * \brief           Unsubscribe from topic
 * \param[in]       client: MQTT API client handle
 * \param[in]       topic: Topic to unsubscribe from
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
gsmr_t
gsm_mqtt_client_api_unsubscribe(gsm_mqtt_client_api_p client, const char* topic) {
    gsmr_t res = gsmERR;

    GSM_ASSERT("client != NULL", client != NULL);
    GSM_ASSERT("topic != NULL", topic != NULL);

    gsm_sys_mutex_lock(&client->mutex);
    gsm_sys_sem_wait(&client->sync_sem, 0);
    client->release_sem = 1;
    if (gsm_mqtt_client_unsubscribe(client->mc, topic, NULL) == gsmOK) {
        gsm_sys_sem_wait(&client->sync_sem, 0);
        res = client->sub_pub_rgsm;
    } else {
        GSM_DEBUGF(GSM_CFG_DBG_MQTT_API_TRACE_WARNING,
            "[MQTT API] Cannot unsubscribe from topic %s\r\n", topic);
    }
    client->release_sem = 0;
    gsm_sys_sem_release(&client->sync_sem);
    gsm_sys_mutex_unlock(&client->mutex);

    return res;
}

/**
 * \brief           Publish new packet to MQTT network
 * \param[in]       client: MQTT API client handle
 * \param[in]       topic: Topic to publish on
 * \param[in]       data: Data to send
 * \param[in]       btw: Number of bytes to send for data parameter
 * \param[in]       qos: Quality of service. This parameter can be a value of \ref gsm_mqtt_qos_t
 * \param[in]       retain: Set to `1` for retain flag, `0` otherwise
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
gsmr_t
gsm_mqtt_client_api_publish(gsm_mqtt_client_api_p client, const char* topic, const void* data,
                            size_t btw, gsm_mqtt_qos_t qos, uint8_t retain) {
    gsmr_t res = gsmERR;

    GSM_ASSERT("client != NULL", client != NULL);
    GSM_ASSERT("topic != NULL", topic != NULL);
    GSM_ASSERT("data != NULL", data != NULL);
    GSM_ASSERT("btw > 0", btw > 0);

    gsm_sys_mutex_lock(&client->mutex);
    gsm_sys_sem_wait(&client->sync_sem, 0);
    client->release_sem = 1;
    if (gsm_mqtt_client_publish(client->mc, topic, data, GSM_U16(btw), qos, 1, NULL) == gsmOK) {
        gsm_sys_sem_wait(&client->sync_sem, 0);
        res = client->sub_pub_rgsm;
    } else {
        GSM_DEBUGF(GSM_CFG_DBG_MQTT_API_TRACE_WARNING,
            "[MQTT API] Cannot publish new packet\r\n");
    }
    client->release_sem = 0;
    gsm_sys_sem_release(&client->sync_sem);
    gsm_sys_mutex_unlock(&client->mutex);

    return res;
}

/**
 * \brief           Receive next packet in specific timeout time
 * \note            This function can be called from separate thread
 *                      than the rest of API function, which allows you to 
 *                      handle receive data separated with custom timeout
 * \param[in]       client: MQTT API client handle
 * \param[in]       p: Pointer to output buffer
 * \param[in]       timeout: Maximal time to wait before function returns timeout
 * \return          \ref gsmOK on success, \ref gsmCLOSED if MQTT is closed, \ref gsmTIMEOUT on timeout
 */
gsmr_t
gsm_mqtt_client_api_receive(gsm_mqtt_client_api_p client, gsm_mqtt_client_api_buf_p* p,
                            uint32_t timeout) {
    GSM_ASSERT("client != NULL", client != NULL);
    GSM_ASSERT("p != NULL", p != NULL);

    *p = NULL;

    /* Get new entry from mbox */
    if (timeout == 0) {
        if (!gsm_sys_mbox_getnow(&client->rcv_mbox, (void **)p)) {
            return gsmTIMEOUT;
        }
    } else if (gsm_sys_mbox_get(&client->rcv_mbox, (void **)p, timeout) == GSM_SYS_TIMEOUT) {
        return gsmTIMEOUT;
    }

    /* Check for MQTT closed event */
    if ((uint8_t *)(*p) == (uint8_t *)&mqtt_closed) {
        GSM_DEBUGF(GSM_CFG_DBG_MQTT_API_TRACE,
            "[MQTT API] Closed event received from queue\r\n");

        *p = NULL;
        return gsmCLOSED;
    }
    return gsmOK;
}

/**
 * \brief           Free buffer memory after usage
 * \param[in]       p: Buffer to free
 */
void
gsm_mqtt_client_api_buf_free(gsm_mqtt_client_api_buf_p p) {
    if (p != NULL) {
        gsm_mem_free(p);
    }
}
