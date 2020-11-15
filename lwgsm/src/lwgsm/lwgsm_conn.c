/**
 * \file            lwgsm_conn.c
 * \brief           Connection API
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
#include "lwgsm/lwgsm_private.h"
#include "lwgsm/lwgsm_conn.h"
#include "lwgsm/lwgsm_mem.h"
#include "lwgsm/lwgsm_timeout.h"

#if LWGSM_CFG_CONN || __DOXYGEN__

/**
 * \brief           Check if connection is closed or in closing state
 * \param[in]       conn: Connection handle
 */
#define CONN_CHECK_CLOSED_IN_CLOSING(conn) do { \
        lwgsmr_t r = lwgsmOK;                           \
        lwgsm_core_lock();                         \
        if (conn->status.f.in_closing || !conn->status.f.active) {  \
            r = lwgsmCLOSED;                          \
        }                                           \
        lwgsm_core_unlock();                       \
        if (r != lwgsmOK) {                           \
            return r;                               \
        }                                           \
    } while (0)

/**
 * \brief           Timeout callback for connection
 * \param[in]       arg: Timeout callback custom argument
 */
static void
conn_timeout_cb(void* arg) {
    lwgsm_conn_p conn = arg;                    /* Argument is actual connection */

    if (conn->status.f.active) {                /* Handle only active connections */
        lwgsm.evt.type = LWGSM_EVT_CONN_POLL;   /* Poll connection event */
        lwgsm.evt.evt.conn_poll.conn = conn;    /* Set connection pointer */
        lwgsmi_send_conn_cb(conn, NULL);        /* Send connection callback */

        lwgsmi_conn_start_timeout(conn);        /* Schedule new timeout */
        LWGSM_DEBUGF(LWGSM_CFG_DBG_CONN | LWGSM_DBG_TYPE_TRACE,
                   "[CONN] Poll event: %p\r\n", conn);
    }
}

/**
 * \brief           Start timeout function for connection
 * \param[in]       conn: Connection handle as user argument
 */
void
lwgsmi_conn_start_timeout(lwgsm_conn_p conn) {
    lwgsm_timeout_add(LWGSM_CFG_CONN_POLL_INTERVAL, conn_timeout_cb, conn); /* Add connection timeout */
}

/**
 * \brief           Get connection validation ID
 * \param[in]       conn: Connection handle
 * \return          Connection current validation ID
 */
uint8_t
lwgsmi_conn_get_val_id(lwgsm_conn_p conn) {
    uint8_t val_id;
    lwgsm_core_lock();
    val_id = conn->val_id;
    lwgsm_core_unlock();

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
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
static lwgsmr_t
conn_send(lwgsm_conn_p conn, const lwgsm_ip_t* const ip, lwgsm_port_t port, const void* data,
          size_t btw, size_t* const bw, uint8_t fau, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_ASSERT("conn != NULL", conn != NULL);
    LWGSM_ASSERT("data != NULL", data != NULL);
    LWGSM_ASSERT("btw > 0", btw > 0);

    if (bw != NULL) {
        *bw = 0;
    }

    CONN_CHECK_CLOSED_IN_CLOSING(conn);         /* Check if we can continue */

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_CIPSEND;

    LWGSM_MSG_VAR_REF(msg).msg.conn_send.conn = conn;
    LWGSM_MSG_VAR_REF(msg).msg.conn_send.data = data;
    LWGSM_MSG_VAR_REF(msg).msg.conn_send.btw = btw;
    LWGSM_MSG_VAR_REF(msg).msg.conn_send.bw = bw;
    LWGSM_MSG_VAR_REF(msg).msg.conn_send.remote_ip = ip;
    LWGSM_MSG_VAR_REF(msg).msg.conn_send.remote_port = port;
    LWGSM_MSG_VAR_REF(msg).msg.conn_send.fau = fau;
    LWGSM_MSG_VAR_REF(msg).msg.conn_send.val_id = lwgsmi_conn_get_val_id(conn);

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

/**
 * \brief           Flush buffer on connection
 * \param[in]       conn: Connection to flush buffer on
 * \return          \ref lwgsmOK if data flushed and put to queue, member of \ref lwgsmr_t otherwise
 */
static lwgsmr_t
flush_buff(lwgsm_conn_p conn) {
    lwgsmr_t res = lwgsmOK;
    lwgsm_core_lock();
    if (conn != NULL && conn->buff.buff != NULL) {  /* Do we have something ready? */
        /*
         * If there is nothing to write or if write was not successful,
         * simply free the memory and stop execution
         */
        if (conn->buff.ptr > 0) {               /* Anything to send at the moment? */
            res = conn_send(conn, NULL, 0, conn->buff.buff, conn->buff.ptr, NULL, 1, 0);
        } else {
            res = lwgsmERR;
        }
        if (res != lwgsmOK) {
            LWGSM_DEBUGF(LWGSM_CFG_DBG_CONN | LWGSM_DBG_TYPE_TRACE,
                       "[CONN] Free write buffer: %p\r\n", (void*)conn->buff.buff);
            lwgsm_mem_free_s((void**)&conn->buff.buff);
        }
        conn->buff.buff = NULL;
    }
    lwgsm_core_unlock();
    return res;
}

/**
 * \brief           Initialize connection module
 */
void
lwgsmi_conn_init(void) {

}

/**
 * \brief           Start a new connection of specific type
 * \param[out]      conn: Pointer to connection handle to set new connection reference in case of successful connection
 * \param[in]       type: Connection type. This parameter can be a value of \ref lwgsm_conn_type_t enumeration
 * \param[in]       host: Connection host. In case of IP, write it as string, ex. "192.168.1.1"
 * \param[in]       port: Connection port
 * \param[in]       arg: Pointer to user argument passed to connection if successfully connected
 * \param[in]       conn_evt_fn: Callback function for this connection
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_conn_start(lwgsm_conn_p* conn, lwgsm_conn_type_t type, const char* const host, lwgsm_port_t port,
               void* const arg, lwgsm_evt_fn conn_evt_fn, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_ASSERT("host != NULL", host != NULL);
    LWGSM_ASSERT("port > 0", port > 0);
    LWGSM_ASSERT("conn_evt_fn != NULL", conn_evt_fn != NULL);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_CIPSTART;
    LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CIPSTATUS;
    LWGSM_MSG_VAR_REF(msg).msg.conn_start.num = LWGSM_CFG_MAX_CONNS;/* Set maximal value as invalid number */
    LWGSM_MSG_VAR_REF(msg).msg.conn_start.conn = conn;
    LWGSM_MSG_VAR_REF(msg).msg.conn_start.type = type;
    LWGSM_MSG_VAR_REF(msg).msg.conn_start.host = host;
    LWGSM_MSG_VAR_REF(msg).msg.conn_start.port = port;
    LWGSM_MSG_VAR_REF(msg).msg.conn_start.evt_func = conn_evt_fn;
    LWGSM_MSG_VAR_REF(msg).msg.conn_start.arg = arg;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

/**
 * \brief           Close specific or all connections
 * \param[in]       conn: Connection handle to close. Set to NULL if you want to close all connections.
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_conn_close(lwgsm_conn_p conn, const uint32_t blocking) {
    lwgsmr_t res = lwgsmOK;
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_ASSERT("conn != NULL", conn != NULL);

    CONN_CHECK_CLOSED_IN_CLOSING(conn);         /* Check if we can continue */

    /* Proceed with close event at this point! */
    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_CIPCLOSE;
    LWGSM_MSG_VAR_REF(msg).msg.conn_close.conn = conn;
    LWGSM_MSG_VAR_REF(msg).msg.conn_close.val_id = lwgsmi_conn_get_val_id(conn);

    flush_buff(conn);                           /* First flush buffer */
    res = lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 1000);
    if (res == lwgsmOK && !blocking) {          /* Function succedded in non-blocking mode */
        lwgsm_core_lock();
        LWGSM_DEBUGF(LWGSM_CFG_DBG_CONN | LWGSM_DBG_TYPE_TRACE,
                   "[CONN] Connection %d set to closing state\r\n", (int)conn->num);
        conn->status.f.in_closing = 1;          /* Connection is in closing mode but not yet closed */
        lwgsm_core_unlock();
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
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_conn_sendto(lwgsm_conn_p conn, const lwgsm_ip_t* const ip, lwgsm_port_t port, const void* data,
                size_t btw, size_t* bw, const uint32_t blocking) {
    LWGSM_ASSERT("conn != NULL", conn != NULL);

    flush_buff(conn);                           /* Flush currently written memory if exists */
    return conn_send(conn, ip, port, data, btw, bw, 0, blocking);
}

/**
 * \brief           Send data on already active connection either as client or server
 * \param[in]       conn: Connection handle to send data
 * \param[in]       data: Data to send
 * \param[in]       btw: Number of bytes to send
 * \param[out]      bw: Pointer to output variable to save number of sent data when successfully sent.
 *                      Parameter value might not be accurate if you combine \ref lwgsm_conn_write and \ref lwgsm_conn_send functions
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_conn_send(lwgsm_conn_p conn, const void* data, size_t btw, size_t* const bw,
              const uint32_t blocking) {
    lwgsmr_t res;
    const uint8_t* d = data;

    LWGSM_ASSERT("conn != NULL", conn != NULL);
    LWGSM_ASSERT("data != NULL", data != NULL);
    LWGSM_ASSERT("btw > 0", btw > 0);

    lwgsm_core_lock();
    if (conn->buff.buff != NULL) {              /* Check if memory available */
        size_t to_copy;
        to_copy = LWGSM_MIN(btw, conn->buff.len - conn->buff.ptr);
        if (to_copy > 0) {
            LWGSM_MEMCPY(&conn->buff.buff[conn->buff.ptr], d, to_copy);
            conn->buff.ptr += to_copy;
            d += to_copy;
            btw -= to_copy;
        }
    }
    lwgsm_core_unlock();
    res = flush_buff(conn);                     /* Flush currently written memory if exists */
    if (btw > 0) {                              /* Check for remaining data */
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
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_conn_recved(lwgsm_conn_p conn, lwgsm_pbuf_p pbuf) {
#if LWGSM_CFG_CONN_MANUAL_TCP_RECEIVE
    size_t len;
    len = lwgsm_pbuf_length(pbuf, 1);           /* Get length of pbuf */
    if (conn->tcp_available_data > len) {
        conn->tcp_available_data -= len;        /* Decrease for available length */
        if (conn->tcp_available_data > 0) {
            /* Start new manual receive here... */
        }
    }
#else /* LWGSM_CFG_CONN_MANUAL_TCP_RECEIVE */
    LWGSM_UNUSED(conn);
    LWGSM_UNUSED(pbuf);
#endif /* !LWGSM_CFG_CONN_MANUAL_TCP_RECEIVE */
    return lwgsmOK;
}

/**
 * \brief           Set argument variable for connection
 * \param[in]       conn: Connection handle to set argument
 * \param[in]       arg: Pointer to argument
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 * \sa              lwgsm_conn_get_arg
 */
lwgsmr_t
lwgsm_conn_set_arg(lwgsm_conn_p conn, void* const arg) {
    lwgsm_core_lock();
    conn->arg = arg;                            /* Set argument for connection */
    lwgsm_core_unlock();
    return lwgsmOK;
}

/**
 * \brief           Get user defined connection argument
 * \param[in]       conn: Connection handle to get argument
 * \return          User argument
 * \sa              lwgsm_conn_set_arg
 */
void*
lwgsm_conn_get_arg(lwgsm_conn_p conn) {
    void* arg;
    lwgsm_core_lock();
    arg = conn->arg;                            /* Set argument for connection */
    lwgsm_core_unlock();
    return arg;
}

/**
 * \brief           Gets connections status
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_get_conns_status(const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_CIPSTATUS;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 1000);
}

/**
 * \brief           Check if connection type is client
 * \param[in]       conn: Pointer to connection to check for status
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_conn_is_client(lwgsm_conn_p conn) {
    uint8_t res = 0;
    if (conn != NULL && lwgsmi_is_valid_conn_ptr(conn)) {
        lwgsm_core_lock();
        res = conn->status.f.active && conn->status.f.client;
        lwgsm_core_unlock();
    }
    return res;
}

/**
 * \brief           Check if connection is active
 * \param[in]       conn: Pointer to connection to check for status
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_conn_is_active(lwgsm_conn_p conn) {
    uint8_t res = 0;
    if (conn != NULL && lwgsmi_is_valid_conn_ptr(conn)) {
        lwgsm_core_lock();
        res = conn->status.f.active;
        lwgsm_core_unlock();
    }
    return res;
}

/**
 * \brief           Check if connection is closed
 * \param[in]       conn: Pointer to connection to check for status
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_conn_is_closed(lwgsm_conn_p conn) {
    uint8_t res = 0;
    if (conn != NULL && lwgsmi_is_valid_conn_ptr(conn)) {
        lwgsm_core_lock();
        res = !conn->status.f.active;
        lwgsm_core_unlock();
    }
    return res;
}

/**
 * \brief           Get the number from connection
 * \param[in]       conn: Connection pointer
 * \return          Connection number in case of success or -1 on failure
 */
int8_t
lwgsm_conn_getnum(lwgsm_conn_p conn) {
    int8_t res = -1;
    if (conn != NULL && lwgsmi_is_valid_conn_ptr(conn)) {
        /* Protection not needed as every connection has always the same number */
        res = conn->num;                        /* Get number */
    }
    return res;
}

/**
 * \brief           Get connection from connection based event
 * \param[in]       evt: Event which happened for connection
 * \return          Connection pointer on success, `NULL` otherwise
 */
lwgsm_conn_p
lwgsm_conn_get_from_evt(lwgsm_evt_t* evt) {
    switch (evt->type) {
        case LWGSM_EVT_CONN_ACTIVE:
            return lwgsm_evt_conn_active_get_conn(evt);
        case LWGSM_EVT_CONN_CLOSE:
            return lwgsm_evt_conn_close_get_conn(evt);
        case LWGSM_EVT_CONN_RECV:
            return lwgsm_evt_conn_recv_get_conn(evt);
        case LWGSM_EVT_CONN_SEND:
            return lwgsm_evt_conn_send_get_conn(evt);
        case LWGSM_EVT_CONN_POLL:
            return lwgsm_evt_conn_poll_get_conn(evt);
        default:
            return NULL;
    }
}

/**
 * \brief           Write data to connection buffer and if it is full, send it non-blocking way
 * \note            This function may only be called from core (connection callbacks)
 * \param[in]       conn: Connection to write
 * \param[in]       data: Data to copy to write buffer
 * \param[in]       btw: Number of bytes to write
 * \param[in]       flush: Flush flag. Set to `1` if you want to send data immediately after copying
 * \param[out]      mem_available: Available memory size available in current write buffer.
 *                  When the buffer length is reached, current one is sent and a new one is automatically created.
 *                  If function returns \ref lwgsmOK and `*mem_available = 0`, there was a problem
 *                  allocating a new buffer for next operation
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_conn_write(lwgsm_conn_p conn, const void* data, size_t btw, uint8_t flush, size_t* const mem_available) {
    size_t len;

    const uint8_t* d = data;

    LWGSM_ASSERT("conn != NULL", conn != NULL);

    /*
     * Steps during write process:
     *
     * 1. Check if we have buffer already allocated and
     *      write data to the tail of buffer
     *   1.1. In case buffer is full, send it non-blocking,
     *      and enable freeing after it is sent
     * 2. Check how many bytes we can copy as single buffer directly and send
     * 3. Create last buffer and copy remaining data to it even if no remaining data
     *      This is useful when calling function with no parameters (len = 0)
     * 4. Flush (send) current buffer if necessary
     */

    /* Step 1 */
    if (conn->buff.buff != NULL) {
        len = LWGSM_MIN(conn->buff.len - conn->buff.ptr, btw);
        LWGSM_MEMCPY(&conn->buff.buff[conn->buff.ptr], d, len);

        d += len;
        btw -= len;
        conn->buff.ptr += len;

        /* Step 1.1 */
        if (conn->buff.ptr == conn->buff.len || flush) {
            /* Try to send to processing queue in non-blocking way */
            if (conn_send(conn, NULL, 0, conn->buff.buff, conn->buff.ptr, NULL, 1, 0) != lwgsmOK) {
                LWGSM_DEBUGF(LWGSM_CFG_DBG_CONN | LWGSM_DBG_TYPE_TRACE,
                           "[CONN] Free write buffer: %p\r\n", conn->buff.buff);
                lwgsm_mem_free_s((void**)&conn->buff.buff);
            }
            conn->buff.buff = NULL;
        }
    }

    /* Step 2 */
    while (btw >= LWGSM_CFG_CONN_MAX_DATA_LEN) {
        uint8_t* buff;
        buff = lwgsm_mem_malloc(sizeof(*buff) * LWGSM_CFG_CONN_MAX_DATA_LEN);
        if (buff != NULL) {
            LWGSM_MEMCPY(buff, d, LWGSM_CFG_CONN_MAX_DATA_LEN); /* Copy data to buffer */
            if (conn_send(conn, NULL, 0, buff, LWGSM_CFG_CONN_MAX_DATA_LEN, NULL, 1, 0) != lwgsmOK) {
                LWGSM_DEBUGF(LWGSM_CFG_DBG_CONN | LWGSM_DBG_TYPE_TRACE,
                           "[CONN] Free write buffer: %p\r\n", (void*)buff);
                lwgsm_mem_free_s((void**)&buff);
                return lwgsmERRMEM;
            }
        } else {
            return lwgsmERRMEM;
        }

        btw -= LWGSM_CFG_CONN_MAX_DATA_LEN;     /* Decrease remaining length */
        d += LWGSM_CFG_CONN_MAX_DATA_LEN;       /* Advance data pointer */
    }

    /* Step 3 */
    if (conn->buff.buff == NULL) {
        conn->buff.buff = lwgsm_mem_malloc(sizeof(*conn->buff.buff) * LWGSM_CFG_CONN_MAX_DATA_LEN);
        conn->buff.len = LWGSM_CFG_CONN_MAX_DATA_LEN;
        conn->buff.ptr = 0;

        LWGSM_DEBUGW(LWGSM_CFG_DBG_CONN | LWGSM_DBG_TYPE_TRACE, conn->buff.buff != NULL,
                   "[CONN] New write buffer allocated, addr = %p\r\n", conn->buff.buff);
        LWGSM_DEBUGW(LWGSM_CFG_DBG_CONN | LWGSM_DBG_TYPE_TRACE, conn->buff.buff == NULL,
                   "[CONN] Cannot allocate new write buffer\r\n");
    }
    if (btw > 0) {
        if (conn->buff.buff != NULL) {
            LWGSM_MEMCPY(conn->buff.buff, d, btw);  /* Copy data to memory */
            conn->buff.ptr = btw;
        } else {
            return lwgsmERRMEM;
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
    return lwgsmOK;
}

/**
 * \brief           Get total number of bytes ever received on connection and sent to user
 * \param[in]       conn: Connection handle
 * \return          Count of received bytes on connection
 */
size_t
lwgsm_conn_get_total_recved_count(lwgsm_conn_p conn) {
    size_t tot;

    LWGSM_ASSERT("conn != NULL", conn != NULL);

    lwgsm_core_lock();
    tot = conn->total_recved;                   /* Get total received bytes */
    lwgsm_core_unlock();

    return tot;
}

/**
 * \brief           Get connection remote IP address
 * \param[in]       conn: Connection handle
 * \param[out]      ip: Pointer to IP output handle
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_conn_get_remote_ip(lwgsm_conn_p conn, lwgsm_ip_t* ip) {
    if (conn != NULL && ip != NULL) {
        lwgsm_core_lock();
        LWGSM_MEMCPY(ip, &conn->remote_ip, sizeof(*ip));/* Copy data */
        lwgsm_core_unlock();
        return 1;
    }
    return 0;
}

/**
 * \brief           Get connection remote port number
 * \param[in]       conn: Connection handle
 * \return          Port number on success, `0` otherwise
 */
lwgsm_port_t
lwgsm_conn_get_remote_port(lwgsm_conn_p conn) {
    lwgsm_port_t port = 0;
    if (conn != NULL) {
        lwgsm_core_lock();
        port = conn->remote_port;
        lwgsm_core_unlock();
    }
    return port;
}

/**
 * \brief           Get connection local port number
 * \param[in]       conn: Connection handle
 * \return          Port number on success, `0` otherwise
 */
lwgsm_port_t
lwgsm_conn_get_local_port(lwgsm_conn_p conn) {
    lwgsm_port_t port = 0;
    if (conn != NULL) {
        lwgsm_core_lock();
        port = conn->local_port;
        lwgsm_core_unlock();
    }
    return port;
}

#endif /* LWGSM_CFG_CONN || __DOXYGEN__ */
