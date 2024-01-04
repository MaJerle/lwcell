/**
 * \file            lwcell_conn.c
 * \brief           Connection API
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
#include "lwcell/lwcell_conn.h"
#include "lwcell/lwcell_private.h"

#if LWCELL_CFG_CONN || __DOXYGEN__

/**
 * \brief           Check if connection is closed or in closing state
 * \param[in]       conn: Connection handle
 */
#define CONN_CHECK_CLOSED_IN_CLOSING(conn)                                                                             \
    do {                                                                                                               \
        lwcellr_t r = lwcellOK;                                                                                        \
        lwcell_core_lock();                                                                                            \
        if (conn->status.f.in_closing || !conn->status.f.active) {                                                     \
            r = lwcellCLOSED;                                                                                          \
        }                                                                                                              \
        lwcell_core_unlock();                                                                                          \
        if (r != lwcellOK) {                                                                                           \
            return r;                                                                                                  \
        }                                                                                                              \
    } while (0)

/**
 * \brief           Timeout callback for connection
 * \param[in]       arg: Timeout callback custom argument
 */
static void
conn_timeout_cb(void* arg) {
    lwcell_conn_p conn = arg;                   /* Argument is actual connection */

    if (conn->status.f.active) {                /* Handle only active connections */
        lwcell.evt.type = LWCELL_EVT_CONN_POLL; /* Poll connection event */
        lwcell.evt.evt.conn_poll.conn = conn;   /* Set connection pointer */
        lwcelli_send_conn_cb(conn, NULL);       /* Send connection callback */

        lwcelli_conn_start_timeout(conn);       /* Schedule new timeout */
        LWCELL_DEBUGF(LWCELL_CFG_DBG_CONN | LWCELL_DBG_TYPE_TRACE, "[LWCELL CONN] Poll event: %p\r\n", (void*)conn);
    }
}

/**
 * \brief           Start timeout function for connection
 * \param[in]       conn: Connection handle as user argument
 */
void
lwcelli_conn_start_timeout(lwcell_conn_p conn) {
    lwcell_timeout_add(LWCELL_CFG_CONN_POLL_INTERVAL, conn_timeout_cb, conn); /* Add connection timeout */
}

/**
 * \brief           Get connection validation ID
 * \param[in]       conn: Connection handle
 * \return          Connection current validation ID
 */
uint8_t
lwcelli_conn_get_val_id(lwcell_conn_p conn) {
    uint8_t val_id;
    lwcell_core_lock();
    val_id = conn->val_id;
    lwcell_core_unlock();

    return val_id;
}

/**
 * \brief           Send data on already active connection of type UDP to specific remote IP and port
 * \note            In case IP and port values are not set, it will behave as normal send function (suitable for TCP too)
 * \param[in]       conn: Pointer to connection to send data
 * \param[in]       ip: Remote IP address for UDP connection
 * \param[in]       port: Remote port connection
 * \param[in]       data: Pointer to data to send
 * \param[in]       btw: Number of bytes to send
 * \param[out]      bw: Pointer to output variable to save number of sent data when successfully sent
 * \param[in]       fau: "Free After Use" flag. Set to `1` if stack should free the memory after data sent
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
static lwcellr_t
conn_send(lwcell_conn_p conn, const lwcell_ip_t* const ip, lwcell_port_t port, const void* data, size_t btw,
          size_t* const bw, uint8_t fau, const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_ASSERT(conn != NULL);
    LWCELL_ASSERT(data != NULL);
    LWCELL_ASSERT(btw > 0);

    if (bw != NULL) {
        *bw = 0;
    }

    CONN_CHECK_CLOSED_IN_CLOSING(conn); /* Check if we can continue */

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CIPSEND;

    LWCELL_MSG_VAR_REF(msg).msg.conn_send.conn = conn;
    LWCELL_MSG_VAR_REF(msg).msg.conn_send.data = data;
    LWCELL_MSG_VAR_REF(msg).msg.conn_send.btw = btw;
    LWCELL_MSG_VAR_REF(msg).msg.conn_send.bw = bw;
    LWCELL_MSG_VAR_REF(msg).msg.conn_send.remote_ip = ip;
    LWCELL_MSG_VAR_REF(msg).msg.conn_send.remote_port = port;
    LWCELL_MSG_VAR_REF(msg).msg.conn_send.fau = fau;
    LWCELL_MSG_VAR_REF(msg).msg.conn_send.val_id = lwcelli_conn_get_val_id(conn);

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 60000);
}

/**
 * \brief           Flush buffer on connection
 * \param[in]       conn: Connection to flush buffer on
 * \return          \ref lwcellOK if data flushed and put to queue, member of \ref lwcellr_t otherwise
 */
static lwcellr_t
flush_buff(lwcell_conn_p conn) {
    lwcellr_t res = lwcellOK;
    lwcell_core_lock();
    if (conn != NULL && conn->buff.buff != NULL) { /* Do we have something ready? */
        /*
         * If there is nothing to write or if write was not successful,
         * simply free the memory and stop execution
         */
        if (conn->buff.ptr > 0) { /* Anything to send at the moment? */
            res = conn_send(conn, NULL, 0, conn->buff.buff, conn->buff.ptr, NULL, 1, 0);
        } else {
            res = lwcellERR;
        }
        if (res != lwcellOK) {
            LWCELL_DEBUGF(LWCELL_CFG_DBG_CONN | LWCELL_DBG_TYPE_TRACE, "[LWCELL CONN] Free write buffer: %p\r\n",
                          (void*)conn->buff.buff);
            lwcell_mem_free_s((void**)&conn->buff.buff);
        }
        conn->buff.buff = NULL;
    }
    lwcell_core_unlock();
    return res;
}

/**
 * \brief           Initialize connection module
 */
void
lwcelli_conn_init(void) {}

/**
 * \brief           Start a new connection of specific type
 * \param[out]      conn: Pointer to connection handle to set new connection reference in case of successful connection
 * \param[in]       type: Connection type. This parameter can be a value of \ref lwcell_conn_type_t enumeration
 * \param[in]       host: Connection host. In case of IP, write it as string, ex. "192.168.1.1"
 * \param[in]       port: Connection port
 * \param[in]       arg: Pointer to user argument passed to connection if successfully connected
 * \param[in]       conn_evt_fn: Callback function for this connection
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_conn_start(lwcell_conn_p* conn, lwcell_conn_type_t type, const char* const host, lwcell_port_t port,
                  void* const arg, lwcell_evt_fn conn_evt_fn, const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_ASSERT(host != NULL);
    LWCELL_ASSERT(port > 0);
    LWCELL_ASSERT(conn_evt_fn != NULL);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CIPSTART;
    LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CIPSTATUS;
    LWCELL_MSG_VAR_REF(msg).msg.conn_start.num = LWCELL_CFG_MAX_CONNS; /* Set maximal value as invalid number */
    LWCELL_MSG_VAR_REF(msg).msg.conn_start.conn = conn;
    LWCELL_MSG_VAR_REF(msg).msg.conn_start.type = type;
    LWCELL_MSG_VAR_REF(msg).msg.conn_start.host = host;
    LWCELL_MSG_VAR_REF(msg).msg.conn_start.port = port;
    LWCELL_MSG_VAR_REF(msg).msg.conn_start.evt_func = conn_evt_fn;
    LWCELL_MSG_VAR_REF(msg).msg.conn_start.arg = arg;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 60000);
}

/**
 * \brief           Close specific or all connections
 * \param[in]       conn: Connection handle to close. Set to NULL if you want to close all connections.
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_conn_close(lwcell_conn_p conn, const uint32_t blocking) {
    lwcellr_t res = lwcellOK;
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_ASSERT(conn != NULL);

    CONN_CHECK_CLOSED_IN_CLOSING(conn); /* Check if we can continue */

    /* Proceed with close event at this point! */
    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CIPCLOSE;
    LWCELL_MSG_VAR_REF(msg).msg.conn_close.conn = conn;
    LWCELL_MSG_VAR_REF(msg).msg.conn_close.val_id = lwcelli_conn_get_val_id(conn);

    flush_buff(conn);                   /* First flush buffer */
    res = lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 1000);
    if (res == lwcellOK && !blocking) { /* Function succedded in non-blocking mode */
        lwcell_core_lock();
        LWCELL_DEBUGF(LWCELL_CFG_DBG_CONN | LWCELL_DBG_TYPE_TRACE,
                      "[LWCELL CONN] Connection %d set to closing state\r\n", (int)conn->num);
        conn->status.f.in_closing = 1; /* Connection is in closing mode but not yet closed */
        lwcell_core_unlock();
    }
    return res;
}

/**
 * \brief           Send data on active connection of type UDP to specific remote IP and port
 * \note            In case IP and port values are not set, it will behave as normal send function (suitable for TCP too)
 * \param[in]       conn: Connection handle to send data
 * \param[in]       ip: Remote IP address for UDP connection
 * \param[in]       port: Remote port connection
 * \param[in]       data: Pointer to data to send
 * \param[in]       btw: Number of bytes to send
 * \param[out]      bw: Pointer to output variable to save number of sent data when successfully sent
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_conn_sendto(lwcell_conn_p conn, const lwcell_ip_t* const ip, lwcell_port_t port, const void* data, size_t btw,
                   size_t* bw, const uint32_t blocking) {
    LWCELL_ASSERT(conn != NULL);

    flush_buff(conn); /* Flush currently written memory if exists */
    return conn_send(conn, ip, port, data, btw, bw, 0, blocking);
}

/**
 * \brief           Send data on already active connection either as client or server
 * \param[in]       conn: Connection handle to send data
 * \param[in]       data: Data to send
 * \param[in]       btw: Number of bytes to send
 * \param[out]      bw: Pointer to output variable to save number of sent data when successfully sent.
 *                      Parameter value might not be accurate if you combine \ref lwcell_conn_write and \ref lwcell_conn_send functions
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_conn_send(lwcell_conn_p conn, const void* data, size_t btw, size_t* const bw, const uint32_t blocking) {
    lwcellr_t res;
    const uint8_t* d = data;

    LWCELL_ASSERT(conn != NULL);
    LWCELL_ASSERT(data != NULL);
    LWCELL_ASSERT(btw > 0);

    lwcell_core_lock();
    if (conn->buff.buff != NULL) { /* Check if memory available */
        size_t to_copy;
        to_copy = LWCELL_MIN(btw, conn->buff.len - conn->buff.ptr);
        if (to_copy > 0) {
            LWCELL_MEMCPY(&conn->buff.buff[conn->buff.ptr], d, to_copy);
            conn->buff.ptr += to_copy;
            d += to_copy;
            btw -= to_copy;
        }
    }
    lwcell_core_unlock();
    res = flush_buff(conn); /* Flush currently written memory if exists */
    if (btw > 0) {          /* Check for remaining data */
        res = conn_send(conn, NULL, 0, d, btw, bw, 0, blocking);
    }
    return res;
}

/**
 * \brief           Notify connection about received data which means connection is ready to accept more data
 *
 * Once data reception is confirmed, stack will try to send more data to user.
 *
 * \note            Since this feature is not supported yet by AT commands, function is only prototype
 *                  and should be used in connection callback when data are received
 *
 * \note            Function is not thread safe and may only be called from connection event function
 *
 * \param[in]       conn: Connection handle
 * \param[in]       pbuf: Packet buffer received on connection
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_conn_recved(lwcell_conn_p conn, lwcell_pbuf_p pbuf) {
    LWCELL_UNUSED(conn);
    LWCELL_UNUSED(pbuf);
    return lwcellOK;
}

/**
 * \brief           Set argument variable for connection
 * \param[in]       conn: Connection handle to set argument
 * \param[in]       arg: Pointer to argument
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 * \sa              lwcell_conn_get_arg
 */
lwcellr_t
lwcell_conn_set_arg(lwcell_conn_p conn, void* const arg) {
    lwcell_core_lock();
    conn->arg = arg; /* Set argument for connection */
    lwcell_core_unlock();
    return lwcellOK;
}

/**
 * \brief           Get user defined connection argument
 * \param[in]       conn: Connection handle to get argument
 * \return          User argument
 * \sa              lwcell_conn_set_arg
 */
void*
lwcell_conn_get_arg(lwcell_conn_p conn) {
    void* arg;
    lwcell_core_lock();
    arg = conn->arg; /* Set argument for connection */
    lwcell_core_unlock();
    return arg;
}

/**
 * \brief           Gets connections status
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_get_conns_status(const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CIPSTATUS;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 1000);
}

/**
 * \brief           Check if connection type is client
 * \param[in]       conn: Pointer to connection to check for status
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwcell_conn_is_client(lwcell_conn_p conn) {
    uint8_t res = 0;
    if (conn != NULL && lwcelli_is_valid_conn_ptr(conn)) {
        lwcell_core_lock();
        res = conn->status.f.active && conn->status.f.client;
        lwcell_core_unlock();
    }
    return res;
}

/**
 * \brief           Check if connection is active
 * \param[in]       conn: Pointer to connection to check for status
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwcell_conn_is_active(lwcell_conn_p conn) {
    uint8_t res = 0;
    if (conn != NULL && lwcelli_is_valid_conn_ptr(conn)) {
        lwcell_core_lock();
        res = conn->status.f.active;
        lwcell_core_unlock();
    }
    return res;
}

/**
 * \brief           Check if connection is closed
 * \param[in]       conn: Pointer to connection to check for status
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwcell_conn_is_closed(lwcell_conn_p conn) {
    uint8_t res = 0;
    if (conn != NULL && lwcelli_is_valid_conn_ptr(conn)) {
        lwcell_core_lock();
        res = !conn->status.f.active;
        lwcell_core_unlock();
    }
    return res;
}

/**
 * \brief           Get the number from connection
 * \param[in]       conn: Connection pointer
 * \return          Connection number in case of success or -1 on failure
 */
int8_t
lwcell_conn_getnum(lwcell_conn_p conn) {
    int8_t res = -1;
    if (conn != NULL && lwcelli_is_valid_conn_ptr(conn)) {
        /* Protection not needed as every connection has always the same number */
        res = conn->num; /* Get number */
    }
    return res;
}

/**
 * \brief           Get connection from connection based event
 * \param[in]       evt: Event which happened for connection
 * \return          Connection pointer on success, `NULL` otherwise
 */
lwcell_conn_p
lwcell_conn_get_from_evt(lwcell_evt_t* evt) {
    switch (evt->type) {
        case LWCELL_EVT_CONN_ACTIVE: return lwcell_evt_conn_active_get_conn(evt);
        case LWCELL_EVT_CONN_CLOSE: return lwcell_evt_conn_close_get_conn(evt);
        case LWCELL_EVT_CONN_RECV: return lwcell_evt_conn_recv_get_conn(evt);
        case LWCELL_EVT_CONN_SEND: return lwcell_evt_conn_send_get_conn(evt);
        case LWCELL_EVT_CONN_POLL: return lwcell_evt_conn_poll_get_conn(evt);
        default: return NULL;
    }
}

/**
 * \brief           Write data to connection buffer and if it is full, send it non-blocking way
 * \note            This function may only be called from core (connection callbacks)
 * \param[in]       conn: Connection to write
 * \param[in]       data: Data to copy to write buffer
 * \param[in]       btw: Number of bytes to write
 * \param[in]       flush: Flush flag. Set to `1` if you want to send data immediately after copying
 * \param[out]      mem_available: Available memory size in current write buffer.
 *                  When the buffer length is reached, current one is sent and a new one is automatically created.
 *                  If function returns \ref lwcellOK and `*mem_available = 0`, there was a problem
 *                  allocating a new buffer for next operation
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_conn_write(lwcell_conn_p conn, const void* data, size_t btw, uint8_t flush, size_t* const mem_available) {
    size_t len;
    const uint8_t* d = data;

    LWCELL_ASSERT(conn != NULL);

    /*
     * Steps during write process:
     *
     * 1. Check if we have buffer already allocated and
     *      write data to the tail of the buffer
     *   1.1. In case buffer is full, send it non-blocking,
     *      and enable freeing after it is sent
     * 2. Check how many bytes we can copy as single buffer directly and send
     * 3. Create last buffer and copy remaining data to it even if no remaining data
     *      This is useful when calling function with no parameters (len = 0)
     * 4. Flush (send) current buffer if necessary
     */

    /* Step 1 */
    if (conn->buff.buff != NULL) {
        len = LWCELL_MIN(conn->buff.len - conn->buff.ptr, btw);
        LWCELL_MEMCPY(&conn->buff.buff[conn->buff.ptr], d, len);

        d += len;
        btw -= len;
        conn->buff.ptr += len;

        /* Step 1.1 */
        if (conn->buff.ptr == conn->buff.len || flush) {
            /* Try to send to processing queue in non-blocking way */
            if (conn_send(conn, NULL, 0, conn->buff.buff, conn->buff.ptr, NULL, 1, 0) != lwcellOK) {
                LWCELL_DEBUGF(LWCELL_CFG_DBG_CONN | LWCELL_DBG_TYPE_TRACE, "[LWCELL CONN] Free write buffer: %p\r\n",
                              conn->buff.buff);
                lwcell_mem_free_s((void**)&conn->buff.buff);
            }
            conn->buff.buff = NULL;
        }
    }

    /* Step 2 */
    while (btw >= LWCELL_CFG_CONN_MAX_DATA_LEN) {
        uint8_t* buff;
        buff = lwcell_mem_malloc(sizeof(*buff) * LWCELL_CFG_CONN_MAX_DATA_LEN);
        if (buff != NULL) {
            LWCELL_MEMCPY(buff, d, LWCELL_CFG_CONN_MAX_DATA_LEN); /* Copy data to buffer */
            if (conn_send(conn, NULL, 0, buff, LWCELL_CFG_CONN_MAX_DATA_LEN, NULL, 1, 0) != lwcellOK) {
                LWCELL_DEBUGF(LWCELL_CFG_DBG_CONN | LWCELL_DBG_TYPE_TRACE, "[LWCELL CONN] Free write buffer: %p\r\n",
                              (void*)buff);
                lwcell_mem_free_s((void**)&buff);
                return lwcellERRMEM;
            }
        } else {
            return lwcellERRMEM;
        }

        btw -= LWCELL_CFG_CONN_MAX_DATA_LEN; /* Decrease remaining length */
        d += LWCELL_CFG_CONN_MAX_DATA_LEN;   /* Advance data pointer */
    }

    /* Step 3 */
    if (conn->buff.buff == NULL) {
        conn->buff.buff = lwcell_mem_malloc(sizeof(*conn->buff.buff) * LWCELL_CFG_CONN_MAX_DATA_LEN);
        conn->buff.len = LWCELL_CFG_CONN_MAX_DATA_LEN;
        conn->buff.ptr = 0;

        LWCELL_DEBUGW(LWCELL_CFG_DBG_CONN | LWCELL_DBG_TYPE_TRACE, conn->buff.buff != NULL,
                      "[LWCELL CONN] New write buffer allocated, addr = %p\r\n", conn->buff.buff);
        LWCELL_DEBUGW(LWCELL_CFG_DBG_CONN | LWCELL_DBG_TYPE_TRACE, conn->buff.buff == NULL,
                      "[LWCELL CONN] Cannot allocate new write buffer\r\n");
    }
    if (btw > 0) {
        if (conn->buff.buff != NULL) {
            LWCELL_MEMCPY(conn->buff.buff, d, btw); /* Copy data to memory */
            conn->buff.ptr = btw;
        } else {
            return lwcellERRMEM;
        }
    }

    /* Step 4 */
    if (flush && conn->buff.buff != NULL) {
        flush_buff(conn);
    }

    /* Calculate number of available memory after write operation */
    if (mem_available != NULL) {
        if (conn->buff.buff != NULL) {
            *mem_available = conn->buff.len - conn->buff.ptr;
        } else {
            *mem_available = 0;
        }
    }
    return lwcellOK;
}

/**
 * \brief           Get total number of bytes ever received on connection and sent to user
 * \param[in]       conn: Connection handle
 * \return          Count of received bytes on connection
 */
size_t
lwcell_conn_get_total_recved_count(lwcell_conn_p conn) {
    size_t tot;

    LWCELL_ASSERT(conn != NULL);

    lwcell_core_lock();
    tot = conn->total_recved; /* Get total received bytes */
    lwcell_core_unlock();

    return tot;
}

/**
 * \brief           Get connection remote IP address
 * \param[in]       conn: Connection handle
 * \param[out]      ip: Pointer to IP output handle
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwcell_conn_get_remote_ip(lwcell_conn_p conn, lwcell_ip_t* ip) {
    if (conn != NULL && ip != NULL) {
        lwcell_core_lock();
        LWCELL_MEMCPY(ip, &conn->remote_ip, sizeof(*ip)); /* Copy data */
        lwcell_core_unlock();
        return 1;
    }
    return 0;
}

/**
 * \brief           Get connection remote port number
 * \param[in]       conn: Connection handle
 * \return          Port number on success, `0` otherwise
 */
lwcell_port_t
lwcell_conn_get_remote_port(lwcell_conn_p conn) {
    lwcell_port_t port = 0;
    if (conn != NULL) {
        lwcell_core_lock();
        port = conn->remote_port;
        lwcell_core_unlock();
    }
    return port;
}

/**
 * \brief           Get connection local port number
 * \param[in]       conn: Connection handle
 * \return          Port number on success, `0` otherwise
 */
lwcell_port_t
lwcell_conn_get_local_port(lwcell_conn_p conn) {
    lwcell_port_t port = 0;
    if (conn != NULL) {
        lwcell_core_lock();
        port = conn->local_port;
        lwcell_core_unlock();
    }
    return port;
}

#endif /* LWCELL_CFG_CONN || __DOXYGEN__ */
