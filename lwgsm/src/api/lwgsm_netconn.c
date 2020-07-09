/**
 * \file            lwgsm_netconn.c
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
 * This file is part of GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         $_version_$
 */
#include "lwgsm/lwgsm_netconn.h"
#include "lwgsm/lwgsm_private.h"
#include "lwgsm/lwgsm_conn.h"
#include "lwgsm/lwgsm_mem.h"

#if GSM_CFG_NETCONN || __DOXYGEN__

/* Check conditions */
#if !GSM_CFG_CONN
#error "GSM_CFG_CONN must be enabled for NETCONN API!"
#endif /* !GSM_CFG_CONN */

#if GSM_CFG_NETCONN_RECEIVE_QUEUE_LEN < 2
#error "GSM_CFG_NETCONN_RECEIVE_QUEUE_LEN must be greater or equal to 2"
#endif /* GSM_CFG_NETCONN_RECEIVE_QUEUE_LEN < 2 */

/**
 * \brief           Sequential API structure
 */
typedef struct lwgsm_netconn {
    struct lwgsm_netconn* next;                   /*!< Linked list entry */

    lwgsm_netconn_type_t type;                    /*!< Netconn type */

    size_t rcv_packets;                         /*!< Number of received packets so far on this connection */
    lwgsm_conn_p conn;                            /*!< Pointer to actual connection */

    lwgsm_sys_mbox_t mbox_receive;                /*!< Message queue for receive mbox */

    lwgsm_linbuff_t buff;                         /*!< Linear buffer structure */

    uint16_t conn_timeout;                      /*!< Connection timeout in units of seconds when
                                                    netconn is in server (listen) mode.
                                                    Connection will be automatically closed if there is no
                                                    data exchange in time. Set to `0` when timeout feature is disabled. */

#if GSM_CFG_NETCONN_RECEIVE_TIMEOUT || __DOXYGEN__
    uint32_t rcv_timeout;                       /*!< Receive timeout in unit of milliseconds */
#endif
} lwgsm_netconn_t;

static uint8_t recv_closed = 0xFF;
static lwgsm_netconn_t* netconn_list;             /*!< Linked list of netconn entries */

/**
 * \brief           Flush all mboxes and clear possible used memories
 * \param[in]       nc: Pointer to netconn to flush
 * \param[in]       protect: Set to 1 to protect against multi-thread access
 */
static void
flush_mboxes(lwgsm_netconn_t* nc, uint8_t protect) {
    lwgsm_pbuf_p pbuf;
    if (protect) {
        lwgsm_core_lock();
    }
    if (lwgsm_sys_mbox_isvalid(&nc->mbox_receive)) {
        while (lwgsm_sys_mbox_getnow(&nc->mbox_receive, (void**)&pbuf)) {
            if (pbuf != NULL && (uint8_t*)pbuf != (uint8_t*)&recv_closed) {
                lwgsm_pbuf_free(pbuf);            /* Free received data buffers */
            }
        }
        lwgsm_sys_mbox_delete(&nc->mbox_receive); /* Delete message queue */
        lwgsm_sys_mbox_invalid(&nc->mbox_receive);/* Invalid handle */
    }
    if (protect) {
        lwgsm_core_unlock();
    }
}

/**
 * \brief           Callback function for every server connection
 * \param[in]       evt: Pointer to callback structure
 * \return          Member of \ref lwgsmr_t enumeration
 */
static lwgsmr_t
netconn_evt(lwgsm_evt_t* evt) {
    lwgsm_conn_p conn;
    lwgsm_netconn_t* nc = NULL;
    uint8_t close = 0;

    conn = lwgsm_conn_get_from_evt(evt);          /* Get connection from event */
    switch (lwgsm_evt_get_type(evt)) {
        /*
         * A new connection has been active
         * and should be handled by netconn API
         */
        case GSM_EVT_CONN_ACTIVE: {             /* A new connection active is active */
            if (lwgsm_conn_is_client(conn)) {     /* Was connection started by us? */
                nc = lwgsm_conn_get_arg(conn);    /* Argument should be already set */
                if (nc != NULL) {
                    nc->conn = conn;            /* Save actual connection */
                } else {
                    close = 1;                  /* Close this connection, invalid netconn */
                }
            } else {
                GSM_DEBUGF(GSM_CFG_DBG_NETCONN | GSM_DBG_TYPE_TRACE | GSM_DBG_LVL_WARNING,
                           "[NETCONN] Closing connection, it is not in client mode!\r\n");
                close = 1;                      /* Close the connection at this point */
            }

            /* Decide if some events want to close the connection */
            if (close) {
                if (nc != NULL) {
                    lwgsm_conn_set_arg(conn, NULL);   /* Reset argument */
                    lwgsm_netconn_delete(nc);     /* Free memory for API */
                }
                lwgsm_conn_close(conn, 0);        /* Close the connection */
                close = 0;
            }
            break;
        }

        /*
         * We have a new data received which
         * should have netconn structure as argument
         */
        case GSM_EVT_CONN_RECV: {
            lwgsm_pbuf_p pbuf;

            nc = lwgsm_conn_get_arg(conn);        /* Get API from connection */
            pbuf = lwgsm_evt_conn_recv_get_buff(evt);/* Get received buff */

            lwgsm_conn_recved(conn, pbuf);        /* Notify stack about received data */

            lwgsm_pbuf_ref(pbuf);                 /* Increase reference counter */
            if (nc == NULL || !lwgsm_sys_mbox_isvalid(&nc->mbox_receive)
                || !lwgsm_sys_mbox_putnow(&nc->mbox_receive, pbuf)) {
                GSM_DEBUGF(GSM_CFG_DBG_NETCONN,
                           "[NETCONN] Ignoring more data for receive!\r\n");
                lwgsm_pbuf_free(pbuf);            /* Free pbuf */
                return gsmOKIGNOREMORE;         /* Return OK to free the memory and ignore further data */
            }
            ++nc->rcv_packets;                  /* Increase number of received packets */
            GSM_DEBUGF(GSM_CFG_DBG_NETCONN | GSM_DBG_TYPE_TRACE,
                       "[NETCONN] Received pbuf contains %d bytes. Handle written to receive mbox\r\n",
                       (int)lwgsm_pbuf_length(pbuf, 0));
            break;
        }

        /* Connection was just closed */
        case GSM_EVT_CONN_CLOSE: {
            nc = lwgsm_conn_get_arg(conn);        /* Get API from connection */

            /*
             * In case we have a netconn available,
             * simply write pointer to received variable to indicate closed state
             */
            if (nc != NULL && lwgsm_sys_mbox_isvalid(&nc->mbox_receive)) {
                lwgsm_sys_mbox_putnow(&nc->mbox_receive, (void*)&recv_closed);
            }

            break;
        }
        default:
            return gsmERR;
    }
    return gsmOK;
}

/**
 * \brief           Global event callback function
 * \param[in]       evt: Callback information and data
 * \return          \ref gsmOK on success, member of \ref lwgsmr_t otherwise
 */
static lwgsmr_t
lwgsm_evt(lwgsm_evt_t* evt) {
    switch (lwgsm_evt_get_type(evt)) {
        default:
            break;
    }
    return gsmOK;
}

/**
 * \brief           Create new netconn connection
 * \param[in]       type: Netconn connection type
 * \return          New netconn connection on success, `NULL` otherwise
 */
lwgsm_netconn_p
lwgsm_netconn_new(lwgsm_netconn_type_t type) {
    lwgsm_netconn_t* a;
    static uint8_t first = 1;

    /* Register only once! */
    lwgsm_core_lock();
    if (first) {
        first = 0;
        lwgsm_evt_register(lwgsm_evt);              /* Register global event function */
    }
    lwgsm_core_unlock();
    a = lwgsm_mem_calloc(1, sizeof(*a));          /* Allocate memory for core object */
    if (a != NULL) {
        a->type = type;                         /* Save netconn type */
        a->conn_timeout = 0;                    /* Default connection timeout */
        if (!lwgsm_sys_mbox_create(&a->mbox_receive, GSM_CFG_NETCONN_RECEIVE_QUEUE_LEN)) {    /* Allocate memory for receiving message box */
            GSM_DEBUGF(GSM_CFG_DBG_NETCONN | GSM_DBG_TYPE_TRACE | GSM_DBG_LVL_DANGER,
                       "[NETCONN] Cannot create receive MBOX\r\n");
            goto free_ret;
        }
        lwgsm_core_lock();
        if (netconn_list == NULL) {             /* Add new netconn to the existing list */
            netconn_list = a;
        } else {
            a->next = netconn_list;             /* Add it to beginning of the list */
            netconn_list = a;
        }
        lwgsm_core_unlock();
    }
    return a;
free_ret:
    if (lwgsm_sys_mbox_isvalid(&a->mbox_receive)) {
        lwgsm_sys_mbox_delete(&a->mbox_receive);
        lwgsm_sys_mbox_invalid(&a->mbox_receive);
    }
    if (a != NULL) {
        lwgsm_mem_free_s((void**)&a);
    }
    return NULL;
}

/**
 * \brief           Delete netconn connection
 * \param[in]       nc: Netconn handle
 * \return          \ref gsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_netconn_delete(lwgsm_netconn_p nc) {
    GSM_ASSERT("netconn != NULL", nc != NULL);

    lwgsm_core_lock();
    flush_mboxes(nc, 0);                        /* Clear mboxes */

    /* Remove netconn from linkedlist */
    if (netconn_list == nc) {
        netconn_list = netconn_list->next;      /* Remove first from linked list */
    } else if (netconn_list != NULL) {
        lwgsm_netconn_p tmp, prev;
        /* Find element on the list */
        for (prev = netconn_list, tmp = netconn_list->next;
             tmp != NULL; prev = tmp, tmp = tmp->next) {
            if (nc == tmp) {
                prev->next = tmp->next;         /* Remove tmp from linked list */
                break;
            }
        }
    }
    lwgsm_core_unlock();

    lwgsm_mem_free_s((void**)&nc);
    return gsmOK;
}

/**
 * \brief           Connect to server as client
 * \param[in]       nc: Netconn handle
 * \param[in]       host: Pointer to host, such as domain name or IP address in string format
 * \param[in]       port: Target port to use
 * \return          \ref gsmOK if successfully connected, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_netconn_connect(lwgsm_netconn_p nc, const char* host, lwgsm_port_t port) {
    lwgsmr_t res;

    GSM_ASSERT("nc != NULL", nc != NULL);
    GSM_ASSERT("host != NULL", host != NULL);
    GSM_ASSERT("port > 0", port > 0);

    /*
     * Start a new connection as client and:
     *
     *  - Set current netconn structure as argument
     *  - Set netconn callback function for connection management
     *  - Start connection in blocking mode
     */
    res = lwgsm_conn_start(NULL, (lwgsm_conn_type_t)nc->type, host, port, nc, netconn_evt, 1);
    return res;
}

/**
 * \brief           Write data to connection output buffers
 * \note            This function may only be used on TCP or SSL connections
 * \param[in]       nc: Netconn handle used to write data to
 * \param[in]       data: Pointer to data to write
 * \param[in]       btw: Number of bytes to write
 * \return          \ref gsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_netconn_write(lwgsm_netconn_p nc, const void* data, size_t btw) {
    size_t len, sent;
    const uint8_t* d = data;
    lwgsmr_t res;

    GSM_ASSERT("nc != NULL", nc != NULL);
    GSM_ASSERT("nc->type must be TCP or SSL", nc->type == GSM_NETCONN_TYPE_TCP || nc->type == GSM_NETCONN_TYPE_SSL);
    GSM_ASSERT("nc->conn must be active", lwgsm_conn_is_active(nc->conn));

    /*
     * Several steps are done in write process
     *
     * 1. Check if buffer is set and check if there is something to write to it.
     *    1. In case buffer will be full after copy, send it and free memory.
     * 2. Check how many bytes we can write directly without need to copy
     * 3. Try to allocate a new buffer and copy remaining input data to it
     * 4. In case buffer allocation fails, send data directly (may affect on speed and effectivenes)
     */

    /* Step 1 */
    if (nc->buff.buff != NULL) {                /* Is there a write buffer ready to accept more data? */
        len = GSM_MIN(nc->buff.len - nc->buff.ptr, btw);    /* Get number of bytes we can write to buffer */
        if (len > 0) {
            GSM_MEMCPY(&nc->buff.buff[nc->buff.ptr], data, len);/* Copy memory to temporary write buffer */
            d += len;
            nc->buff.ptr += len;
            btw -= len;
        }

        /* Step 1.1 */
        if (nc->buff.ptr == nc->buff.len) {
            res = lwgsm_conn_send(nc->conn, nc->buff.buff, nc->buff.len, &sent, 1);

            lwgsm_mem_free_s((void**)&nc->buff.buff);
            if (res != gsmOK) {
                return res;
            }
        } else {
            return gsmOK;                       /* Buffer is not yet full yet */
        }
    }

    /* Step 2 */
    if (btw >= GSM_CFG_CONN_MAX_DATA_LEN) {
        size_t rem;
        rem = btw % GSM_CFG_CONN_MAX_DATA_LEN;  /* Get remaining bytes for max data length */
        res = lwgsm_conn_send(nc->conn, d, btw - rem, &sent, 1);  /* Write data directly */
        if (res != gsmOK) {
            return res;
        }
        d += sent;                              /* Advance in data pointer */
        btw -= sent;                            /* Decrease remaining data to send */
    }

    if (btw == 0) {                             /* Sent everything? */
        return gsmOK;
    }

    /* Step 3 */
    if (nc->buff.buff == NULL) {                /* Check if we should allocate a new buffer */
        nc->buff.buff = lwgsm_mem_malloc(sizeof(*nc->buff.buff) * GSM_CFG_CONN_MAX_DATA_LEN);
        nc->buff.len = GSM_CFG_CONN_MAX_DATA_LEN;   /* Save buffer length */
        nc->buff.ptr = 0;                       /* Save buffer pointer */
    }

    /* Step 4 */
    if (nc->buff.buff != NULL) {                /* Memory available? */
        GSM_MEMCPY(&nc->buff.buff[nc->buff.ptr], d, btw);   /* Copy data to buffer */
        nc->buff.ptr += btw;
    } else {                                    /* Still no memory available? */
        return lwgsm_conn_send(nc->conn, data, btw, NULL, 1); /* Simply send directly blocking */
    }
    return gsmOK;
}

/**
 * \brief           Flush buffered data on netconn \e TCP/SSL connection
 * \note            This function may only be used on \e TCP/SSL connection
 * \param[in]       nc: Netconn handle to flush data
 * \return          \ref gsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_netconn_flush(lwgsm_netconn_p nc) {
    GSM_ASSERT("nc != NULL", nc != NULL);
    GSM_ASSERT("nc->type must be TCP or SSL", nc->type == GSM_NETCONN_TYPE_TCP || nc->type == GSM_NETCONN_TYPE_SSL);
    GSM_ASSERT("nc->conn must be active", lwgsm_conn_is_active(nc->conn));

    /*
     * In case we have data in write buffer,
     * flush them out to network
     */
    if (nc->buff.buff != NULL) {                /* Check remaining data */
        if (nc->buff.ptr > 0) {                 /* Do we have data in current buffer? */
            lwgsm_conn_send(nc->conn, nc->buff.buff, nc->buff.ptr, NULL, 1);  /* Send data */
        }
        lwgsm_mem_free_s((void**)&nc->buff.buff);
    }
    return gsmOK;
}

/**
 * \brief           Send data on \e UDP connection to default IP and port
 * \param[in]       nc: Netconn handle used to send
 * \param[in]       data: Pointer to data to write
 * \param[in]       btw: Number of bytes to write
 * \return          \ref gsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_netconn_send(lwgsm_netconn_p nc, const void* data, size_t btw) {
    GSM_ASSERT("nc != NULL", nc != NULL);
    GSM_ASSERT("nc->type must be UDP", nc->type == GSM_NETCONN_TYPE_UDP);
    GSM_ASSERT("nc->conn must be active", lwgsm_conn_is_active(nc->conn));

    return lwgsm_conn_send(nc->conn, data, btw, NULL, 1);
}

/**
 * \brief           Send data on \e UDP connection to specific IP and port
 * \note            Use this function in case of UDP type netconn
 * \param[in]       nc: Netconn handle used to send
 * \param[in]       ip: Pointer to IP address
 * \param[in]       port: Port number used to send data
 * \param[in]       data: Pointer to data to write
 * \param[in]       btw: Number of bytes to write
 * \return          \ref gsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_netconn_sendto(lwgsm_netconn_p nc, const lwgsm_ip_t* ip, lwgsm_port_t port, const void* data, size_t btw) {
    GSM_ASSERT("nc != NULL", nc != NULL);
    GSM_ASSERT("nc->type must be UDP", nc->type == GSM_NETCONN_TYPE_UDP);
    GSM_ASSERT("nc->conn must be active", lwgsm_conn_is_active(nc->conn));

    return lwgsm_conn_sendto(nc->conn, ip, port, data, btw, NULL, 1);
}

/**
 * \brief           Receive data from connection
 * \param[in]       nc: Netconn handle used to receive from
 * \param[in]       pbuf: Pointer to pointer to save new receive buffer to.
 *                     When function returns, user must check for valid pbuf value `pbuf != NULL`
 * \return          \ref gsmOK when new data ready,
 * \return          \ref gsmCLOSED when connection closed by remote side,
 * \return          \ref gsmTIMEOUT when receive timeout occurs
 * \return          Any other member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_netconn_receive(lwgsm_netconn_p nc, lwgsm_pbuf_p* pbuf) {
    GSM_ASSERT("nc != NULL", nc != NULL);
    GSM_ASSERT("pbuf != NULL", pbuf != NULL);

    *pbuf = NULL;
#if GSM_CFG_NETCONN_RECEIVE_TIMEOUT
    /*
     * Wait for new received data for up to specific timeout
     * or throw error for timeout notification
     */
    if (lwgsm_sys_mbox_get(&nc->mbox_receive, (void**)pbuf, nc->rcv_timeout) == GSM_SYS_TIMEOUT) {
        return gsmTIMEOUT;
    }
#else /* GSM_CFG_NETCONN_RECEIVE_TIMEOUT */
    /* Forever wait for new receive packet */
    lwgsm_sys_mbox_get(&nc->mbox_receive, (void**)pbuf, 0);
#endif /* !GSM_CFG_NETCONN_RECEIVE_TIMEOUT */

    /* Check if connection closed */
    if ((uint8_t*)(*pbuf) == (uint8_t*)&recv_closed) {
        *pbuf = NULL;                           /* Reset pbuf */
        return gsmCLOSED;
    }
    return gsmOK;                               /* We have data available */
}

/**
 * \brief           Close a netconn connection
 * \param[in]       nc: Netconn handle to close
 * \return          \ref gsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_netconn_close(lwgsm_netconn_p nc) {
    lwgsm_conn_p conn;

    GSM_ASSERT("nc != NULL", nc != NULL);
    GSM_ASSERT("nc->conn != NULL", nc->conn != NULL);
    GSM_ASSERT("nc->conn must be active", lwgsm_conn_is_active(nc->conn));

    lwgsm_netconn_flush(nc);                      /* Flush data and ignore result */
    conn = nc->conn;
    nc->conn = NULL;

    lwgsm_conn_set_arg(conn, NULL);               /* Reset argument */
    lwgsm_conn_close(conn, 1);                    /* Close the connection */
    flush_mboxes(nc, 1);                        /* Flush message queues */
    return gsmOK;
}

/**
 * \brief           Get connection number used for netconn
 * \param[in]       nc: Netconn handle
 * \return          `-1` on failure, connection number between `0` and \ref GSM_CFG_MAX_CONNS otherwise
 */
int8_t
lwgsm_netconn_getconnnum(lwgsm_netconn_p nc) {
    if (nc != NULL && nc->conn != NULL) {
        return lwgsm_conn_getnum(nc->conn);
    }
    return -1;
}

#if GSM_CFG_NETCONN_RECEIVE_TIMEOUT || __DOXYGEN__

/**
 * \brief           Set timeout value for receiving data.
 *
 * When enabled, \ref lwgsm_netconn_receive will only block for up to
 * \e timeout value and will return if no new data within this time
 *
 * \param[in]       nc: Netconn handle
 * \param[in]       timeout: Timeout in units of milliseconds.
 *                  Set to `0` to disable timeout for \ref lwgsm_netconn_receive function
 */
void
lwgsm_netconn_set_receive_timeout(lwgsm_netconn_p nc, uint32_t timeout) {
    nc->rcv_timeout = timeout;
}

/**
 * \brief           Get netconn receive timeout value
 * \param[in]       nc: Netconn handle
 * \return          Timeout in units of milliseconds.
 *                  If value is `0`, timeout is disabled (wait forever)
 */
uint32_t
lwgsm_netconn_get_receive_timeout(lwgsm_netconn_p nc) {
    return nc->rcv_timeout;                     /* Return receive timeout */
}

#endif /* GSM_CFG_NETCONN_RECEIVE_TIMEOUT || __DOXYGEN__ */

#endif /* GSM_CFG_NETCONN || __DOXYGEN__ */
