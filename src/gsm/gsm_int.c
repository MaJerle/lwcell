/**
 * \file            gsm_int.c
 * \brief           Internal functions
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
#include "gsm/gsm_private.h"
#include "gsm/gsm.h"
#include "gsm/gsm_int.h"
#include "gsm/gsm_mem.h"
#include "gsm/gsm_parser.h"
#include "gsm/gsm_unicode.h"
#include "system/gsm_ll.h"

#if !__DOXYGEN__
/**
 * \brief           Receive character structure to handle full line terminated with `\n` character
 */
typedef struct {
    char data[128];                             /*!< Received characters */
    uint8_t len;                                /*!< Length of valid characters */
} gsm_recv_t;

/* Receive character macros */
#define RECV_ADD(ch)                        do { if (recv_buff.len < (sizeof(recv_buff.data)) - 1) { recv_buff.data[recv_buff.len++] = ch; recv_buff.data[recv_buff.len] = 0; } } while (0)
#define RECV_RESET()                        do { recv_buff.len = 0; recv_buff.data[0] = 0; } while (0)
#define RECV_LEN()                          ((size_t)recv_buff.len)
#define RECV_IDX(index)                     recv_buff.data[index]

/* Send data over AT port */
#define AT_PORT_SEND_STR(str)               gsm.ll.send_fn((const uint8_t *)(str), (size_t)strlen(str))
#define AT_PORT_SEND_CONST_STR(str)         gsm.ll.send_fn((const uint8_t *)(str), (size_t)(sizeof(str) - 1))
#define AT_PORT_SEND_CHR(ch)                gsm.ll.send_fn((const uint8_t *)(ch), (size_t)1)
#define AT_PORT_SEND(d, l)                  gsm.ll.send_fn((const uint8_t *)(d), (size_t)(l))

/* Beginning and end of every AT command */
#define AT_PORT_SEND_BEGIN()                AT_PORT_SEND_CONST_STR("AT")
#define AT_PORT_SEND_END()                  AT_PORT_SEND(CRLF, CRLF_LEN)

/* Send special characters over AT port with condition */
#define AT_PORT_SEND_QUOTE_COND(q)          do { if ((q)) { AT_PORT_SEND_CONST_STR("\""); } } while (0)
#define AT_PORT_SEND_COMMA_COND(c)          do { if ((c)) { AT_PORT_SEND_CONST_STR(","); } } while (0)
#define AT_PORT_SEND_EQUAL_COND(e)          do { if ((e)) { AT_PORT_SEND_CONST_STR("="); } } while (0)

/* Send special characters */
#define AT_PORT_SEND_CTRL_Z()               AT_PORT_SEND_STR("\x1A")
#define AT_PORT_SEND_ESC()                  AT_PORT_SEND_STR("\x1B")
#endif /* !__DOXYGEN__ */

static gsm_recv_t recv_buff;
static gsmr_t gsmi_process_sub_cmd(gsm_msg_t* msg, uint8_t* is_ok, uint16_t* is_error);

/**
 * \brief           Memory mapping
 */
const gsm_dev_mem_map_t
gsm_dev_mem_map[] = {
#define GSM_DEV_MEMORY_ENTRY(name, str_code)    { GSM_MEM_ ## name, str_code },
#include "gsm/gsm_memories.h"
};

/**
 * \brief           Size of device memory mapping array
 */
const size_t
gsm_dev_mem_map_size = GSM_ARRAYSIZE(gsm_dev_mem_map);

/**
 * \brief           List of supported devices
 */
const gsm_dev_model_map_t
gsm_dev_model_map[] = {
#define GSM_DEVICE_MODEL_ENTRY(name, str_id, is_2g, is_lte)     { GSM_DEVICE_MODEL_ ## name, str_id, is_2g, is_lte },
#include "gsm/gsm_models.h"
};

/**
 * \brief           Size of device models mapping array
 */
const size_t
gsm_dev_model_map_size = GSM_ARRAYSIZE(gsm_dev_model_map);

/**
 * \brief           Free connection send data memory
 * \param[in]       m: Send data message type
 */
#define CONN_SEND_DATA_FREE(m)      do {            \
    if ((m) != NULL && (m)->msg.conn_send.fau) {    \
        (m)->msg.conn_send.fau = 0;                 \
        if ((m)->msg.conn_send.data != NULL) {      \
            GSM_DEBUGF(GSM_CFG_DBG_CONN | GSM_DBG_TYPE_TRACE,   \
                "[CONN] Free write buffer fau: %p\r\n", (void *)(m)->msg.conn_send.data);   \
            gsm_mem_free((void *)(m)->msg.conn_send.data);  \
            (m)->msg.conn_send.data = NULL;         \
        }                                           \
    }                                               \
} while (0)

/**
 * \brief           Send connection callback for "data send"
 * \param[in]       m: Command message
 * \param[in]       err: Error of type \ref gsmr_t
 */
#define CONN_SEND_DATA_SEND_EVT(m, err)  do { \
    CONN_SEND_DATA_FREE(m);                         \
    gsm.evt.type = GSM_EVT_CONN_SEND;               \
    gsm.evt.evt.conn_data_send.res = err;           \
    gsm.evt.evt.conn_data_send.conn = (m)->msg.conn_send.conn;  \
    gsm.evt.evt.conn_data_send.sent = (m)->msg.conn_send.sent_all;  \
    gsmi_send_conn_cb((m)->msg.conn_send.conn, NULL);   \
} while (0)

/**
 * \brief           Send reset sequence event
 * \param[in]       m: Command message
 * \param[in]       err: Error of type \ref gsmr_t
 */
#define RESET_SEND_EVT(m, err)  do {                \
    gsm.evt.evt.reset.res = err;                    \
    gsmi_send_cb(GSM_EVT_RESET);                    \
} while (0)

/**
 * \brief           Send restore sequence event
 * \param[in]       m: Connection send message
 * \param[in]       err: Error of type \ref gsmr_t
 */
#define RESTORE_SEND_EVT(m, err)  do {              \
    gsm.evt.evt.restore.res = err;                  \
    gsmi_send_cb(GSM_EVT_RESTORE);                  \
} while (0)

 /**
 * \brief           Send SMS delete operation event
 * \param[in]       m: SMS delete message
 * \param[in]       err: Error of type \ref gsmr_t
 */
#define SMS_SEND_DELETE_EVT(m, err)     do {        \
    gsm.evt.evt.sms_delete.res = err;               \
    gsm.evt.evt.sms_delete.mem = (m)->msg.sms_delete.mem;   \
    gsm.evt.evt.sms_delete.pos = (m)->msg.sms_delete.pos;   \
    gsmi_send_cb(GSM_EVT_SMS_DELETE);               \
} while (0)

/**
 * \brief           Send SMS read operation event
 * \param[in]       m: SMS read message
 * \param[in]       err: Error of type \ref gsmr_t
 */
#define SMS_SEND_READ_EVT(m, err)     do {          \
    gsm.evt.evt.sms_read.res = err;                 \
    gsm.evt.evt.sms_read.entry = (m)->msg.sms_read.entry;   \
    gsmi_send_cb(GSM_EVT_SMS_READ);                 \
} while (0)

/**
 * \brief           Send SMS read operation event
 * \param[in]       _m_: SMS list message
 * \param[in]       err: Error of type \ref gsmr_t
 */
#define SMS_SEND_LIST_EVT(_m_, err)     do {        \
    gsm.evt.evt.sms_list.mem = gsm.m.sms.mem[0].current;\
    gsm.evt.evt.sms_list.entries = (_m_)->msg.sms_list.entries; \
    gsm.evt.evt.sms_list.size = (_m_)->msg.sms_list.ei; \
    gsm.evt.evt.sms_list.res = err;                 \
    gsmi_send_cb(GSM_EVT_SMS_LIST);                 \
} while (0)

/**
 * \brief           Send SMS send operation event
 * \param[in]       m: SMS send message
 * \param[in]       err: Error of type \ref gsmr_t
 */
#define SMS_SEND_SEND_EVT(m, err)     do {          \
    gsm.evt.evt.sms_send.pos = (m)->msg.sms_send.pos;   \
    gsm.evt.evt.sms_send.res = err;                 \
    gsmi_send_cb(GSM_EVT_SMS_SEND);                 \
} while (0)

/**
 * \brief           Get SIM info when SIM is ready
 * \param[in]       blocking: Blocking command
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
gsmr_t
gsmi_get_sim_info(const uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);

    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_SIM_PROCESS_BASIC_CMDS;
    GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CNUM;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 60000);
}

/**
 * \brief           Send IP or MAC address to AT port
 * \param[in]       d: Pointer to IP or MAC address
 * \param[in]       is_ip: Set to `1` when sending IP, `0` when MAC
 * \param[in]       q: Set to `1` to include start and ending quotes
 * \param[in]       c: Set to `1` to include comma before string
 */
void
gsmi_send_ip_mac(const void* d, uint8_t is_ip, uint8_t q, uint8_t c) {
    uint8_t ch;
    char str[4];
    const gsm_mac_t* mac = d;
    const gsm_ip_t* ip = d;

    AT_PORT_SEND_COMMA_COND(c);             /* Send comma */
    if (d == NULL) {
        return;
    }
    AT_PORT_SEND_QUOTE_COND(q);             /* Send quote */
    ch = is_ip ? '.' : ':';                     /* Get delimiter character */
    for (uint8_t i = 0; i < (is_ip ? 4 : 6); i++) { /* Process byte by byte */
        if (is_ip) {                            /* In case of IP ... */
            gsm_u8_to_str(ip->ip[i], str);      /* ... go to decimal format ... */
        } else {                                /* ... in case of MAC ... */
            gsm_u8_to_hex_str(mac->mac[i], str, 2); /* ... go to HEX format */
        }
        AT_PORT_SEND_STR(str);              /* Send str */
        if (i < (is_ip ? 4 : 6) - 1) {          /* Check end if characters */
            AT_PORT_SEND_CHR(&ch);          /* Send character */
        }
    }
    AT_PORT_SEND_QUOTE_COND(q);             /* Send quote */
}

/**
 * \brief           Send string to AT port, either plain or escaped
 * \param[in]       str: Pointer to input string to string
 * \param[in]       e: Value to indicate string send format, escaped (`1`) or plain (`0`)
 * \param[in]       q: Value to indicate starting and ending quotes, enabled (`1`) or disabled (`0`)
 * \param[in]       c: Set to `1` to include comma before string
 */
void
gsmi_send_string(const char* str, uint8_t e, uint8_t q, uint8_t c) {
    char special = '\\';

    AT_PORT_SEND_COMMA_COND(c);             /* Send comma */
    AT_PORT_SEND_QUOTE_COND(q);             /* Send quote */
    if (str != NULL) {
        if (e) {                                /* Do we have to escape string? */
            while (*str) {                      /* Go through string */
                if (*str == ',' || *str == '"' || *str == '\\') {   /* Check for special character */
                    AT_PORT_SEND_CHR(&special); /* Send special character */
                }
                AT_PORT_SEND_CHR(str);      /* Send character */
                str++;
            }
        } else {
            AT_PORT_SEND_STR(str);          /* Send plain string */
        }
    }
    AT_PORT_SEND_QUOTE_COND(q);             /* Send quote */
}

/**
 * \brief           Send number (decimal) to AT port
 * \param[in]       num: Number to send to AT port
 * \param[in]       q: Value to indicate starting and ending quotes, enabled (`1`) or disabled (`0`)
 * \param[in]       c: Set to `1` to include comma before string
 */
void
gsmi_send_number(uint32_t num, uint8_t q, uint8_t c) {
    char str[11];

    gsm_u32_to_str(num, str);                   /* Convert digit to decimal string */

    AT_PORT_SEND_COMMA_COND(c);                 /* Send comma */
    AT_PORT_SEND_QUOTE_COND(q);                 /* Send quote */
    AT_PORT_SEND_STR(str);                      /* Send string with number */
    AT_PORT_SEND_QUOTE_COND(q);                 /* Send quote */
}

/**
 * \brief           Send port number to AT port
 * \param[in]       port: Port number to send
 * \param[in]       q: Value to indicate starting and ending quotes, enabled (`1`) or disabled (`0`)
 * \param[in]       c: Set to `1` to include comma before string
 */
void
gsmi_send_port(gsm_port_t port, uint8_t q, uint8_t c) {
    char str[6];

    gsm_u16_to_str(GSM_PORT2NUM(port), str);    /* Convert digit to decimal string */

    AT_PORT_SEND_COMMA_COND(c);                 /* Send comma */
    AT_PORT_SEND_QUOTE_COND(q);                 /* Send quote */
    AT_PORT_SEND_STR(str);                      /* Send string with number */
    AT_PORT_SEND_QUOTE_COND(q);                 /* Send quote */
}

/**
 * \brief           Send signed number to AT port
 * \param[in]       num: Number to send to AT port
 * \param[in]       q: Value to indicate starting and ending quotes, enabled (`1`) or disabled (`0`)
 * \param[in]       c: Set to `1` to include comma before string
 */
void
gsmi_send_signed_number(int32_t num, uint8_t q, uint8_t c) {
    char str[11];

    gsm_i32_to_str(num, str);                   /* Convert digit to decimal string */

    AT_PORT_SEND_COMMA_COND(c);                 /* Send comma */
    AT_PORT_SEND_QUOTE_COND(q);                 /* Send quote */
    AT_PORT_SEND_STR(str);                      /* Send string with number */
    AT_PORT_SEND_QUOTE_COND(q);                 /* Send quote */
}

/**
 * \brief           Send memory string to device
 * \param[in]       mem: Memory index to send
 * \param[in]       q: Value to indicate starting and ending quotes, enabled (`1`) or disabled (`0`)
 * \param[in]       c: Set to `1` to include comma before string
 */
void
gsmi_send_dev_memory(gsm_mem_t mem, uint8_t q, uint8_t c) {
    if (mem < GSM_MEM_END) {                    /* Check valid range */
        gsmi_send_string(gsm_dev_mem_map[GSM_SZ(mem)].mem_str, 0, q, c);
    }
}

#if GSM_CFG_SMS || __DOXYGEN__

/**
 * \brief           Send SMS status text
 * \param[in]       status: SMS status
 * \param[in]       q: Value to indicate starting and ending quotes, enabled (`1`) or disabled (`0`)
 * \param[in]       c: Set to `1` to include comma before string
 */
void
gsmi_send_sms_stat(gsm_sms_status_t status, uint8_t q, uint8_t c) {
    const char* t;
    switch (status) {
        case GSM_SMS_STATUS_UNREAD: t = "REC UNREAD";   break;
        case GSM_SMS_STATUS_READ:   t = "REC READ";     break;
        case GSM_SMS_STATUS_UNSENT: t = "STO UNSENT";   break;
        case GSM_SMS_STATUS_SENT:   t = "STO SENT";     break;
        case GSM_SMS_STATUS_ALL:
        default:                    t = "ALL";          break;
    }
    gsmi_send_string(t, 0, q, c);
}

#endif /* GSM_CFG_SMS */

#if GSM_CFG_CONN || __DOXYGEN__

/**
 * \brief           Reset all connections
 * \note            Used to notify upper layer stack to close everything and reset the memory if necessary
 * \param[in]       forced: Flag indicating reset was forced by user
 */
static void
reset_connections(uint8_t forced) {
    gsm.evt.type = GSM_EVT_CONN_CLOSED;
    gsm.evt.evt.conn_active_closed.forced = forced;
    
    for (size_t i = 0; i < GSM_CFG_MAX_CONNS; i++) {/* Check all connections */
        if (gsm.m.conns[i].status.f.active) {
            gsm.m.conns[i].status.f.active = 0;
    
            gsm.evt.evt.conn_active_closed.conn = &gsm.m.conns[i];
            gsm.evt.evt.conn_active_closed.client = gsm.m.conns[i].status.f.client;
            gsmi_send_conn_cb(&gsm.m.conns[i], NULL);   /* Send callback function */
        }
    }
}

#endif /* GSM_CFG_CONN || __DOXYGEN__ */

/**
 * \brief           Reset everything after reset was detected
 * \param[in]       forced: Set to `1` if reset forced by user
 */
void
gsmi_reset_everything(uint8_t forced) {
    /**
     * \todo: Put stack to default state:
     *          - Close all the connection in memory
     *          - Clear entire data memory
     *          - Reset GSM structure
     */

#if GSM_CFG_CONN
     /* Manually close all connections in memory */
    reset_connections(forced);

    /* Check if IPD active */
    if (gsm.m.ipd.buff != NULL) {
        gsm_pbuf_free(gsm.m.ipd.buff);
        gsm.m.ipd.buff = NULL;
    }
#endif /* GSM_CFG_CONN */

#if GSM_CFG_NETWORK
    /* Notify app about detached network PDP context */
    if (gsm.m.network.is_attached) {
        gsm.m.network.is_attached = 0;
        gsmi_send_cb(GSM_EVT_NETWORK_DETACHED);
    }
#endif /* GSM_CFG_NETWORK */

    /* Invalid GSM modules */
    GSM_MEMSET(&gsm.m, 0x00, sizeof(gsm.m));

    /* Manually set states */
    gsm.m.sim.state = (gsm_sim_state_t)-1;
    gsm.m.model = GSM_DEVICE_MODEL_UNKNOWN;
}

/**
 * \brief           Process callback function to user with specific type
 * \param[in]       type: Callback event type
 * \return          Member of \ref gsmr_t enumeration
 */
gsmr_t
gsmi_send_cb(gsm_evt_type_t type) {
    gsm.evt.type = type;                         /* Set callback type to process */

    /* Call callback function for all registered functions */
    for (gsm_evt_func_t* link = gsm.evt_func; link != NULL; link = link->next) {
        link->fn(&gsm.evt);
    }
    return gsmOK;
}

#if GSM_CFG_CONN || __DOXYGEN__

/**
 * \brief           Process connection callback
 * \note            Before calling function, callback structure must be prepared
 * \param[in]       conn: Pointer to connection to use as callback
 * \param[in]       evt: Event callback function for connection
 * \return          Member of \ref gsmr_t enumeration
 */
gsmr_t
gsmi_send_conn_cb(gsm_conn_t* conn, gsm_evt_fn evt) {
    if (conn->status.f.in_closing && gsm.evt.type != GSM_EVT_CONN_CLOSED) { /* Do not continue if in closing mode */
        /* return gsmOK; */
    }

    if (evt != NULL) {                          /* Try with user connection */
        return evt(&gsm.evt);                   /* Call temporary function */
    } else if (conn != NULL && conn->evt_func != NULL) {/* Connection custom callback? */
        return conn->evt_func(&gsm.evt);        /* Process callback function */
    } else if (conn == NULL) {
        return gsmOK;
    }

    /*
     * On normal API operation,
     * we should never enter to this part of code
     */

    /*
     * If connection doesn't have callback function
     * automatically close the connection?
     *
     * Since function call is non-blocking,
     * it will set active connection to closing mode
     * and further callback events should not be executed anymore
     */
    return gsm_conn_close(conn, 0);
}

/**
 * \brief           Process and send data from device buffer
 * \return          Member of \ref gsmr_t enumeration
 */
static gsmr_t
gsmi_tcpip_process_send_data(void) {
    gsm_conn_t* c = gsm.msg->msg.conn_send.conn;
    if (!gsm_conn_is_active(c) ||               /* Is the connection already closed? */
        gsm.msg->msg.conn_send.val_id != c->val_id  /* Did validation ID change after we set parameter? */
    ) {
        /* Send event to user about failed send event */
        CONN_SEND_DATA_SEND_EVT(gsm.msg, gsmCLOSED);
        return gsmERR;
    }
    gsm.msg->msg.conn_send.sent = GSM_MIN(gsm.msg->msg.conn_send.btw, GSM_CFG_CONN_MAX_DATA_LEN);

    AT_PORT_SEND_BEGIN();
    AT_PORT_SEND_CONST_STR("+CIPSEND=");
    gsmi_send_number(GSM_U32(c->num), 0, 0);    /* Send connection number */
    gsmi_send_number(GSM_U32(gsm.msg->msg.conn_send.sent), 0, 1);   /* Send length number */

    /* On UDP connections, IP address and port may be selected */
    if (c->type == GSM_CONN_TYPE_UDP) {
        if (gsm.msg->msg.conn_send.remote_ip != NULL && gsm.msg->msg.conn_send.remote_port) {
            gsmi_send_ip_mac(gsm.msg->msg.conn_send.remote_ip, 1, 1, 1);/* Send IP address including quotes */
            gsmi_send_port(gsm.msg->msg.conn_send.remote_port, 0, 1);   /* Send length number */
        }
    }
    AT_PORT_SEND_END();
    return gsmOK;
}

/**
 * \brief           Process data sent and send remaining
 * \param[in]       sent: Status whether data were sent or not,
 *                      info received from GSM with "SEND OK" or "SEND FAIL"
 * \return          `1` in case we should stop sending or `0` if we still have data to process
 */
static uint8_t
gsmi_tcpip_process_data_sent(uint8_t sent) {
    if (sent) {                                 /* Data were successfully sent */
        gsm.msg->msg.conn_send.sent_all += gsm.msg->msg.conn_send.sent;
        gsm.msg->msg.conn_send.btw -= gsm.msg->msg.conn_send.sent;
        gsm.msg->msg.conn_send.ptr += gsm.msg->msg.conn_send.sent;
        if (gsm.msg->msg.conn_send.bw != NULL) {
            *gsm.msg->msg.conn_send.bw += gsm.msg->msg.conn_send.sent;
        }
        gsm.msg->msg.conn_send.tries = 0;
    } else {                                    /* We were not successful */
        gsm.msg->msg.conn_send.tries++;         /* Increase number of tries */
        if (gsm.msg->msg.conn_send.tries == GSM_CFG_MAX_SEND_RETRIES) { /* In case we reached max number of retransmissions */
            return 1;                           /* Return 1 and indicate error */
        }
    }
    if (gsm.msg->msg.conn_send.btw > 0) {       /* Do we still have data to send? */
        if (gsmi_tcpip_process_send_data() != gsmOK) {  /* Check if we can continue */
            return 1;                           /* Finish at this point */
        }
        return 0;                               /* We still have data to send */
    }
    return 1;                                   /* Everything was sent, we can stop execution */
}

/**
 * \brief           Process CIPSEND response
 * \param[in]       rcv: Received data
 * \param[in,out]   is_ok: Pointer to current ok status
 * \param[in,out]   is_ok: Pointer to current error status
 */
void
gsmi_process_cipsend_response(gsm_recv_t* rcv, uint8_t* is_ok, uint16_t* is_error) {
    if (gsm.msg->msg.conn_send.wait_send_ok_err) {
        if (GSM_CHARISNUM(rcv->data[0]) && rcv->data[1] == ',') {
            uint8_t num = GSM_CHARTONUM(rcv->data[0]);
            if (!strncmp(&rcv->data[3], "SEND OK" CRLF, 7 + CRLF_LEN)) {
                gsm.msg->msg.conn_send.wait_send_ok_err = 0;
                *is_ok = gsmi_tcpip_process_data_sent(1);    /* Process as data were sent */
                if (*is_ok && gsm.msg->msg.conn_send.conn->status.f.active) {
                    CONN_SEND_DATA_SEND_EVT(gsm.msg, gsmOK);
                }
            } else if (!strncmp(&rcv->data[3], "SEND FAIL" CRLF, 9 + CRLF_LEN)) {
                gsm.msg->msg.conn_send.wait_send_ok_err = 0;
                *is_error = gsmi_tcpip_process_data_sent(0);/* Data were not sent due to SEND FAIL or command didn't even start */
                if (*is_error && gsm.msg->msg.conn_send.conn->status.f.active) {
                    CONN_SEND_DATA_SEND_EVT(gsm.msg, gsmERR);
                }
            }
            GSM_UNUSED(num);
        }
    /* Check for an error or if connection closed in the meantime */
    } else if (*is_error) {
        CONN_SEND_DATA_SEND_EVT(gsm.msg, gsmERR);
    }
}

/**
 * \brief           Send error event to application layer
 * \param[in]       msg: Message from user with connection start
 */
static void
gsmi_send_conn_error_cb(gsm_msg_t* msg, gsmr_t error) {
    gsm.evt.type = GSM_EVT_CONN_ERROR;          /* Connection error */
    gsm.evt.evt.conn_error.host = gsm.msg->msg.conn_start.host;
    gsm.evt.evt.conn_error.port = gsm.msg->msg.conn_start.port;
    gsm.evt.evt.conn_error.type = gsm.msg->msg.conn_start.type;
    gsm.evt.evt.conn_error.arg = gsm.msg->msg.conn_start.arg;
    gsm.evt.evt.conn_error.err = error;

    /* Call callback specified by user on connection startup */
    gsm.msg->msg.conn_start.evt_func(&gsm.evt);
    GSM_UNUSED(msg);
}

/**
 * \brief           Checks if connection pointer has valid address
 * \param[in]       conn: Address to check if valid connection ptr
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_is_valid_conn_ptr(gsm_conn_p conn) {
    uint8_t i = 0;
    for (i = 0; i < GSM_ARRAYSIZE(gsm.m.conns); i++) {
        if (conn == &gsm.m.conns[i]) {
            return 1;
        }
    }
    return 0;
}

/**
 * \brief           Connection close event detected, process with callback to user
 * \param[in]       conn_num: Connection number
 * \param[in]       forced: Set to `1` if close forced by command, `0` otherwise
 * \return          `1` on success, `0` otherwise
 */
uint8_t
gsmi_conn_closed_process(uint8_t conn_num, uint8_t forced) {
    gsm_conn_t* conn = &gsm.m.conns[conn_num];

    conn->status.f.active = 0;

    /* Check if write buffer is set */
    if (conn->buff.buff != NULL) {
        GSM_DEBUGF(GSM_CFG_DBG_CONN | GSM_DBG_TYPE_TRACE,
            "[CONN] Free write buffer: %p\r\n", conn->buff.buff);
        gsm_mem_free(conn->buff.buff);  /* Free the memory */
        conn->buff.buff = NULL;
    }

    /* Send event */
    gsm.evt.type = GSM_EVT_CONN_CLOSED;
    gsm.evt.evt.conn_active_closed.conn = conn;
    gsm.evt.evt.conn_active_closed.forced = forced;
    gsm.evt.evt.conn_active_closed.client = conn->status.f.client;
    gsmi_send_conn_cb(conn, NULL);

    return 1;
}

#endif /* GSM_CFG_CONN || __DOXYGEN__ */

/**
 * \brief           Process received string from GSM
 * \param[in]       recv: Pointer to \ref gsm_rect_t structure with input string
 */
static void
gsmi_parse_received(gsm_recv_t* rcv) {
    uint8_t is_ok = 0;
    uint16_t is_error = 0;

    /* Try to remove non-parsable strings */
    if (rcv->len == 2 && rcv->data[0] == '\r' && rcv->data[1] == '\n') {
        return;
    }

    /* Check OK response */
    is_ok = rcv->len == (2 + CRLF_LEN) && !strcmp(rcv->data, "OK" CRLF);    /* Check if received string is OK */
    if (!is_ok) {                               /* Check for SHUT OK string */
        is_ok = rcv->len == (7 + CRLF_LEN) && !strcmp(rcv->data, "SEND OK" CRLF);
    }

    /* Check error response */
    if (!is_ok) {                               /* If still not ok, check if error? */
        is_error = rcv->data[0] == '+' && !strncmp(rcv->data, "+CME ERROR", 10); /* First check +CME coded errors */
        if (!is_error) {                        /* Check basic error aswell */
            is_error = rcv->data[0] == '+' && !strncmp(rcv->data, "+CMS ERROR", 10); /* First check +CME coded errors */
            if (!is_error) {
                is_error = !strcmp(rcv->data, "ERROR" CRLF) || !strcmp(rcv->data, "FAIL" CRLF);
            }
        }
    }

    /* Scan received strings which start with '+' */
    if (rcv->data[0] == '+') {
        if (!strncmp(rcv->data, "+CSQ", 4)) {
            gsmi_parse_csq(rcv->data);          /* Parse +CSQ response */
#if GSM_CFG_NETWORK
        } else if (!strncmp(rcv->data, "+PDP: DEACT", 11)) {
            /* PDP has been deactivated */
            gsm_network_check_status(NULL, NULL, 0);/* Update status */
#endif /* GSM_CFG_NETWORK */
#if GSM_CFG_CONN
        } else if (!strncmp(rcv->data, "+RECEIVE", 8)) {
            gsmi_parse_ipd(rcv->data);          /* Parse IPD */
#endif /* GSM_CFG_CONN */
        } else if (!strncmp(rcv->data, "+CREG", 5)) {   /* Check for +CREG indication */
            gsmi_parse_creg(rcv->data, GSM_U8(CMD_IS_CUR(GSM_CMD_CREG_GET)));  /* Parse +CREG response */
        } else if (!strncmp(rcv->data, "+CPIN", 5)) {   /* Check for +CPIN indication for SIM */
            gsmi_parse_cpin(rcv->data, 1 /* !CMD_IS_DEF(GSM_CMD_CPIN_SET) */);  /* Parse +CPIN response */
        } else if (CMD_IS_CUR(GSM_CMD_COPS_GET) && !strncmp(rcv->data, "+COPS", 5)) {
            gsmi_parse_cops(rcv->data);         /* Parse current +COPS */
#if GSM_CFG_SMS
        } else if (CMD_IS_CUR(GSM_CMD_CMGS) && !strncmp(rcv->data, "+CMGS", 5)) {
            gsmi_parse_cmgs(rcv->data, &gsm.msg->msg.sms_send.pos);  /* Parse +CMGS response */
        } else if (CMD_IS_CUR(GSM_CMD_CMGR) && !strncmp(rcv->data, "+CMGR", 5)) {
            if (gsmi_parse_cmgr(rcv->data)) {   /* Parse +CMGR response */
                gsm.msg->msg.sms_read.read = 2; /* Set read flag and process the data */
            } else {
                gsm.msg->msg.sms_read.read = 1; /* Read but ignore data */
            }
        } else if (CMD_IS_CUR(GSM_CMD_CMGL) && !strncmp(rcv->data, "+CMGL", 5)) {
            if (gsmi_parse_cmgl(rcv->data)) {   /* Parse +CMGL response */
                gsm.msg->msg.sms_list.read = 2; /* Set read flag and process the data */
            } else {
                gsm.msg->msg.sms_list.read = 1; /* Read but ignore data */
            }
        } else if (!strncmp(rcv->data, "+CMTI", 5)) {
            gsmi_parse_cmti(rcv->data, 1);      /* Parse +CMTI response with received SMS */
        } else if (CMD_IS_CUR(GSM_CMD_CPMS_GET_OPT) && !strncmp(rcv->data, "+CPMS", 5)) {
            gsmi_parse_cpms(rcv->data, 0);      /* Parse +CPMS with SMS memories info */
        } else if (CMD_IS_CUR(GSM_CMD_CPMS_GET) && !strncmp(rcv->data, "+CPMS", 5)) {
            gsmi_parse_cpms(rcv->data, 1);      /* Parse +CPMS with SMS memories info */
        } else if (CMD_IS_CUR(GSM_CMD_CPMS_SET) && !strncmp(rcv->data, "+CPMS", 5)) {
            gsmi_parse_cpms(rcv->data, 2);      /* Parse +CPMS with SMS memories info */
#endif /* GSM_CFG_SMS */
#if GSM_CFG_CALL
        } else if (!strncmp(rcv->data, "+CLCC", 5)) {
            gsmi_parse_clcc(rcv->data, 1);      /* Parse +CLCC response with call info change */
#endif /* GSM_CFG_CALL */
#if GSM_CFG_PHONEBOOK
        } else if (CMD_IS_CUR(GSM_CMD_CPBS_GET_OPT) && !strncmp(rcv->data, "+CPBS", 5)) {
            gsmi_parse_cpbs(rcv->data, 0);      /* Parse +CPBS response */
        } else if (CMD_IS_CUR(GSM_CMD_CPBS_GET) && !strncmp(rcv->data, "+CPBS", 5)) {
            gsmi_parse_cpbs(rcv->data, 1);      /* Parse +CPBS response */
        } else if (CMD_IS_CUR(GSM_CMD_CPBS_SET) && !strncmp(rcv->data, "+CPBS", 5)) {
            gsmi_parse_cpbs(rcv->data, 2);      /* Parse +CPBS response */
        } else if (CMD_IS_CUR(GSM_CMD_CPBR) && !strncmp(rcv->data, "+CPBR", 5)) {
            gsmi_parse_cpbr(rcv->data);         /* Parse +CPBR statement */
        } else if (CMD_IS_CUR(GSM_CMD_CPBF) && !strncmp(rcv->data, "+CPBF", 5)) {
            gsmi_parse_cpbf(rcv->data);         /* Parse +CPBR statement */
#endif /* GSM_CFG_PHONEBOOK */
        }

    /* Messages not starting with '+' sign */
    } else {
        if (rcv->data[0] == 'S' && !strncmp(rcv->data, "SHUT OK" CRLF, 7 + CRLF_LEN)) {
            is_ok = 1;
#if GSM_CFG_CONN
        } else if (GSM_CHARISNUM(rcv->data[0]) && rcv->data[1] == ',' && rcv->data[2] == ' '
            && (!strncmp(&rcv->data[3], "CLOSE OK" CRLF, 8 + CRLF_LEN) || !strncmp(&rcv->data[3], "CLOSED" CRLF, 6 + CRLF_LEN))) {
            uint8_t forced = 0, num;

            num = GSM_CHARTONUM(rcv->data[0]);  /* Get connection number */
            if (CMD_IS_CUR(GSM_CMD_CIPCLOSE) && gsm.msg->msg.conn_close.conn->num == num) {
                forced = 1;
                is_ok = 1;                      /* If forced and connection is closed, command is OK */
            }

            /* Manually stop send command? */
            if (CMD_IS_CUR(GSM_CMD_CIPSEND) && gsm.msg->msg.conn_send.conn->num == num) {
                /*
                 * If active command is CIPSEND and CLOSED event received,
                 * manually set error and process usual "ERROR" event on senddata
                 */
                is_error = 1;                   /* This is an error in response */
                gsmi_process_cipsend_response(rcv, &is_ok, &is_error);
            }
            gsmi_conn_closed_process(num, forced);  /* Connection closed, process */
#endif /* GSM_CFG_CONN */
#if GSM_CFG_CALL
        } else if (rcv->data[0] == 'C' && !strncmp(rcv->data, "Call Ready" CRLF, 10 + CRLF_LEN)) {
            gsm.m.call.ready = 1;
            gsmi_send_cb(GSM_EVT_CALL_READY);   /* Send CALL ready event */
        } else if (rcv->data[0] == 'R' && !strncmp(rcv->data, "RING" CRLF, 4 + CRLF_LEN)) {
            gsmi_send_cb(GSM_EVT_CALL_RING);    /* Send call ring */
        } else if (rcv->data[0] == 'N' && !strncmp(rcv->data, "NO CARRIER" CRLF, 10 + CRLF_LEN)) {
            gsmi_send_cb(GSM_EVT_CALL_NO_CARRIER);  /* Send call no carrier event */
        } else if (rcv->data[0] == 'B' && !strncmp(rcv->data, "BUSY" CRLF, 4 + CRLF_LEN)) {
            gsmi_send_cb(GSM_EVT_CALL_BUSY);    /* Send call busy message */
#endif /* GSM_CFG_CALL */
#if GSM_CFG_SMS
        } else if (rcv->data[0] == 'S' && !strncmp(rcv->data, "SMS Ready" CRLF, 9 + CRLF_LEN)) {
            gsm.m.sms.ready = 1;                /* SMS ready flag */
            gsmi_send_cb(GSM_EVT_SMS_READY);    /* Send SMS ready event */
#endif /* GSM_CFG_SMS */
        } else if ((CMD_IS_CUR(GSM_CMD_CGMI_GET) || CMD_IS_CUR(GSM_CMD_CGMM_GET) || CMD_IS_CUR(GSM_CMD_CGSN_GET) || CMD_IS_CUR(GSM_CMD_CGMR_GET))
                    && !is_ok && !is_error && strncmp(rcv->data, "AT+", 3)) {
            const char* tmp = rcv->data;
            size_t tocopy;
            if (CMD_IS_CUR(GSM_CMD_CGMI_GET)) { /* Check device manufacturer */
                gsmi_parse_string(&tmp, gsm.m.model_manufacturer, sizeof(gsm.m.model_manufacturer), 1);
                if (CMD_IS_DEF(GSM_CMD_CGMI_GET)) {
                    tocopy = GSM_MIN(sizeof(gsm.m.model_manufacturer), gsm.msg->msg.device_info.len);
                    GSM_MEMCPY(gsm.msg->msg.device_info.str, gsm.m.model_manufacturer, tocopy);
                    gsm.msg->msg.device_info.str[tocopy - 1] = 0;
                }
            } else if (CMD_IS_CUR(GSM_CMD_CGMM_GET)) {  /* Check device model number */
                gsmi_parse_string(&tmp, gsm.m.model_number, sizeof(gsm.m.model_number), 1);
                if (CMD_IS_DEF(GSM_CMD_CGMM_GET)) {
                    tocopy = GSM_MIN(sizeof(gsm.m.model_number), gsm.msg->msg.device_info.len);
                    GSM_MEMCPY(gsm.msg->msg.device_info.str, gsm.m.model_number, tocopy);
                    gsm.msg->msg.device_info.str[tocopy - 1] = 0;
                }
                for (size_t i = 0; i < gsm_dev_model_map_size; i++) {
                    if (strstr(gsm.m.model_number, gsm_dev_model_map[i].id_str) != NULL) {
                        gsm.m.model = gsm_dev_model_map[i].model;
                        break;
                    }
                }
            } else if (CMD_IS_CUR(GSM_CMD_CGSN_GET)) {  /* Check device serial number */
                gsmi_parse_string(&tmp, gsm.m.model_serial_number, sizeof(gsm.m.model_serial_number), 1);
                if (CMD_IS_DEF(GSM_CMD_CGSN_GET)) {
                    tocopy = GSM_MIN(sizeof(gsm.m.model_serial_number), gsm.msg->msg.device_info.len);
                    GSM_MEMCPY(gsm.msg->msg.device_info.str, gsm.m.model_serial_number, tocopy);
                    gsm.msg->msg.device_info.str[tocopy - 1] = 0;
                }
            } else if (CMD_IS_CUR(GSM_CMD_CGMR_GET)) {  /* Check device revision */
                if (!strncmp(tmp, "Revision:", 9)) {
                    tmp += 9;
                }
                gsmi_parse_string(&tmp, gsm.m.model_revision, sizeof(gsm.m.model_revision), 1);
                if (CMD_IS_DEF(GSM_CMD_CGMR_GET)) {
                    tocopy = GSM_MIN(sizeof(gsm.m.model_revision), gsm.msg->msg.device_info.len);
                    GSM_MEMCPY(gsm.msg->msg.device_info.str, gsm.m.model_revision, tocopy);
                    gsm.msg->msg.device_info.str[tocopy - 1] = 0;
                }
            }
        } else if (CMD_IS_CUR(GSM_CMD_CIFSR) && GSM_CHARISNUM(rcv->data[0])) {
            const char* tmp = rcv->data;
            gsmi_parse_ip(&tmp, &gsm.m.network.ip_addr);/* Parse IP address */

            is_ok = 1;                          /* Manually set OK flag as we don't expect OK in CIFSR command */
        }
    }

    /* Check general responses for active commands */
    if (gsm.msg != NULL) {
        if (0) {
#if GSM_CFG_SMS
        } else if (CMD_IS_CUR(GSM_CMD_CMGS) && is_ok) {
            /* At this point we have to wait for "> " to send data */
#endif /* GSM_CFG_SMS */
#if GSM_CFG_CONN
        } else if (CMD_IS_CUR(GSM_CMD_CIPSTATUS)) {
            /* For CIPSTATUS, OK is returned before important data */
            if (is_ok) {
                is_ok = 0;
            }
            /* Check if connection data received */
            if (rcv->len > 3) {
                uint8_t continueScan, processed = 0;
                if (rcv->data[0] == 'C' && rcv->data[1] == ':' && rcv->data[2] == ' ') {
                    processed = 1;
                    gsmi_parse_cipstatus_conn(rcv->data, 1, &continueScan);

                    if (gsm.m.active_conns_cur_parse_num == (GSM_CFG_MAX_CONNS - 1)) {
                        is_ok = 1;
                    }
                } else if (!strncmp(rcv->data, "STATE:", 6)) {
                    processed = 1;
                    gsmi_parse_cipstatus_conn(rcv->data, 0, &continueScan);
                }

                /* Check if we shall stop processing at this stage */
                if (processed && !continueScan) {
                    is_ok = 1;
                }
            }
        } else if (CMD_IS_CUR(GSM_CMD_CIPSTART)) {
            /* For CIPSTART, OK is returned before important data */
            if (is_ok) {
                is_ok = 0;
            }

            /* Wait here for CONNECT status before we cancel connection */
            if (GSM_CHARISNUM(rcv->data[0])
                && rcv->data[1] == ',' && rcv->data[2] == ' ') {
                uint8_t num = GSM_CHARTONUM(rcv->data[0]);
                if (num < GSM_CFG_MAX_CONNS) {
                    uint8_t id;
                    gsm_conn_t* conn = &gsm.m.conns[num];   /* Get connection handle */

                    if (!strncmp(&rcv->data[3], "CONNECT OK" CRLF, 10 + CRLF_LEN)) {
                        id = conn->val_id;
                        GSM_MEMSET(conn, 0x00, sizeof(*conn));  /* Reset connection parameters */
                        conn->num = num;
                        conn->status.f.active = 1;
                        conn->val_id = ++id;            /* Set new validation ID */

                        /* Set connection parameters */
                        conn->status.f.client = 1;
                        conn->evt_func = gsm.msg->msg.conn_start.evt_func;
                        conn->arg = gsm.msg->msg.conn_start.arg;

                        /* Set status */
                        gsm.msg->msg.conn_start.conn_res = GSM_CONN_CONNECT_OK;
                        is_ok = 1;
                    } else if (!strncmp(&rcv->data[3], "CONNECT FAIL" CRLF, 12 + CRLF_LEN)) {
                        gsm.msg->msg.conn_start.conn_res = GSM_CONN_CONNECT_ERROR;
                        is_error = 1;
                    } else if (!strncmp(&rcv->data[3], "ALREADY CONNECT" CRLF, 15 + CRLF_LEN)) {
                        gsm.msg->msg.conn_start.conn_res = GSM_CONN_CONNECT_ALREADY;
                        is_error = 1;
                    }
                }
            }
        } else if (CMD_IS_CUR(GSM_CMD_CIPSEND)) {
            if (is_ok) {
                is_ok = 0;
            }
            gsmi_process_cipsend_response(rcv, &is_ok, &is_error);
#endif /* GSM_CFG_CONN */
        }
    }

    /*
     * In case of any of these events, simply release semaphore
     * and proceed with next command
     */
    if (is_ok || is_error) {
        gsmr_t res = gsmOK;
        if (gsm.msg != NULL) {                  /* Do we have active message? */
            res = gsmi_process_sub_cmd(gsm.msg, &is_ok, &is_error);
            if (res != gsmCONT) {               /* Shall we continue with next subcommand under this one? */
                if (is_ok) {                    /* Check OK status */
                    res = gsm.msg->res = gsmOK;
                } else {                        /* Or error status */
                    res = gsm.msg->res = res;   /* Set the error status */
                }
            } else {
                gsm.msg->i++;                   /* Number of continue calls */
            }

            /*
             * When the command is finished,
             * release synchronization semaphore
             * from user thread and start with next command
             */
            if (res != gsmCONT) {                   /* Do we have to continue to wait for command? */
                gsm_sys_sem_release(&gsm.sem_sync); /* Release semaphore */
            }
        }
    }
}

#if !GSM_CFG_INPUT_USE_PROCESS || __DOXYGEN__
/**
 * \brief           Process data from input buffer
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsmi_process_buffer(void) {
    void* data;
    size_t len;

    do {
        /*
         * Get length of linear memory in buffer
         * we can process directly as memory
         */
        len = gsm_buff_get_linear_block_read_length(&gsm.buff);
        if (len > 0) {
            /*
             * Get memory address of first element
             * in linear block of data to process
             */
            data = gsm_buff_get_linear_block_read_address(&gsm.buff);

            /* Process actual received data */
            gsmi_process(data, len);

            /*
             * Once data is processed, simply skip
             * the buffer memory and start over
             */
            gsm_buff_skip(&gsm.buff, len);
        }
    } while (len);
    return gsmOK;
}
#endif /* !GSM_CFG_INPUT_USE_PROCESS || __DOXYGEN__ */

/**
 * \brief           Process input data received from GSM device
 * \param[in]       data: Pointer to data to process
 * \param[in]       data_len: Length of data to process in units of bytes
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsmi_process(const void* data, size_t data_len) {
    uint8_t ch;
    const uint8_t* d = data;
    size_t d_len = data_len;
    static uint8_t ch_prev1, ch_prev2;
    static gsm_unicode_t unicode;

    /* Check status if device is available */
    if (!gsm.status.f.dev_present) {
        return gsmERRNODEVICE;
    }

    while (d_len) {                             /* Read entire set of characters from buffer */
        ch = *d++;                              /* Get next character */
        d_len--;                                /* Decrease remaining length */

        if (0) {
#if GSM_CFG_CONN
        } else if (gsm.m.ipd.read) {            /* Read connection data */
            size_t len;

            if (gsm.m.ipd.buff != NULL) {       /* Do we have active buffer? */
                gsm.m.ipd.buff->payload[gsm.m.ipd.buff_ptr] = ch;   /* Save data character */
            }
            gsm.m.ipd.buff_ptr++;
            gsm.m.ipd.rem_len--;

            /* Try to read more data directly from buffer */
            len = GSM_MIN(d_len, GSM_MIN(gsm.m.ipd.rem_len, gsm.m.ipd.buff != NULL ? (gsm.m.ipd.buff->len - gsm.m.ipd.buff_ptr) : gsm.m.ipd.rem_len));
            GSM_DEBUGF(GSM_CFG_DBG_IPD | GSM_DBG_TYPE_TRACE,
                "[IPD] New length to read: %d bytes\r\n", (int)len);
            if (len > 0) {
                if (gsm.m.ipd.buff != NULL) {   /* Is buffer valid? */
                    GSM_MEMCPY(&gsm.m.ipd.buff->payload[gsm.m.ipd.buff_ptr], d, len);
                    GSM_DEBUGF(GSM_CFG_DBG_IPD | GSM_DBG_TYPE_TRACE,
                        "[IPD] Bytes read: %d\r\n", (int)len);
                } else {                        /* Simply skip the data in buffer */
                    GSM_DEBUGF(GSM_CFG_DBG_IPD | GSM_DBG_TYPE_TRACE,
                        "[IPD] Bytes skipped: %d\r\n", (int)len);
                }
                d_len -= len;                   /* Decrease effective length */
                d += len;                       /* Skip remaining length */
                gsm.m.ipd.buff_ptr += len;      /* Forward buffer pointer */
                gsm.m.ipd.rem_len -= len;       /* Decrease remaining length */
            }

            /* Did we reach end of buffer or no more data? */
            if (gsm.m.ipd.rem_len == 0 || (gsm.m.ipd.buff != NULL && gsm.m.ipd.buff_ptr == gsm.m.ipd.buff->len)) {
                gsmr_t res = gsmOK;

                /* Call user callback function with received data */
                if (gsm.m.ipd.buff != NULL) {     /* Do we have valid buffer? */
                    gsm.m.ipd.conn->total_recved += gsm.m.ipd.buff->tot_len;/* Increase number of bytes received */

                    /*
                     * Send data buffer to upper layer
                     *
                     * From this moment, user is responsible for packet
                     * buffer and must free it manually
                     */
                    gsm.evt.type = GSM_EVT_CONN_RECV;
                    gsm.evt.evt.conn_data_recv.buff = gsm.m.ipd.buff;
                    gsm.evt.evt.conn_data_recv.conn = gsm.m.ipd.conn;
                    res = gsmi_send_conn_cb(gsm.m.ipd.conn, NULL);

                    gsm_pbuf_free(gsm.m.ipd.buff);  /* Free packet buffer at this point */
                    GSM_DEBUGF(GSM_CFG_DBG_IPD | GSM_DBG_TYPE_TRACE,
                        "[IPD] Free packet buffer\r\n");
                    if (res == gsmOKIGNOREMORE) {   /* We should ignore more data */
                        GSM_DEBUGF(GSM_CFG_DBG_IPD | GSM_DBG_TYPE_TRACE,
                            "[IPD] Ignoring more data from this IPD if available\r\n");
                        gsm.m.ipd.buff = NULL;  /* Set to NULL to ignore more data if possibly available */
                    }

                    /*
                     * Create new data packet if case if:
                     *
                     *  - Previous one was successful and more data to read and
                     *  - Connection is not in closing state
                     */
                    if (gsm.m.ipd.buff != NULL && gsm.m.ipd.rem_len > 0 && !gsm.m.ipd.conn->status.f.in_closing) {
                        size_t new_len = GSM_MIN(gsm.m.ipd.rem_len, GSM_CFG_IPD_MAX_BUFF_SIZE); /* Calculate new buffer length */

                        GSM_DEBUGF(GSM_CFG_DBG_IPD | GSM_DBG_TYPE_TRACE,
                            "[IPD] Allocating new packet buffer of size: %d bytes\r\n", (int)new_len);
                        gsm.m.ipd.buff = gsm_pbuf_new(new_len); /* Allocate new packet buffer */

                        GSM_DEBUGW(GSM_CFG_DBG_IPD | GSM_DBG_TYPE_TRACE | GSM_DBG_LVL_WARNING,
                            gsm.m.ipd.buff == NULL, "[IPD] Buffer allocation failed for %d bytes\r\n", (int)new_len);
                    } else {
                        gsm.m.ipd.buff = NULL;  /* Reset it */
                    }
                }
                if (gsm.m.ipd.rem_len == 0) {   /* Check if we read everything */
                    gsm.m.ipd.buff = NULL;      /* Reset buffer pointer */
                    gsm.m.ipd.read = 0;         /* Stop reading data */
                }
                gsm.m.ipd.buff_ptr = 0;         /* Reset input buffer pointer */
            }
#endif /* GSM_CFG_CONN */
        /*
         * Check if operators scan command is active
         * and if we are ready to read the incoming data
         */
        } else if (CMD_IS_CUR(GSM_CMD_COPS_GET_OPT) && gsm.msg->msg.cops_scan.read) {
            if (ch == '\n') {
                gsm.msg->msg.cops_scan.read = 0;
            } else {
                gsmi_parse_cops_scan(ch, 0);    /* Parse character by character */
            }
#if GSM_CFG_SMS
        } else if (CMD_IS_CUR(GSM_CMD_CMGR) && gsm.msg->msg.sms_read.read) {
            gsm_sms_entry_t* e = gsm.msg->msg.sms_read.entry;
            if (gsm.msg->msg.sms_read.read == 2) {  /* Read only if set to 2 */
                if (e != NULL) {                /* Check if valid entry */
                    if (e->length < (sizeof(e->data) - 1)) {
                        e->data[e->length++] = ch;
                    }
                } else {
                    gsm.msg->msg.sms_read.read = 1; /* Read but ignore data */
                }
            }
            if (ch == '\n' && ch_prev1 == '\r') {
                if (gsm.msg->msg.sms_read.read == 2) {

                }
                gsm.msg->msg.sms_read.read = 0;
            }
        } else if (CMD_IS_CUR(GSM_CMD_CMGL) && gsm.msg->msg.sms_list.read) {
            if (gsm.msg->msg.sms_list.read == 2) {
                gsm_sms_entry_t* e = &gsm.msg->msg.sms_list.entries[gsm.msg->msg.sms_list.ei];
                if (e->length < (sizeof(e->data) - 1)) {
                    e->data[e->length++] = ch;
                }
            }
            if (ch == '\n' && ch_prev1 == '\r') {
                if (gsm.msg->msg.sms_list.read == 2) {
                    gsm.msg->msg.sms_list.ei++; /* Go to next entry */
                    if (gsm.msg->msg.sms_list.er != NULL) { /* Check and update user variable */
                        *gsm.msg->msg.sms_list.er = gsm.msg->msg.sms_list.ei;
                    }
                }
                gsm.msg->msg.sms_list.read = 0;
            }
#endif /* GSM_CFG_SMS */
        /*
         * We are in command mode where we have to process byte by byte
         * Simply check for ASCII and unicode format and process data accordingly
         */
        } else {
            gsmr_t res = gsmERR;
            if (GSM_ISVALIDASCII(ch)) {         /* Manually check if valid ASCII character */
                res = gsmOK;
                unicode.t = 1;                  /* Manually set total to 1 */
                unicode.r = 0;                  /* Reset remaining bytes */
            } else if (ch >= 0x80) {            /* Process only if more than ASCII can hold */
                res = gsmi_unicode_decode(&unicode, ch);    /* Try to decode unicode format */
            }

            if (res == gsmERR) {                /* In case of an ERROR */
                unicode.r = 0;
            }
            if (res == gsmOK) {                 /* Can we process the character(s) */
                if (unicode.t == 1) {           /* Totally 1 character? */
                    switch (ch) {
                        case '\n':
                            RECV_ADD(ch);       /* Add character to input buffer */
                            gsmi_parse_received(&recv_buff);	/* Parse received string */
                            RECV_RESET();       /* Reset received string */
                            break;
                        default:
                            RECV_ADD(ch);       /* Any ASCII valid character */
                            break;
                    }

#if GSM_CFG_CONN
                    /* Check if we have to read data */
                    if (ch == '\n' && gsm.m.ipd.read) {
                        size_t len;
                        GSM_DEBUGF(GSM_CFG_DBG_IPD | GSM_DBG_TYPE_TRACE,
                            "[IPD] Data on connection %d with total size %d byte(s)\r\n",
                            (int)gsm.m.ipd.conn->num, (int)gsm.m.ipd.tot_len);

                        len = GSM_MIN(gsm.m.ipd.rem_len, GSM_CFG_IPD_MAX_BUFF_SIZE);

                        /*
                         * Read received data in case of:
                         *
                         *  - Connection is active and
                         *  - Connection is not in closing mode
                         */
                        if (gsm.m.ipd.conn->status.f.active && !gsm.m.ipd.conn->status.f.in_closing) {
                            gsm.m.ipd.buff = gsm_pbuf_new(len); /* Allocate new packet buffer */
                            GSM_DEBUGW(GSM_CFG_DBG_IPD | GSM_DBG_TYPE_TRACE | GSM_DBG_LVL_WARNING, gsm.m.ipd.buff == NULL,
                                "[IPD] Buffer allocation failed for %d byte(s)\r\n", (int)len);
                        } else {
                            gsm.m.ipd.buff = NULL;  /* Ignore reading on closed connection */
                            GSM_DEBUGF(GSM_CFG_DBG_IPD | GSM_DBG_TYPE_TRACE,
                                "[IPD] Connection %d closed or in closing, skipping %d byte(s)\r\n",
                                (int)gsm.m.ipd.conn->num, (int)len);
                        }
                        gsm.m.ipd.conn->status.f.data_received = 1; /* We have first received data */

                        gsm.m.ipd.buff_ptr = 0; /* Reset buffer write pointer */
                    }
#endif /* GSM_CFG_CONN */

                    /*
                     * Do we have a special sequence "> "?
                     *
                     * Check if any command active which may expect that kind of response
                     */
                    if (ch_prev2 == '\n' && ch_prev1 == '>' && ch == ' ') {
                        if (0) {
#if GSM_CFG_CONN
                        } else if (CMD_IS_CUR(GSM_CMD_CIPSEND)) {
                            RECV_RESET();       /* Reset received object */

                            /* Now actually send the data prepared before */
                            AT_PORT_SEND(&gsm.msg->msg.conn_send.data[gsm.msg->msg.conn_send.ptr], gsm.msg->msg.conn_send.sent);
                            gsm.msg->msg.conn_send.wait_send_ok_err = 1;    /* Now we are waiting for "SEND OK" or "SEND ERROR" */
#endif /* GSM_CFG_CONN */
#if GSM_CFG_SMS
                        } else if (CMD_IS_CUR(GSM_CMD_CMGS)) {  /* Send SMS? */
                            AT_PORT_SEND(gsm.msg->msg.sms_send.text, strlen(gsm.msg->msg.sms_send.text));
                            AT_PORT_SEND_CTRL_Z();
#endif /* GSM_CFG_SMS */
                        }
                    } else if (CMD_IS_CUR(GSM_CMD_COPS_GET_OPT)) {
                        if (RECV_LEN() > 5 && !strncmp(recv_buff.data, "+COPS:", 5)) {
                            RECV_RESET();       /* Reset incoming buffer */
                            gsmi_parse_cops_scan(0, 1); /* Reset parser state */
                            gsm.msg->msg.cops_scan.read = 1;    /* Start reading incoming bytes */
                        }
                    }
                } else {                        /* We have sequence of unicode characters */
                    /*
                     * Unicode sequence characters are not "meta" characters
                     * so it is safe to just add them to receive array without checking
                     * what are the actual values
                     */
                    for (uint8_t i = 0; i < unicode.t; i++) {
                        RECV_ADD(unicode.ch[i]);/* Add character to receive array */
                    }
                }
            } else if (res != gsmINPROG) {      /* Not in progress? */
                RECV_RESET();                   /* Invalid character in sequence */
            }
        }

        ch_prev2 = ch_prev1;                    /* Save previous character as previous previous */
        ch_prev1 = ch;                          /* Set current as previous */
    }
    return gsmOK;
}

/* Temporary macros, only available for inside gsmi_process_sub_cmd function */
/* Set new command, but first check for error on previous */
#define SET_NEW_CMD_CHECK_ERROR(new_cmd) do {   \
    if (!*(is_error)) {                         \
        n_cmd = (new_cmd);                      \
    }                                           \
} while (0)

/* Set new command, ignore result of previous */
#define SET_NEW_CMD(new_cmd) do {               \
    n_cmd = (new_cmd);                          \
} while (0)

/**
 * \brief           Process current command with known execution status and start another if necessary
 * \param[in]       msg: Pointer to current message
 * \param[in]       is_ok: pointer to status whether last command result was OK
 * \param[in]       is_error: Pointer to status whether last command result was ERROR
 * \return          \ref gsmCONT if you sent more data and we need to process more data, \ref gsmOK on success, or \ref gsmERR on error
 */
static gsmr_t
gsmi_process_sub_cmd(gsm_msg_t* msg, uint8_t* is_ok, uint16_t* is_error) {
    gsm_cmd_t n_cmd = GSM_CMD_IDLE;
    if (CMD_IS_DEF(GSM_CMD_RESET)) {
        switch (CMD_GET_CUR()) {                /* Check current command */
            case GSM_CMD_RESET: {
                gsmi_reset_everything(1);       /* Reset everything */
                SET_NEW_CMD(GSM_CFG_AT_ECHO ? GSM_CMD_ATE1 : GSM_CMD_ATE0); /* Set ECHO mode */
                gsm_delay(5000);                /* Delay for some time before we can continue after reset */
                break;
            }
            case GSM_CMD_ATE0:
            case GSM_CMD_ATE1:      SET_NEW_CMD(GSM_CMD_CFUN_SET); break;   /* Set full functionality */
            case GSM_CMD_CFUN_SET:  SET_NEW_CMD(GSM_CMD_CMEE_SET); break;   /* Set detailed error reporting */
            case GSM_CMD_CMEE_SET:  SET_NEW_CMD(GSM_CMD_CGMI_GET); break;   /* Get manufacturer */
            case GSM_CMD_CGMI_GET:  SET_NEW_CMD(GSM_CMD_CGMM_GET); break;   /* Get model */
            case GSM_CMD_CGMM_GET:  SET_NEW_CMD(GSM_CMD_CGSN_GET); break;   /* Get product serial number */
            case GSM_CMD_CGSN_GET:  SET_NEW_CMD(GSM_CMD_CGMR_GET); break;   /* Get product revision */
            case GSM_CMD_CGMR_GET: {
                /*
                 * At this point we have modem info.
                 * It is now time to send info to user
                 * to select between device drivers
                 */
                gsmi_send_cb(GSM_EVT_DEVICE_IDENTIFIED);

                SET_NEW_CMD(GSM_CMD_CREG_SET);      /* Enable unsolicited code for CREG */
                break;
            }
            case GSM_CMD_CREG_SET: SET_NEW_CMD(GSM_CMD_CLCC_SET); break;/* Set call state */
            case GSM_CMD_CLCC_SET: SET_NEW_CMD(GSM_CMD_CPIN_GET); break;/* Get SIM state */
            case GSM_CMD_CPIN_GET: break;
            default: break;
        }

        /* Send event */
        if (n_cmd == GSM_CMD_IDLE) {
            RESET_SEND_EVT(msg, gsmOK);
        }
    } else if (CMD_IS_DEF(GSM_CMD_COPS_GET)) {
        if (CMD_IS_CUR(GSM_CMD_COPS_GET)) {
            gsm.evt.evt.operator_current.operator_current = &gsm.m.network.curr_operator;
            gsmi_send_cb(GSM_EVT_NETWORK_OPERATOR_CURRENT);
        }
#if GSM_CFG_SMS
    } else if (CMD_IS_DEF(GSM_CMD_SMS_ENABLE)) {
        switch (CMD_GET_CUR()) {
            case GSM_CMD_CPMS_GET_OPT: SET_NEW_CMD(GSM_CMD_CPMS_GET); break;
            case GSM_CMD_CPMS_GET: break;
            default: break;
        }
        if (!*is_ok || n_cmd == GSM_CMD_IDLE) { /* Stop execution on any command */
            SET_NEW_CMD(GSM_CMD_IDLE);
            gsm.m.sms.enabled = n_cmd == GSM_CMD_IDLE;  /* Set enabled status */
            gsm.evt.evt.sms_enable.status = gsm.m.sms.enabled ? gsmOK : gsmERR;
            gsmi_send_cb(GSM_EVT_SMS_ENABLE);   /* Send to user */
        }
    } else if (CMD_IS_DEF(GSM_CMD_CMGS)) {      /* Send SMS default command */
        if (CMD_IS_CUR(GSM_CMD_CMGF) && *is_ok) {   /* Set message format current command */
            SET_NEW_CMD(GSM_CMD_CMGS);          /* Now send actual message */
        }

        /* Send event on finish */
        if (n_cmd == GSM_CMD_IDLE) {
            SMS_SEND_SEND_EVT(gsm.msg, *is_ok ? gsmOK : gsmERR);
        }
    } else if (CMD_IS_DEF(GSM_CMD_CMGR)) {      /* Read SMS message */
        if (CMD_IS_CUR(GSM_CMD_CPMS_GET) && *is_ok) {
            SET_NEW_CMD(GSM_CMD_CPMS_SET);      /* Set memory */
        } else if (CMD_IS_CUR(GSM_CMD_CPMS_SET) && *is_ok) {
            SET_NEW_CMD(GSM_CMD_CMGF);          /* Set text mode */
        } else if (CMD_IS_CUR(GSM_CMD_CMGF) && *is_ok) {/* Set message format current command*/
            SET_NEW_CMD(GSM_CMD_CMGR);          /* Start message read */
        } else if (CMD_IS_CUR(GSM_CMD_CMGR) && *is_ok) {
            msg->msg.sms_read.mem = gsm.m.sms.mem[0].current;   /* Set current memory */
        }

        /* Send event on finish */
        if (n_cmd == GSM_CMD_IDLE) {
            SMS_SEND_READ_EVT(gsm.msg, *is_ok ? gsmOK : gsmERR);
        }
    } else if (CMD_IS_DEF(GSM_CMD_CMGD)) {      /* Delete SMS message*/
        if (CMD_IS_CUR(GSM_CMD_CPMS_GET) && *is_ok) {
            SET_NEW_CMD(GSM_CMD_CPMS_SET);      /* Set memory */
        } else if (CMD_IS_CUR(GSM_CMD_CPMS_SET) && *is_ok) {
            SET_NEW_CMD(GSM_CMD_CMGD);          /* Delete message */
        }

        /* Send event on finish */
        if (n_cmd == GSM_CMD_IDLE) {
            SMS_SEND_DELETE_EVT(msg, *is_ok ? gsmOK : gsmERR);
        }
    } else if (CMD_IS_DEF(GSM_CMD_CMGDA)) {
        if (CMD_IS_CUR(GSM_CMD_CMGF) && *is_ok) {
            SET_NEW_CMD(GSM_CMD_CMGDA);         /* Mass storage */
        }
    } else if (CMD_IS_DEF(GSM_CMD_CMGL)) {      /* List SMS messages */
        if (CMD_IS_CUR(GSM_CMD_CPMS_GET) && *is_ok) {
            SET_NEW_CMD(GSM_CMD_CPMS_SET);      /* Set memory */
        } else if (CMD_IS_CUR(GSM_CMD_CPMS_SET) && *is_ok) {
            SET_NEW_CMD(GSM_CMD_CMGF);          /* Set text format */
        } else if (CMD_IS_CUR(GSM_CMD_CMGF) && *is_ok) {
            SET_NEW_CMD(GSM_CMD_CMGL);          /* List messages */
        }

        /* Send event on finish */
        if (n_cmd == GSM_CMD_IDLE) {
            SMS_SEND_LIST_EVT(msg, *is_ok ? gsmOK : gsmERR);
        }
    } else if (CMD_IS_DEF(GSM_CMD_CPMS_SET)) {  /* Set preferred memory */
        if (CMD_IS_CUR(GSM_CMD_CPMS_GET) && *is_ok) {
            SET_NEW_CMD(GSM_CMD_CPMS_SET);      /* Now set the command */
        }
#endif /* GSM_CFG_SMS */
    } else if (CMD_IS_DEF(GSM_CMD_SIM_PROCESS_BASIC_CMDS)) {
        switch (CMD_GET_CUR()) {
            case GSM_CMD_CNUM: {                /* Get own phone number */
                if (!*is_ok) {
                    /* Sometimes SIM is not ready just after PIN entered */
                    if (msg->msg.sim_info.cnum_tries < 5) {
                        msg->msg.sim_info.cnum_tries++;
                        SET_NEW_CMD(GSM_CMD_CNUM);
                        gsm_delay(1000);
                    }
                }
            }
            default: break;
        }
    } else if (CMD_IS_DEF(GSM_CMD_CPIN_SET)) {  /* Set PIN code */
        switch (CMD_GET_CUR()) {
            case GSM_CMD_CPIN_GET: {            /* Get own phone number */
                if (*is_ok && gsm.m.sim.state == GSM_SIM_STATE_PIN) {
                    SET_NEW_CMD(GSM_CMD_CPIN_SET);  /* Set command to write PIN */
                } else if (gsm.m.sim.state != GSM_SIM_STATE_READY) {
                    *is_ok = 0;
                    *is_error = 1;
                }
                break;
            }
            case GSM_CMD_CPIN_SET: {            /* Set CPIN */
                if (*is_ok) {
                    gsm_delay(5000);            /* Make delay to make sure SIM is ready */
                }
                break;
            }
            default:
                break;
        }
#if GSM_CFG_CALL
    } else if (CMD_IS_DEF(GSM_CMD_CALL_ENABLE)) {
        gsm.m.call.enabled = *is_ok;            /* Set enabled status */
        gsm.evt.evt.call_enable.res = gsm.m.call.enabled ? gsmOK : gsmERR;
        gsmi_send_cb(GSM_EVT_CALL_ENABLE);      /* Send to user */
#endif /* GSM_CFG_CALL */
#if GSM_CFG_PHONEBOOK
    } else if (CMD_IS_DEF(GSM_CMD_PHONEBOOK_ENABLE)) {
        gsm.m.pb.enabled = *is_ok;              /* Set enabled status */
        gsm.evt.evt.pb_enable.res = gsm.m.pb.enabled ? gsmOK : gsmERR;
        gsmi_send_cb(GSM_EVT_PB_ENABLE);        /* Send to user */
    } else if (CMD_IS_DEF(GSM_CMD_CPBW_SET)) {  /* Write phonebook entry */
        if (CMD_IS_CUR(GSM_CMD_CPBS_GET) && *is_ok) {   /* Get current memory */
            SET_NEW_CMD(GSM_CMD_CPBS_SET);      /* Set current memory */
        } else if (CMD_IS_CUR(GSM_CMD_CPBS_SET) && *is_ok) {
            SET_NEW_CMD(GSM_CMD_CPBW_SET);      /* Write entry to phonebook */
        }
    } else if (CMD_IS_DEF(GSM_CMD_CPBR)) {
        if (CMD_IS_CUR(GSM_CMD_CPBS_GET) && *is_ok) {/* Get current memory */
            SET_NEW_CMD(GSM_CMD_CPBS_SET);      /* Set current memory */
        } else if (CMD_IS_CUR(GSM_CMD_CPBS_SET) && *is_ok) {
            SET_NEW_CMD(GSM_CMD_CPBR);          /* Read entries */
        } else if (CMD_IS_CUR(GSM_CMD_CPBR)) {
            gsm.evt.evt.pb_list.mem = gsm.m.pb.mem.current;
            gsm.evt.evt.pb_list.entries = gsm.msg->msg.pb_list.entries;
            gsm.evt.evt.pb_list.size = gsm.msg->msg.pb_list.ei;
            gsm.evt.evt.pb_list.res = *is_ok ? gsmOK : gsmERR;
            gsmi_send_cb(GSM_EVT_PB_LIST);
        }
    } else if (CMD_IS_DEF(GSM_CMD_CPBF)) {
        if (CMD_IS_CUR(GSM_CMD_CPBS_GET) && *is_ok) {/* Get current memory */
            SET_NEW_CMD(GSM_CMD_CPBS_SET);      /* Set current memory */
        } else if (CMD_IS_CUR(GSM_CMD_CPBS_SET) && *is_ok) {
            SET_NEW_CMD(GSM_CMD_CPBF);          /* Read entries */
        } else if (CMD_IS_CUR(GSM_CMD_CPBF)) {
            gsm.evt.evt.pb_search.mem = gsm.m.pb.mem.current;
            gsm.evt.evt.pb_search.search = gsm.msg->msg.pb_search.search;
            gsm.evt.evt.pb_search.entries = gsm.msg->msg.pb_search.entries;
            gsm.evt.evt.pb_search.size = gsm.msg->msg.pb_search.ei;
            gsm.evt.evt.pb_search.res = *is_ok ? gsmOK : gsmERR;
            gsmi_send_cb(GSM_EVT_PB_SEARCH);
        }
#endif /* GSM_CFG_PHONEBOOK */
#if GSM_CFG_NETWORK
    } if (CMD_IS_DEF(GSM_CMD_NETWORK_ATTACH)) {
        switch (msg->i) {
            case 0: SET_NEW_CMD_CHECK_ERROR(GSM_CMD_CGACT_SET_0); break;
            case 1: SET_NEW_CMD(GSM_CMD_CGACT_SET_1); break;
#if GSM_CFG_NETWORK_IGNORE_CGACT_RESULT
            case 2: SET_NEW_CMD(GSM_CMD_CGATT_SET_0); break;
#else /* GSM_CFG_NETWORK_IGNORE_CGACT_RESULT */
            case 2: SET_NEW_CMD_CHECK_ERROR(GSM_CMD_CGATT_SET_0); break;
#endif /* !GSM_CFG_NETWORK_IGNORE_CGACT_RESULT */
            case 3: SET_NEW_CMD(GSM_CMD_CGATT_SET_1); break;
            case 4: SET_NEW_CMD_CHECK_ERROR(GSM_CMD_CIPSHUT); break;
            case 5: SET_NEW_CMD_CHECK_ERROR(GSM_CMD_CIPMUX_SET); break;
            case 6: SET_NEW_CMD_CHECK_ERROR(GSM_CMD_CIPRXGET_SET); break;
            case 7: SET_NEW_CMD_CHECK_ERROR(GSM_CMD_CSTT_SET); break;
            case 8: SET_NEW_CMD_CHECK_ERROR(GSM_CMD_CIICR); break;
            case 9: SET_NEW_CMD_CHECK_ERROR(GSM_CMD_CIFSR); break;
            case 10: SET_NEW_CMD(GSM_CMD_CIPSTATUS); break;
            default: break;
        }
    } else if (CMD_IS_DEF(GSM_CMD_NETWORK_DETACH)) {
        switch (msg->i) {
            case 0: SET_NEW_CMD(GSM_CMD_CGATT_SET_0); break;
            case 1: SET_NEW_CMD(GSM_CMD_CGACT_SET_0); break;
#if GSM_CFG_CONN
            case 2: SET_NEW_CMD(GSM_CMD_CIPSTATUS); break;
#endif /* GSM_CFG_CONN */
            default: break;
        }
        if (!n_cmd) {
            *is_ok = 1;
        }
#endif /* GSM_CFG_NETWORK */
#if GSM_CFG_CONN
    } else if (CMD_IS_DEF(GSM_CMD_CIPSTART)) {
        if (!msg->i && CMD_IS_CUR(GSM_CMD_CIPSTATUS)) { /* Was the current command status info? */
            if (*is_ok) {
                SET_NEW_CMD(GSM_CMD_CIPSSL);    /* Set SSL */
            }
        } else if (msg->i == 1 && CMD_IS_CUR(GSM_CMD_CIPSSL)) {
            SET_NEW_CMD(GSM_CMD_CIPSTART);      /* Now actually start connection */
        } else if (msg->i == 2 && CMD_IS_CUR(GSM_CMD_CIPSTART)) {
            SET_NEW_CMD(GSM_CMD_CIPSTATUS);     /* Go to status mode */
            if (*is_error) {
                msg->msg.conn_start.conn_res = GSM_CONN_CONNECT_ERROR;
            }
        } else if (msg->i == 3 && CMD_IS_CUR(GSM_CMD_CIPSTATUS)) {
            /* After second CIP status, define what to do next */
            switch (msg->msg.conn_start.conn_res) {
                case GSM_CONN_CONNECT_OK: {     /* Successfully connected */
                    gsm_conn_t* conn = &gsm.m.conns[msg->msg.conn_start.num];   /* Get connection number */

                    gsm.evt.type = GSM_EVT_CONN_ACTIVE; /* Connection just active */
                    gsm.evt.evt.conn_active_closed.client = 1;
                    gsm.evt.evt.conn_active_closed.conn = conn;
                    gsm.evt.evt.conn_active_closed.forced = 1;
                    gsmi_send_conn_cb(conn, NULL);
                    gsmi_conn_start_timeout(conn);  /* Start connection timeout timer */
                    break;
                }
                case GSM_CONN_CONNECT_ERROR: {  /* Connection error */
                    gsmi_send_conn_error_cb(msg, gsmERRCONNFAIL);
                    *is_error = 1;              /* Manually set error */
                    *is_ok = 0;                 /* Reset success */
                    break;
                }
                default: {
                    /* Do nothing as of now */
                    break;
                }
            }
        }
#endif /* GSM_CFG_CONN */
    }

    /* Check if new command was set for execution */
    if (n_cmd != GSM_CMD_IDLE) {
        gsmr_t res;
        msg->cmd = n_cmd;
        if ((res = msg->fn(msg)) == gsmOK) {
            return gsmCONT;
        } else {
            *is_ok = 0;
            *is_error = 1;
            return res;
        }
    } else {
        msg->cmd = GSM_CMD_IDLE;
    }
    return *is_ok ? gsmOK : gsmERR;
}

/**
 * \brief           Function to initialize every AT command
 * \note            Never call this function directly. Set as initialization function for command and use `msg->fn(msg)`
 * \param[in]       msg: Pointer to \ref gsm_msg_t with data
 * \return          Member of \ref gsmr_t enumeration
 */
gsmr_t
gsmi_initiate_cmd(gsm_msg_t* msg) {
    switch (CMD_GET_CUR()) {                    /* Check current message we want to send over AT */
        case GSM_CMD_RESET: {                   /* Reset modem with AT commands */
            /* Try with hardware reset */
            if (gsm.ll.reset_fn != NULL && gsm.ll.reset_fn(1)) {
                gsm_delay(2);
                gsm.ll.reset_fn(0);
                gsm_delay(500);
            }

            /* Send manual AT command */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CFUN=1,1");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_RESET_DEVICE_FIRST_CMD: {  /* First command for device driver specific reset */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_ATE0:
        case GSM_CMD_ATE1: {
            AT_PORT_SEND_BEGIN();
            if (CMD_IS_CUR(GSM_CMD_ATE0)) {
                AT_PORT_SEND_CONST_STR("E0");
            } else {
                AT_PORT_SEND_CONST_STR("E1");
            }
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CMEE_SET: {                /* Enable detailed error messages */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CMEE=1");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CLCC_SET: {                /* Enable detailed call info */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CLCC=1");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CGMI_GET: {                /* Get manufacturer */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CGMI");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CGMM_GET: {                /* Get model */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CGMM");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CGSN_GET: {                /* Get serial number */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CGSN");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CGMR_GET: {                /* Get revision */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CGMR");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CREG_SET: {                /* Enable +CREG message */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CREG=1");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CREG_GET: {                /* Get network registration status */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CREG?");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CFUN_SET: {
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CFUN=");
            /**
             * \todo: If CFUN command forced, check value
             */
            if (CMD_IS_DEF(GSM_CMD_RESET)
                || (CMD_IS_DEF(GSM_CMD_CFUN_SET) && msg->msg.cfun.mode)) {
                AT_PORT_SEND_CONST_STR("1");
            } else {
                AT_PORT_SEND_CONST_STR("0");
            }
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CPIN_GET: {                /* Read current SIM status */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CPIN?");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CPIN_SET: {                /* Set SIM pin code */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CPIN=");
            gsmi_send_string(msg->msg.cpin_enter.pin, 0, 1, 0);
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CPIN_ADD: {                /* Add new pin code */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CLCK=\"SC\",1,");
            gsmi_send_string(msg->msg.cpin_add.pin, 0, 1, 0);
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CPIN_CHANGE: {             /* Change already active SIM */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CPWD=\"SC\"");
            gsmi_send_string(msg->msg.cpin_change.current_pin, 0, 1, 1);
            gsmi_send_string(msg->msg.cpin_change.new_pin, 0, 1, 1);
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CPIN_REMOVE: {             /* Remove current PIN */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CLCK=\"SC\",0,");
            gsmi_send_string(msg->msg.cpin_remove.pin, 0, 1, 0);
            AT_PORT_SEND_END();
            break;
        }
        case GMM_CMD_CPUK_SET: {                /* Enter PUK and set new PIN */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CPIN=");
            gsmi_send_string(msg->msg.cpuk_enter.puk, 0, 1, 0);
            gsmi_send_string(msg->msg.cpuk_enter.pin, 0, 1, 1);
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_COPS_SET: {                /* Set current operator */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+COPS=");
            gsmi_send_number(GSM_U32(msg->msg.cops_set.mode), 0, 0);
            if (msg->msg.cops_set.mode != GSM_OPERATOR_MODE_AUTO) {
                gsmi_send_number(GSM_U32(msg->msg.cops_set.format), 0, 1);
                switch (msg->msg.cops_set.format) {
                    case GSM_OPERATOR_FORMAT_LONG_NAME:
                    case GSM_OPERATOR_FORMAT_SHORT_NAME:
                        gsmi_send_string(msg->msg.cops_set.name, 1, 1, 1);
                        break;
                    default:
                        gsmi_send_number(GSM_U32(msg->msg.cops_set.num), 0, 1);
                }
            }
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_COPS_GET: {                /* Get current operator */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+COPS?");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_COPS_GET_OPT: {            /* Get list of available operators */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+COPS=?");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CSQ_GET: {                 /* Get signal strength */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CSQ");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CNUM: {                    /* Get SIM number */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CNUM");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CIPSHUT: {                 /* Shut down network connection and put to reset state */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CIPSHUT");
            AT_PORT_SEND_END();
            break;
        }
#if GSM_CFG_CONN
        case GSM_CMD_CIPMUX: {                  /* Enable multiple connections */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CIPMUX=1");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CIPHEAD: {                 /* Enable information on receive data about connection and length */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CIPHEAD=1");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CIPSRIP: {
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CIPSRIP=1");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CIPSSL: {                  /* Set SSL configuration */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CIPSSL=");
            gsmi_send_number((msg->msg.conn_start.type == GSM_CONN_TYPE_SSL) ? 1 : 0, 0, 0);
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CIPSTART: {                /* Start a new connection */
            gsm_conn_t* c = NULL;

            /* Do we have network connection? */
            /* Check if we are connected to network */

            msg->msg.conn_start.num = 0;        /* Start with max value = invalidated */
            for (int16_t i = GSM_CFG_MAX_CONNS - 1; i >= 0; i--) {  /* Find available connection */
                if (!gsm.m.conns[i].status.f.active) {
                    c = &gsm.m.conns[i];
                    c->num = GSM_U8(i);
                    msg->msg.conn_start.num = GSM_U8(i);    /* Set connection number for message structure */
                    break;
                }
            }
            if (c == NULL) {
                gsmi_send_conn_error_cb(msg, gsmERRNOFREECONN);
                return gsmERRNOFREECONN;        /* We don't have available connection */
            }

            if (msg->msg.conn_start.conn != NULL) { /* Is user interested about connection info? */
                *msg->msg.conn_start.conn = c;  /* Save connection for user */
            }

            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CIPSTART=");
            gsmi_send_number(GSM_U32(c->num), 0, 0);
            if (msg->msg.conn_start.type == GSM_CONN_TYPE_TCP) {
                gsmi_send_string("TCP", 0, 1, 1);
            } else if (msg->msg.conn_start.type == GSM_CONN_TYPE_UDP) {
                gsmi_send_string("UDP", 0, 1, 1);
            }
            gsmi_send_string(msg->msg.conn_start.host, 0, 1, 1);
            gsmi_send_port(msg->msg.conn_start.port, 0, 1);
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CIPCLOSE: {          /* Close the connection */
            gsm_conn_p c = msg->msg.conn_close.conn;
            if (c != NULL &&
                /* Is connection already closed or command for this connection is not valid anymore? */
                (!gsm_conn_is_active(c) || c->val_id != msg->msg.conn_close.val_id)) {
                return gsmERR;
            }
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CIPCLOSE=");
            gsmi_send_number(GSM_U32(msg->msg.conn_close.conn ? msg->msg.conn_close.conn->num : GSM_CFG_MAX_CONNS), 0, 0);
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CIPSEND: {                 /* Send data to connection */
            return gsmi_tcpip_process_send_data();  /* Process send data */
        }
        case GSM_CMD_CIPSTATUS: {               /* Get status of device and all connections */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CIPSTATUS");
            AT_PORT_SEND_END();
            break;
        }
#endif /* GSM_CFG_CONN */
#if GSM_CFG_SMS
        case GSM_CMD_CMGF: {                    /* Select SMS message format */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CMGF=");
            if (CMD_IS_DEF(GSM_CMD_CMGS)) {
                gsmi_send_number(GSM_U32(!!msg->msg.sms_send.format), 0, 0);
            } else if (CMD_IS_DEF(GSM_CMD_CMGR)) {
                gsmi_send_number(GSM_U32(!!msg->msg.sms_read.format), 0, 0);
            } else if (CMD_IS_DEF(GSM_CMD_CMGL)) {
                gsmi_send_number(GSM_U32(!!msg->msg.sms_list.format), 0, 0);
            } else {
                /* Used for all other operations like delete all messages, etc */
                AT_PORT_SEND_CONST_STR("1");
            }
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CMGS: {                    /* Send SMS */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CMGS=");
            gsmi_send_string(msg->msg.sms_send.num, 0, 1, 0);
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CMGR: {                    /* Read message */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CMGR=");
            gsmi_send_number(GSM_U32(msg->msg.sms_read.pos), 0, 0);
            gsmi_send_number(GSM_U32(!msg->msg.sms_read.update), 0, 1);
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CMGD: {                    /* Delete SMS message */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CMGD=");
            gsmi_send_number(GSM_U32(msg->msg.sms_delete.pos), 0, 0);
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CMGDA: {                   /* Mass delete SMS messages */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CMGDA=");
            switch (msg->msg.sms_delete_all.status) {
                case GSM_SMS_STATUS_READ:   gsmi_send_string("DEL READ", 0, 1, 0); break;
                case GSM_SMS_STATUS_UNREAD: gsmi_send_string("DEL UNREAD", 0, 1, 0); break;
                case GSM_SMS_STATUS_SENT:   gsmi_send_string("DEL SENT", 0, 1, 0); break;
                case GSM_SMS_STATUS_UNSENT: gsmi_send_string("DEL UNSENT", 0, 1, 0); break;
                case GSM_SMS_STATUS_INBOX:  gsmi_send_string("DEL INBOX", 0, 1, 0); break;
                case GSM_SMS_STATUS_ALL:    gsmi_send_string("DEL ALL", 0, 1, 0); break;
                default: break;
            }
            AT_PORT_SEND_END();
            break;
        }
                           
        case GSM_CMD_CMGL: {                    /* Delete SMS message */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CMGL=");
            gsmi_send_sms_stat(msg->msg.sms_list.status, 1, 0);
            gsmi_send_number(GSM_U32(!msg->msg.sms_list.update), 0, 1);
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CPMS_GET_OPT: {            /* Get available SMS storages */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CPMS=?");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CPMS_GET: {                /* Get current SMS storage info */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CPMS?");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CPMS_SET: {                /* Set active SMS storage(s) */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CPMS=");
            if (CMD_IS_DEF(GSM_CMD_CMGR)) { /* Read SMS original command? */
                gsmi_send_dev_memory(msg->msg.sms_read.mem == GSM_MEM_CURRENT ? gsm.m.sms.mem[0].current : msg->msg.sms_read.mem, 1, 0);
            } else if(CMD_IS_DEF(GSM_CMD_CMGD)) {   /* Delete SMS original command? */
                gsmi_send_dev_memory(msg->msg.sms_delete.mem == GSM_MEM_CURRENT ? gsm.m.sms.mem[0].current : msg->msg.sms_delete.mem, 1, 0);
            } else if(CMD_IS_DEF(GSM_CMD_CMGL)) {   /* List SMS original command? */
                gsmi_send_dev_memory(msg->msg.sms_list.mem == GSM_MEM_CURRENT ? gsm.m.sms.mem[0].current : msg->msg.sms_list.mem, 1, 0);
            } else if (CMD_IS_DEF(GSM_CMD_CPMS_SET)) {  /* Do we want to set memory for read/delete,sent/write,receive? */
                for (size_t i = 0; i < 3; i++) {/* Write 3 memories */
                    gsmi_send_dev_memory(msg->msg.sms_memory.mem[i] == GSM_MEM_CURRENT ? gsm.m.sms.mem[i].current : msg->msg.sms_memory.mem[i], 1, !!i);
                }
            }
            AT_PORT_SEND_END();
            break;
        }
#endif /* GSM_CFG_SMS */
#if GSM_CFG_CALL
        case GSM_CMD_ATD: {                     /* Start new call */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("D");
            gsmi_send_string(msg->msg.call_start.number, 0, 0, 0);
            AT_PORT_SEND_CONST_STR(";");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_ATA: {                     /* Answer phone call */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("A");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_ATH: {                     /* Disconnect existing connection (hang-up phone call) */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("H");
            AT_PORT_SEND_END();
            break;
        }
#endif /* GSM_CFG_CALL */
#if GSM_CFG_PHONEBOOK
        case GSM_CMD_CPBS_GET_OPT: {            /* Get available phonebook storages */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CPBS=?");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CPBS_GET: {                /* Get current memory info */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CPBS?");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CPBS_SET: {                /* Get current memory info */
            gsm_mem_t mem;
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CPBS=");
            switch (CMD_GET_DEF()) {
                case GSM_CMD_CPBW_SET: mem = msg->msg.pb_write.mem; break;
                case GSM_CMD_CPBR: mem = msg->msg.pb_list.mem; break;
                case GSM_CMD_CPBF: mem = msg->msg.pb_search.mem; break;
                default: break;
            }
            gsmi_send_dev_memory(mem == GSM_MEM_CURRENT ? gsm.m.pb.mem.current : mem, 1, 0);
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CPBW_SET: {                /* Write/Delete new/old entry */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CPBW=");
            if (msg->msg.pb_write.pos > 0) {    /* Write number if more than 0 */
                gsmi_send_number(GSM_U32(msg->msg.pb_write.pos), 0, 0);
            }
            if (!msg->msg.pb_write.del) {
                gsmi_send_string(msg->msg.pb_write.num, 0, 1, 1);
                gsmi_send_number(GSM_U32(msg->msg.pb_write.type), 0, 1);
                gsmi_send_string(msg->msg.pb_write.name, 0, 1, 1);
            }
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CPBR: {                    /* Read entires */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CPBR=");
            gsmi_send_number(GSM_U32(msg->msg.pb_list.start_index), 0, 0);
            gsmi_send_number(GSM_U32(msg->msg.pb_list.etr), 0, 1);
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CPBF: {                    /* Find entires */
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CPBF=");
            gsmi_send_string(msg->msg.pb_search.search, 1, 1, 0);
            AT_PORT_SEND_END();
            break;
        }
#endif /* GSM_CFG_PHONEBOOK */
#if GSM_CFG_NETWORK
        case GSM_CMD_NETWORK_ATTACH:
        case GSM_CMD_CGACT_SET_0: {
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CGACT=0");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CGACT_SET_1: {
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CGACT=1");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_NETWORK_DETACH:
        case GSM_CMD_CGATT_SET_0: {
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CGATT=0");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CGATT_SET_1: {
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CGATT=1");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CIPMUX_SET: {
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CIPMUX=1");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CIPRXGET_SET: {
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CIPRXGET=0");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CSTT_SET: {
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CSTT=");
            gsmi_send_string(msg->msg.network_attach.apn, 1, 1, 0);
            gsmi_send_string(msg->msg.network_attach.user, 1, 1, 1);
            gsmi_send_string(msg->msg.network_attach.pass, 1, 1, 1);
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CIICR: {
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CIICR");
            AT_PORT_SEND_END();
            break;
        }
        case GSM_CMD_CIFSR: {
            AT_PORT_SEND_BEGIN();
            AT_PORT_SEND_CONST_STR("+CIFSR");
            AT_PORT_SEND_END();
            break;
        }
#endif /* GSM_CFG_NETWORK */
        default:
            return gsmERR;                      /* Invalid command */
    }
    return gsmOK;                               /* Valid command */
}

/**
 * \brief           Send message from API function to producer queue for further processing
 * \param[in]       msg: New message to process
 * \param[in]       process_fn: callback function used to process message
 * \param[in]       max_block_time: Maximal time command can block in units of milliseconds
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsmi_send_msg_to_producer_mbox(gsm_msg_t* msg, gsmr_t (*process_fn)(gsm_msg_t *), uint32_t max_block_time) {
    gsmr_t res = msg->res = gsmOK;

    /* Check here if stack is even enabled or shall we disable new command entry? */
    gsm_core_lock();
    /* If locked more than 1 time, means we were called from callback or internally */
    if (gsm.locked_cnt > 1 && msg->is_blocking) {
        res = gsmERRBLOCKING;                   /* Blocking mode not allowed */
    }
    /* Check if device present */
    if (res == gsmOK && !gsm.status.f.dev_present) {
        res = gsmERRNODEVICE;                   /* No device connected */
    }
    gsm_core_unlock();
    if (res != gsmOK) {
        GSM_MSG_VAR_FREE(msg);                  /* Free memory and return */
        return res;
    }

    if (msg->is_blocking) {                     /* In case message is blocking */
        if (!gsm_sys_sem_create(&msg->sem, 0)) {/* Create semaphore and lock it immediatelly */
            GSM_MSG_VAR_FREE(msg);              /* Release memory and return */
            return gsmERRMEM;
        }
    }
    if (!msg->cmd) {                            /* Set start command if not set by user */
        msg->cmd = msg->cmd_def;                /* Set it as default */
    }
    msg->block_time = max_block_time;           /* Set blocking status if necessary */
    msg->fn = process_fn;                       /* Save processing function to be called as callback */
    if (msg->is_blocking) {
        gsm_sys_mbox_put(&gsm.mbox_producer, msg);  /* Write message to producer queue and wait forever */
    } else {
        if (!gsm_sys_mbox_putnow(&gsm.mbox_producer, msg)) {    /* Write message to producer queue immediatelly */
            GSM_MSG_VAR_FREE(msg);              /* Release message */
            return gsmERRMEM;
        }
    }
    if (res == gsmOK && msg->is_blocking) {     /* In case we have blocking request */
        uint32_t time;
        time = gsm_sys_sem_wait(&msg->sem, 0);  /* Wait forever for semaphore */
        if (time == GSM_SYS_TIMEOUT) {          /* If semaphore was not accessed in given time */
            res = gsmTIMEOUT;                   /* Semaphore not released in time */
        } else {
            res = msg->res;                     /* Set response status from message response */
        }
        GSM_MSG_VAR_FREE(msg);                  /* Release message */
    }
    return res;
}

/**
 * \brief           Process events in case of timeout on command or invalid message (if device is not present)
 *
 *                  Function is called from processing thread:
 *
 *                      - On command timeout error
 *                      - If command was sent to queue and before processed, device present status changed
 *
 * \param[in]       msg: Current message
 * \param[in]       err: Error message to send
 */
void
gsmi_process_events_for_timeout_or_error(gsm_msg_t* msg, gsmr_t err) {
    switch (msg->cmd_def) {
        case GSM_CMD_RESET: {
            /* Reset command error */
            RESET_SEND_EVT(msg, err);
            break;
        }

        case GSM_CMD_RESTORE: {
            /* Restore command error */
            RESTORE_SEND_EVT(msg, err);
            break;
        }

#if GSM_CFG_CONN
        case GSM_CMD_CIPSTART: {
            /* Start connection error */
            gsmi_send_conn_error_cb(msg, err);
            break;
        }

        case GSM_CMD_CIPSEND: {
            /* Send data error event */
            CONN_SEND_DATA_SEND_EVT(msg, err);
            break;
        }
#endif /* GSM_CFG_CONN */

#if GSM_CFG_SMS
        case GSM_CMD_CMGS: {
            /* Send error event */
            SMS_SEND_SEND_EVT(msg, err);
            break;
        }

        case GSM_CMD_CMGR: {
            /* Read error event */
            SMS_SEND_READ_EVT(msg, err);
            break;
        }

        case GSM_CMD_CMGL: {
            /* List error event */
            SMS_SEND_LIST_EVT(msg, err);
            break;
        }

        case GSM_CMD_CMGD: {
            /* Delete error event */
            SMS_SEND_DELETE_EVT(msg, err);
            break;
        }
#endif /* GSM_CFG_SMS */

        default: break;
    }
}
