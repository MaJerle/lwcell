/**
 * \file            lwcell_netconn.c
 * \brief           API functions for sequential calls
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
 * This file is part of LwCELL - Lightweight cellular modem AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.1
 */
#include "lwcell/lwcell_netconn.h"
#include "lwcell/lwcell_conn.h"
#include "lwcell/lwcell_mem.h"
#include "lwcell/lwcell_private.h"

#if LWCELL_CFG_NETCONN || __DOXYGEN__

/* Check conditions */
#if !LWCELL_CFG_CONN
#error "LWCELL_CFG_CONN must be enabled for NETCONN API!"
#endif /* !LWCELL_CFG_CONN */

#if LWCELL_CFG_NETCONN_RECEIVE_QUEUE_LEN < 2
#error "LWCELL_CFG_NETCONN_RECEIVE_QUEUE_LEN must be greater or equal to 2"
#endif /* LWCELL_CFG_NETCONN_RECEIVE_QUEUE_LEN < 2 */

/**
 * \brief           Sequential API structure
 */
typedef struct lwcell_netconn {
    struct lwcell_netconn* next;    /*!< Linked list entry */

    lwcell_netconn_type_t type;     /*!< Netconn type */

    size_t rcv_packets;            /*!< Number of received packets so far on this connection */
    lwcell_conn_p conn;             /*!< Pointer to actual connection */

    lwcell_sys_mbox_t mbox_receive; /*!< Message queue for receive mbox */

    lwcell_linbuff_t buff;          /*!< Linear buffer structure */

    uint16_t conn_timeout;         /*!< Connection timeout in units of seconds when
                                                    netconn is in server (listen) mode.
                                                    Connection will be automatically closed if there is no
                                                    data exchange in time. Set to `0` when timeout feature is disabled. */

#if LWCELL_CFG_NETCONN_RECEIVE_TIMEOUT || __DOXYGEN__
    uint32_t rcv_timeout; /*!< Receive timeout in unit of milliseconds */
#endif
} lwcell_netconn_t;

static uint8_t recv_closed = 0xFF;
static lwcell_netconn_t* netconn_list; /*!< Linked list of netconn entries */

/**
 * \brief           Flush all mboxes and clear possible used memories
 * \param[in]       nc: Pointer to netconn to flush
 * \param[in]       protect: Set to 1 to protect against multi-thread access
 */
static void
flush_mboxes(lwcell_netconn_t* nc, uint8_t protect) {
    lwcell_pbuf_p pbuf;
    if (protect) {
        lwcell_core_lock();
    }
    if (lwcell_sys_mbox_isvalid(&nc->mbox_receive)) {
        while (lwcell_sys_mbox_getnow(&nc->mbox_receive, (void**)&pbuf)) {
            if (pbuf != NULL && (uint8_t*)pbuf != (uint8_t*)&recv_closed) {
                lwcell_pbuf_free_s(&pbuf); /* Free received data buffers */
            }
        }
        lwcell_sys_mbox_delete(&nc->mbox_receive);  /* Delete message queue */
        lwcell_sys_mbox_invalid(&nc->mbox_receive); /* Invalid handle */
    }
    if (protect) {
        lwcell_core_unlock();
    }
}

/**
 * \brief           Callback function for every server connection
 * \param[in]       evt: Pointer to callback structure
 * \return          Member of \ref lwcellr_t enumeration
 */
static lwcellr_t
netconn_evt(lwcell_evt_t* evt) {
    lwcell_conn_p conn;
    lwcell_netconn_t* nc = NULL;
    uint8_t close = 0;

    conn = lwcell_conn_get_from_evt(evt); /* Get connection from event */
    switch (lwcell_evt_get_type(evt)) {
        /*
         * A new connection has been active
         * and should be handled by netconn API
         */
        case LWCELL_EVT_CONN_ACTIVE: {          /* A new connection active is active */
            if (lwcell_conn_is_client(conn)) {  /* Was connection started by us? */
                nc = lwcell_conn_get_arg(conn); /* Argument should be already set */
                if (nc != NULL) {
                    nc->conn = conn;           /* Save actual connection */
                } else {
                    close = 1;                 /* Close this connection, invalid netconn */
                }
            } else {
                LWCELL_DEBUGF(LWCELL_CFG_DBG_NETCONN | LWCELL_DBG_TYPE_TRACE | LWCELL_DBG_LVL_WARNING,
                             "[LWCELL NETCONN] Closing connection, it is not in client mode!\r\n");
                close = 1; /* Close the connection at this point */
            }

            /* Decide if some events want to close the connection */
            if (close) {
                if (nc != NULL) {
                    lwcell_conn_set_arg(conn, NULL); /* Reset argument */
                    lwcell_netconn_delete(nc);       /* Free memory for API */
                }
                lwcell_conn_close(conn, 0);          /* Close the connection */
                close = 0;
            }
            break;
        }

        /*
         * We have a new data received which
         * should have netconn structure as argument
         */
        case LWCELL_EVT_CONN_RECV: {
            lwcell_pbuf_p pbuf;

            nc = lwcell_conn_get_arg(conn);            /* Get API from connection */
            pbuf = lwcell_evt_conn_recv_get_buff(evt); /* Get received buff */

            lwcell_conn_recved(conn, pbuf);            /* Notify stack about received data */

            lwcell_pbuf_ref(pbuf);                     /* Increase reference counter */
            if (nc == NULL || !lwcell_sys_mbox_isvalid(&nc->mbox_receive)
                || !lwcell_sys_mbox_putnow(&nc->mbox_receive, pbuf)) {
                LWCELL_DEBUGF(LWCELL_CFG_DBG_NETCONN, "[LWCELL NETCONN] Ignoring more data for receive!\r\n");
                lwcell_pbuf_free_s(&pbuf); /* Free pbuf */
                return lwcellOKIGNOREMORE; /* Return OK to free the memory and ignore further data */
            }
            ++nc->rcv_packets;            /* Increase number of received packets */
            LWCELL_DEBUGF(LWCELL_CFG_DBG_NETCONN | LWCELL_DBG_TYPE_TRACE,
                         "[LWCELL NETCONN] Received pbuf contains %d bytes. Handle written to receive mbox\r\n",
                         (int)lwcell_pbuf_length(pbuf, 0));
            break;
        }

        /* Connection was just closed */
        case LWCELL_EVT_CONN_CLOSE: {
            nc = lwcell_conn_get_arg(conn); /* Get API from connection */

            /*
             * In case we have a netconn available,
             * simply write pointer to received variable to indicate closed state
             */
            if (nc != NULL && lwcell_sys_mbox_isvalid(&nc->mbox_receive)) {
                lwcell_sys_mbox_putnow(&nc->mbox_receive, (void*)&recv_closed);
            }

            break;
        }
        default: return lwcellERR;
    }
    return lwcellOK;
}

/**
 * \brief           Global event callback function
 * \param[in]       evt: Callback information and data
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
static lwcellr_t
lwcell_evt(lwcell_evt_t* evt) {
    switch (lwcell_evt_get_type(evt)) {
        default: break;
    }
    return lwcellOK;
}

/**
 * \brief           Create new netconn connection
 * \param[in]       type: Netconn connection type
 * \return          New netconn connection on success, `NULL` otherwise
 */
lwcell_netconn_p
lwcell_netconn_new(lwcell_netconn_type_t type) {
    lwcell_netconn_t* a;
    static uint8_t first = 1;

    /* Register only once! */
    lwcell_core_lock();
    if (first) {
        first = 0;
        lwcell_evt_register(lwcell_evt); /* Register global event function */
    }
    lwcell_core_unlock();
    a = lwcell_mem_calloc(1, sizeof(*a)); /* Allocate memory for core object */
    if (a != NULL) {
        a->type = type;                  /* Save netconn type */
        a->conn_timeout = 0;             /* Default connection timeout */
        if (!lwcell_sys_mbox_create(
                &a->mbox_receive,
                LWCELL_CFG_NETCONN_RECEIVE_QUEUE_LEN)) { /* Allocate memory for receiving message box */
            LWCELL_DEBUGF(LWCELL_CFG_DBG_NETCONN | LWCELL_DBG_TYPE_TRACE | LWCELL_DBG_LVL_DANGER,
                         "[LWCELL NETCONN] Cannot create receive MBOX\r\n");
            goto free_ret;
        }
        lwcell_core_lock();
        if (netconn_list == NULL) { /* Add new netconn to the existing list */
            netconn_list = a;
        } else {
            a->next = netconn_list; /* Add it to beginning of the list */
            netconn_list = a;
        }
        lwcell_core_unlock();
    }
    return a;
free_ret:
    if (lwcell_sys_mbox_isvalid(&a->mbox_receive)) {
        lwcell_sys_mbox_delete(&a->mbox_receive);
        lwcell_sys_mbox_invalid(&a->mbox_receive);
    }
    if (a != NULL) {
        lwcell_mem_free_s((void**)&a);
    }
    return NULL;
}

/**
 * \brief           Delete netconn connection
 * \param[in]       nc: Netconn handle
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_netconn_delete(lwcell_netconn_p nc) {
    LWCELL_ASSERT(nc != NULL);

    lwcell_core_lock();
    flush_mboxes(nc, 0); /* Clear mboxes */

    /* Remove netconn from linkedlist */
    if (netconn_list == nc) {
        netconn_list = netconn_list->next; /* Remove first from linked list */
    } else if (netconn_list != NULL) {
        lwcell_netconn_p tmp, prev;
        /* Find element on the list */
        for (prev = netconn_list, tmp = netconn_list->next; tmp != NULL; prev = tmp, tmp = tmp->next) {
            if (nc == tmp) {
                prev->next = tmp->next; /* Remove tmp from linked list */
                break;
            }
        }
    }
    lwcell_core_unlock();

    lwcell_mem_free_s((void**)&nc);
    return lwcellOK;
}

/**
 * \brief           Connect to server as client
 * \param[in]       nc: Netconn handle
 * \param[in]       host: Pointer to host, such as domain name or IP address in string format
 * \param[in]       port: Target port to use
 * \return          \ref lwcellOK if successfully connected, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_netconn_connect(lwcell_netconn_p nc, const char* host, lwcell_port_t port) {
    lwcellr_t res;

    LWCELL_ASSERT(nc != NULL);
    LWCELL_ASSERT(host != NULL);
    LWCELL_ASSERT(port > 0);

    /*
     * Start a new connection as client and:
     *
     *  - Set current netconn structure as argument
     *  - Set netconn callback function for connection management
     *  - Start connection in blocking mode
     */
    res = lwcell_conn_start(NULL, (lwcell_conn_type_t)nc->type, host, port, nc, netconn_evt, 1);
    return res;
}

/**
 * \brief           Write data to connection output buffers
 * \note            This function may only be used on TCP or SSL connections
 * \param[in]       nc: Netconn handle used to write data to
 * \param[in]       data: Pointer to data to write
 * \param[in]       btw: Number of bytes to write
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_netconn_write(lwcell_netconn_p nc, const void* data, size_t btw) {
    size_t len, sent;
    const uint8_t* d = data;
    lwcellr_t res;

    LWCELL_ASSERT(nc != NULL);
    LWCELL_ASSERT(nc->type == LWCELL_NETCONN_TYPE_TCP || nc->type == LWCELL_NETCONN_TYPE_SSL);
    LWCELL_ASSERT(lwcell_conn_is_active(nc->conn));

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
    if (nc->buff.buff != NULL) {                           /* Is there a write buffer ready to accept more data? */
        len = LWCELL_MIN(nc->buff.len - nc->buff.ptr, btw); /* Get number of bytes we can write to buffer */
        if (len > 0) {
            LWCELL_MEMCPY(&nc->buff.buff[nc->buff.ptr], data, len); /* Copy memory to temporary write buffer */
            d += len;
            nc->buff.ptr += len;
            btw -= len;
        }

        /* Step 1.1 */
        if (nc->buff.ptr == nc->buff.len) {
            res = lwcell_conn_send(nc->conn, nc->buff.buff, nc->buff.len, &sent, 1);

            lwcell_mem_free_s((void**)&nc->buff.buff);
            if (res != lwcellOK) {
                return res;
            }
        } else {
            return lwcellOK; /* Buffer is not yet full yet */
        }
    }

    /* Step 2 */
    if (btw >= LWCELL_CFG_CONN_MAX_DATA_LEN) {
        size_t rem;
        rem = btw % LWCELL_CFG_CONN_MAX_DATA_LEN;                 /* Get remaining bytes for max data length */
        res = lwcell_conn_send(nc->conn, d, btw - rem, &sent, 1); /* Write data directly */
        if (res != lwcellOK) {
            return res;
        }
        d += sent;   /* Advance in data pointer */
        btw -= sent; /* Decrease remaining data to send */
    }

    if (btw == 0) { /* Sent everything? */
        return lwcellOK;
    }

    /* Step 3 */
    if (nc->buff.buff == NULL) {                    /* Check if we should allocate a new buffer */
        nc->buff.buff = lwcell_mem_malloc(sizeof(*nc->buff.buff) * LWCELL_CFG_CONN_MAX_DATA_LEN);
        nc->buff.len = LWCELL_CFG_CONN_MAX_DATA_LEN; /* Save buffer length */
        nc->buff.ptr = 0;                           /* Save buffer pointer */
    }

    /* Step 4 */
    if (nc->buff.buff != NULL) {                              /* Memory available? */
        LWCELL_MEMCPY(&nc->buff.buff[nc->buff.ptr], d, btw);   /* Copy data to buffer */
        nc->buff.ptr += btw;
    } else {                                                  /* Still no memory available? */
        return lwcell_conn_send(nc->conn, data, btw, NULL, 1); /* Simply send directly blocking */
    }
    return lwcellOK;
}

/**
 * \brief           Extended version of \ref lwcell_netconn_write with additional
 *                  option to set custom flags.
 * 
 * \note            It is recommended to use this for full features support 
 * 
 * \param[in]       nc: Netconn handle used to write data to
 * \param[in]       data: Pointer to data to write
 * \param[in]       btw: Number of bytes to write
 * \param           flags: Bitwise-ORed set of flags for netconn.
 *                      Flags start with \ref LWCELL_NETCONN_FLAG_xxx
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_netconn_write_ex(lwcell_netconn_p nc, const void* data, size_t btw, uint16_t flags) {
    lwcellr_t res = lwcell_netconn_write(nc, data, btw);
    if (res == lwcellOK) {
        if (flags & LWCELL_NETCONN_FLAG_FLUSH) {
            res = lwcell_netconn_flush(nc);
        }
    }
    return res;
}

/**
 * \brief           Flush buffered data on netconn \e TCP/SSL connection
 * \note            This function may only be used on \e TCP/SSL connection
 * \param[in]       nc: Netconn handle to flush data
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_netconn_flush(lwcell_netconn_p nc) {
    LWCELL_ASSERT(nc != NULL);
    LWCELL_ASSERT(nc->type == LWCELL_NETCONN_TYPE_TCP || nc->type == LWCELL_NETCONN_TYPE_SSL);
    LWCELL_ASSERT(lwcell_conn_is_active(nc->conn));

    /*
     * In case we have data in write buffer,
     * flush them out to network
     */
    if (nc->buff.buff != NULL) {                                             /* Check remaining data */
        if (nc->buff.ptr > 0) {                                              /* Do we have data in current buffer? */
            lwcell_conn_send(nc->conn, nc->buff.buff, nc->buff.ptr, NULL, 1); /* Send data */
        }
        lwcell_mem_free_s((void**)&nc->buff.buff);
    }
    return lwcellOK;
}

/**
 * \brief           Send data on \e UDP connection to default IP and port
 * \param[in]       nc: Netconn handle used to send
 * \param[in]       data: Pointer to data to write
 * \param[in]       btw: Number of bytes to write
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_netconn_send(lwcell_netconn_p nc, const void* data, size_t btw) {
    LWCELL_ASSERT(nc != NULL);
    LWCELL_ASSERT(nc->type == LWCELL_NETCONN_TYPE_UDP);
    LWCELL_ASSERT(lwcell_conn_is_active(nc->conn));

    return lwcell_conn_send(nc->conn, data, btw, NULL, 1);
}

/**
 * \brief           Send data on \e UDP connection to specific IP and port
 * \note            Use this function in case of UDP type netconn
 * \param[in]       nc: Netconn handle used to send
 * \param[in]       ip: Pointer to IP address
 * \param[in]       port: Port number used to send data
 * \param[in]       data: Pointer to data to write
 * \param[in]       btw: Number of bytes to write
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_netconn_sendto(lwcell_netconn_p nc, const lwcell_ip_t* ip, lwcell_port_t port, const void* data, size_t btw) {
    LWCELL_ASSERT(nc != NULL);
    LWCELL_ASSERT(nc->type == LWCELL_NETCONN_TYPE_UDP);
    LWCELL_ASSERT(lwcell_conn_is_active(nc->conn));

    return lwcell_conn_sendto(nc->conn, ip, port, data, btw, NULL, 1);
}

/**
 * \brief           Receive data from connection
 * \param[in]       nc: Netconn handle used to receive from
 * \param[in]       pbuf: Pointer to pointer to save new receive buffer to.
 *                     When function returns, user must check for valid pbuf value `pbuf != NULL`
 * \return          \ref lwcellOK when new data ready,
 * \return          \ref lwcellCLOSED when connection closed by remote side,
 * \return          \ref lwcellTIMEOUT when receive timeout occurs
 * \return          Any other member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_netconn_receive(lwcell_netconn_p nc, lwcell_pbuf_p* pbuf) {
    LWCELL_ASSERT(nc != NULL);
    LWCELL_ASSERT(pbuf != NULL);

    *pbuf = NULL;
#if LWCELL_CFG_NETCONN_RECEIVE_TIMEOUT
    /*
     * Wait for new received data for up to specific timeout
     * or throw error for timeout notification
     */
    if (nc->rcv_timeout == LWCELL_NETCONN_RECEIVE_NO_WAIT) {
        if (!lwcell_sys_mbox_getnow(&nc->mbox_receive, (void**)pbuf)) {
            return lwcellTIMEOUT;
        }
    } else if (lwcell_sys_mbox_get(&nc->mbox_receive, (void**)pbuf, nc->rcv_timeout) == LWCELL_SYS_TIMEOUT) {
        return lwcellTIMEOUT;
    }
#else  /* LWCELL_CFG_NETCONN_RECEIVE_TIMEOUT */
    /* Forever wait for new receive packet */
    lwcell_sys_mbox_get(&nc->mbox_receive, (void**)pbuf, 0);
#endif /* !LWCELL_CFG_NETCONN_RECEIVE_TIMEOUT */

    /* Check if connection closed */
    if ((uint8_t*)(*pbuf) == (uint8_t*)&recv_closed) {
        *pbuf = NULL; /* Reset pbuf */
        return lwcellCLOSED;
    }
    return lwcellOK; /* We have data available */
}

/**
 * \brief           Close a netconn connection
 * \param[in]       nc: Netconn handle to close
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_netconn_close(lwcell_netconn_p nc) {
    lwcell_conn_p conn;

    LWCELL_ASSERT(nc != NULL);
    LWCELL_ASSERT(nc->conn != NULL);
    LWCELL_ASSERT(lwcell_conn_is_active(nc->conn));

    lwcell_netconn_flush(nc); /* Flush data and ignore result */
    conn = nc->conn;
    nc->conn = NULL;

    lwcell_conn_set_arg(conn, NULL); /* Reset argument */
    lwcell_conn_close(conn, 1);      /* Close the connection */
    flush_mboxes(nc, 1);            /* Flush message queues */
    return lwcellOK;
}

/**
 * \brief           Get connection number used for netconn
 * \param[in]       nc: Netconn handle
 * \return          `-1` on failure, connection number between `0` and \ref LWCELL_CFG_MAX_CONNS otherwise
 */
int8_t
lwcell_netconn_getconnnum(lwcell_netconn_p nc) {
    if (nc != NULL && nc->conn != NULL) {
        return lwcell_conn_getnum(nc->conn);
    }
    return -1;
}

#if LWCELL_CFG_NETCONN_RECEIVE_TIMEOUT || __DOXYGEN__

/**
 * \brief           Set timeout value for receiving data.
 *
 * When enabled, \ref lwcell_netconn_receive will only block for up to
 * \e timeout value and will return if no new data within this time
 *
 * \param[in]       nc: Netconn handle
 * \param[in]       timeout: Timeout in units of milliseconds.
 *                      Set to `0` to disable timeout feature. Function blocks until data receive or connection closed
 *                      Set to `> 0` to set maximum milliseconds to wait before timeout
 *                      Set to \ref LWCELL_NETCONN_RECEIVE_NO_WAIT to enable non-blocking receive
 */
void
lwcell_netconn_set_receive_timeout(lwcell_netconn_p nc, uint32_t timeout) {
    nc->rcv_timeout = timeout;
}

/**
 * \brief           Get netconn receive timeout value
 * \param[in]       nc: Netconn handle
 * \return          Timeout in units of milliseconds.
 *                  If value is `0`, timeout is disabled (wait forever)
 */
uint32_t
lwcell_netconn_get_receive_timeout(lwcell_netconn_p nc) {
    return nc->rcv_timeout; /* Return receive timeout */
}

#endif /* LWCELL_CFG_NETCONN_RECEIVE_TIMEOUT || __DOXYGEN__ */

#endif /* LWCELL_CFG_NETCONN || __DOXYGEN__ */
