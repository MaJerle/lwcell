/**
 * \file            lwgsm_int.c
 * \brief           Internal functions
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
#include "lwgsm/lwgsm.h"
#include "lwgsm/lwgsm_int.h"
#include "lwgsm/lwgsm_mem.h"
#include "lwgsm/lwgsm_parser.h"
#include "lwgsm/lwgsm_unicode.h"
#include "system/lwgsm_ll.h"

#if !__DOXYGEN__
/**
 * \brief           Receive character structure to handle full line terminated with `\n` character
 */
typedef struct {
    char data[128];                             /*!< Received characters */
    size_t len;                                 /*!< Length of valid characters */
} lwgsm_recv_t;

/* Receive character macros */
#define RECV_ADD(ch)                        do { if (recv_buff.len < (sizeof(recv_buff.data)) - 1) { recv_buff.data[recv_buff.len++] = ch; recv_buff.data[recv_buff.len] = 0; } } while (0)
#define RECV_RESET()                        do { recv_buff.len = 0; recv_buff.data[0] = 0; } while (0)
#define RECV_LEN()                          ((size_t)recv_buff.len)
#define RECV_IDX(index)                     recv_buff.data[index]

/* Send data over AT port */
#define AT_PORT_SEND_STR(str)               lwgsm.ll.send_fn((const void *)(str), (size_t)strlen(str))
#define AT_PORT_SEND_CONST_STR(str)         lwgsm.ll.send_fn((const void *)(str), (size_t)(sizeof(str) - 1))
#define AT_PORT_SEND_CHR(ch)                lwgsm.ll.send_fn((const void *)(ch), (size_t)1)
#define AT_PORT_SEND_FLUSH()                lwgsm.ll.send_fn(NULL, 0)
#define AT_PORT_SEND(d, l)                  lwgsm.ll.send_fn((const void *)(d), (size_t)(l))
#define AT_PORT_SEND_WITH_FLUSH(d, l)       do { AT_PORT_SEND((d), (l)); AT_PORT_SEND_FLUSH(); } while (0)

/* Beginning and end of every AT command */
#define AT_PORT_SEND_BEGIN_AT()             do { AT_PORT_SEND_CONST_STR("AT"); } while (0)
#define AT_PORT_SEND_END_AT()               do { AT_PORT_SEND(CRLF, CRLF_LEN); AT_PORT_SEND(NULL, 0); } while (0)

/* Send special characters over AT port with condition */
#define AT_PORT_SEND_QUOTE_COND(q)          do { if ((q)) { AT_PORT_SEND_CONST_STR("\""); } } while (0)
#define AT_PORT_SEND_COMMA_COND(c)          do { if ((c)) { AT_PORT_SEND_CONST_STR(","); } } while (0)
#define AT_PORT_SEND_EQUAL_COND(e)          do { if ((e)) { AT_PORT_SEND_CONST_STR("="); } } while (0)

/* Send special characters */
#define AT_PORT_SEND_CTRL_Z()               AT_PORT_SEND_STR("\x1A")
#define AT_PORT_SEND_ESC()                  AT_PORT_SEND_STR("\x1B")
#endif /* !__DOXYGEN__ */

static lwgsm_recv_t recv_buff;
static lwgsmr_t lwgsmi_process_sub_cmd(lwgsm_msg_t* msg, uint8_t* is_ok, uint16_t* is_error);

/**
 * \brief           Memory mapping
 */
const lwgsm_dev_mem_map_t
lwgsm_dev_mem_map[] = {
#define LWGSM_DEV_MEMORY_ENTRY(name, str_code)    { LWGSM_MEM_ ## name, str_code },
#include "lwgsm/lwgsm_memories.h"
};

/**
 * \brief           Size of device memory mapping array
 */
const size_t
lwgsm_dev_mem_map_size = LWGSM_ARRAYSIZE(lwgsm_dev_mem_map);

/**
 * \brief           List of supported devices
 */
const lwgsm_dev_model_map_t
lwgsm_dev_model_map[] = {
#define LWGSM_DEVICE_MODEL_ENTRY(name, str_id, is_2g, is_lte)     { LWGSM_DEVICE_MODEL_ ## name, str_id, is_2g, is_lte },
#include "lwgsm/lwgsm_models.h"
};

/**
 * \brief           Size of device models mapping array
 */
const size_t
lwgsm_dev_model_map_size = LWGSM_ARRAYSIZE(lwgsm_dev_model_map);

/**
 * \brief           Free connection send data memory
 * \param[in]       m: Send data message type
 */
#define CONN_SEND_DATA_FREE(m)      do {            \
        if ((m) != NULL && (m)->msg.conn_send.fau) {    \
            (m)->msg.conn_send.fau = 0;                 \
            if ((m)->msg.conn_send.data != NULL) {      \
                LWGSM_DEBUGF(LWGSM_CFG_DBG_CONN | LWGSM_DBG_TYPE_TRACE,   \
                           "[CONN] Free write buffer fau: %p\r\n", (void *)(m)->msg.conn_send.data);   \
                lwgsm_mem_free_s((void **)&((m)->msg.conn_send.data)); \
            }                                           \
        }                                               \
    } while (0)

/**
 * \brief           Send connection callback for "data send"
 * \param[in]       m: Command message
 * \param[in]       err: Error of type \ref lwgsmr_t
 */
#define CONN_SEND_DATA_SEND_EVT(m, err)  do { \
        CONN_SEND_DATA_FREE(m);                         \
        lwgsm.evt.type = LWGSM_EVT_CONN_SEND;               \
        lwgsm.evt.evt.conn_data_send.res = err;           \
        lwgsm.evt.evt.conn_data_send.conn = (m)->msg.conn_send.conn;  \
        lwgsm.evt.evt.conn_data_send.sent = (m)->msg.conn_send.sent_all;  \
        lwgsmi_send_conn_cb((m)->msg.conn_send.conn, NULL);   \
    } while (0)

/**
 * \brief           Send reset sequence event
 * \param[in]       m: Command message
 * \param[in]       err: Error of type \ref lwgsmr_t
 */
#define RESET_SEND_EVT(m, err)  do {                \
        lwgsm.evt.evt.reset.res = err;                    \
        lwgsmi_send_cb(LWGSM_EVT_RESET);                    \
    } while (0)

/**
 * \brief           Send restore sequence event
 * \param[in]       m: Connection send message
 * \param[in]       err: Error of type \ref lwgsmr_t
 */
#define RESTORE_SEND_EVT(m, err)  do {              \
        lwgsm.evt.evt.restore.res = err;                  \
        lwgsmi_send_cb(LWGSM_EVT_RESTORE);                  \
    } while (0)

/**
 * \brief           Send operator scan sequence event
 * \param[in]       m: Command message
 * \param[in]       err: Error of type \ref lwgsmr_t
 */
#define OPERATOR_SCAN_SEND_EVT(m, err)  do {        \
        lwgsm.evt.evt.operator_scan.res = err;            \
        lwgsm.evt.evt.operator_scan.ops = (m)->msg.cops_scan.ops; \
        lwgsm.evt.evt.operator_scan.opf = *(m)->msg.cops_scan.opf;\
        lwgsmi_send_cb(LWGSM_EVT_OPERATOR_SCAN);            \
    } while (0)

/**
* \brief           Send SMS delete operation event
* \param[in]       m: SMS delete message
* \param[in]       err: Error of type \ref lwgsmr_t
*/
#define SMS_SEND_DELETE_EVT(m, err)     do {        \
        lwgsm.evt.evt.sms_delete.res = err;               \
        lwgsm.evt.evt.sms_delete.mem = (m)->msg.sms_delete.mem;   \
        lwgsm.evt.evt.sms_delete.pos = (m)->msg.sms_delete.pos;   \
        lwgsmi_send_cb(LWGSM_EVT_SMS_DELETE);               \
    } while (0)

/**
 * \brief           Send SMS read operation event
 * \param[in]       m: SMS read message
 * \param[in]       err: Error of type \ref lwgsmr_t
 */
#define SMS_SEND_READ_EVT(m, err)     do {          \
        lwgsm.evt.evt.sms_read.res = err;                 \
        lwgsm.evt.evt.sms_read.entry = (m)->msg.sms_read.entry;   \
        lwgsmi_send_cb(LWGSM_EVT_SMS_READ);                 \
    } while (0)

/**
 * \brief           Send SMS read operation event
 * \param[in]       mm: SMS list message
 * \param[in]       err: Error of type \ref lwgsmr_t
 */
#define SMS_SEND_LIST_EVT(mm, err)     do {        \
        lwgsm.evt.evt.sms_list.mem = lwgsm.m.sms.mem[0].current;\
        lwgsm.evt.evt.sms_list.entries = (mm)->msg.sms_list.entries;  \
        lwgsm.evt.evt.sms_list.size = (mm)->msg.sms_list.ei;  \
        lwgsm.evt.evt.sms_list.res = err;                 \
        lwgsmi_send_cb(LWGSM_EVT_SMS_LIST);                 \
    } while (0)

/**
 * \brief           Send SMS send operation event
 * \param[in]       m: SMS send message
 * \param[in]       err: Error of type \ref lwgsmr_t
 */
#define SMS_SEND_SEND_EVT(m, err)     do {          \
        lwgsm.evt.evt.sms_send.pos = (m)->msg.sms_send.pos;   \
        lwgsm.evt.evt.sms_send.res = err;                 \
        lwgsmi_send_cb(LWGSM_EVT_SMS_SEND);                 \
    } while (0)

/**
 * \brief           Get SIM info when SIM is ready
 * \param[in]       blocking: Blocking command
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsmi_get_sim_info(const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_SIM_PROCESS_BASIC_CMDS;
    LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CNUM;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

/**
 * \brief           Send IP or MAC address to AT port
 * \param[in]       d: Pointer to IP or MAC address
 * \param[in]       is_ip: Set to `1` when sending IP, `0` when MAC
 * \param[in]       q: Set to `1` to include start and ending quotes
 * \param[in]       c: Set to `1` to include comma before string
 */
void
lwgsmi_send_ip_mac(const void* d, uint8_t is_ip, uint8_t q, uint8_t c) {
    uint8_t ch;
    char str[4];
    const lwgsm_mac_t* mac = d;
    const lwgsm_ip_t* ip = d;

    AT_PORT_SEND_COMMA_COND(c);                 /* Send comma */
    if (d == NULL) {
        return;
    }
    AT_PORT_SEND_QUOTE_COND(q);                 /* Send quote */
    ch = is_ip ? '.' : ':';                     /* Get delimiter character */
    for (uint8_t i = 0; i < (is_ip ? 4 : 6); ++i) { /* Process byte by byte */
        if (is_ip) {                            /* In case of IP ... */
            lwgsm_u8_to_str(ip->ip[i], str);    /* ... go to decimal format ... */
        } else {                                /* ... in case of MAC ... */
            lwgsm_u8_to_hex_str(mac->mac[i], str, 2);   /* ... go to HEX format */
        }
        AT_PORT_SEND_STR(str);                  /* Send str */
        if (i < (is_ip ? 4 : 6) - 1) {          /* Check end if characters */
            AT_PORT_SEND_CHR(&ch);              /* Send character */
        }
    }
    AT_PORT_SEND_QUOTE_COND(q);                 /* Send quote */
}

/**
 * \brief           Send string to AT port, either plain or escaped
 * \param[in]       str: Pointer to input string to string
 * \param[in]       e: Value to indicate string send format, escaped (`1`) or plain (`0`)
 * \param[in]       q: Value to indicate starting and ending quotes, enabled (`1`) or disabled (`0`)
 * \param[in]       c: Set to `1` to include comma before string
 */
void
lwgsmi_send_string(const char* str, uint8_t e, uint8_t q, uint8_t c) {
    char special = '\\';

    AT_PORT_SEND_COMMA_COND(c);                 /* Send comma */
    AT_PORT_SEND_QUOTE_COND(q);                 /* Send quote */
    if (str != NULL) {
        if (e) {                                /* Do we have to escape string? */
            while (*str) {                      /* Go through string */
                if (*str == ',' || *str == '"' || *str == '\\') {   /* Check for special character */
                    AT_PORT_SEND_CHR(&special); /* Send special character */
                }
                AT_PORT_SEND_CHR(str);          /* Send character */
                ++str;
            }
        } else {
            AT_PORT_SEND_STR(str);              /* Send plain string */
        }
    }
    AT_PORT_SEND_QUOTE_COND(q);                 /* Send quote */
}

/**
 * \brief           Send number (decimal) to AT port
 * \param[in]       num: Number to send to AT port
 * \param[in]       q: Value to indicate starting and ending quotes, enabled (`1`) or disabled (`0`)
 * \param[in]       c: Set to `1` to include comma before string
 */
void
lwgsmi_send_number(uint32_t num, uint8_t q, uint8_t c) {
    char str[11];

    lwgsm_u32_to_str(num, str);                 /* Convert digit to decimal string */

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
lwgsmi_send_port(lwgsm_port_t port, uint8_t q, uint8_t c) {
    char str[6];

    lwgsm_u16_to_str(LWGSM_PORT2NUM(port), str);/* Convert digit to decimal string */

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
lwgsmi_send_signed_number(int32_t num, uint8_t q, uint8_t c) {
    char str[11];

    lwgsm_i32_to_str(num, str);                 /* Convert digit to decimal string */

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
lwgsmi_send_dev_memory(lwgsm_mem_t mem, uint8_t q, uint8_t c) {
    if (mem < LWGSM_MEM_END) {                  /* Check valid range */
        lwgsmi_send_string(lwgsm_dev_mem_map[LWGSM_SZ(mem)].mem_str, 0, q, c);
    }
}

#if LWGSM_CFG_SMS || __DOXYGEN__

/**
 * \brief           Send SMS status text
 * \param[in]       status: SMS status
 * \param[in]       q: Value to indicate starting and ending quotes, enabled (`1`) or disabled (`0`)
 * \param[in]       c: Set to `1` to include comma before string
 */
void
lwgsmi_send_sms_stat(lwgsm_sms_status_t status, uint8_t q, uint8_t c) {
    const char* t;
    switch (status) {
        case LWGSM_SMS_STATUS_UNREAD:
            t = "REC UNREAD";
            break;
        case LWGSM_SMS_STATUS_READ:
            t = "REC READ";
            break;
        case LWGSM_SMS_STATUS_UNSENT:
            t = "STO UNSENT";
            break;
        case LWGSM_SMS_STATUS_SENT:
            t = "STO SENT";
            break;
        case LWGSM_SMS_STATUS_ALL:
        default:
            t = "ALL";
            break;
    }
    lwgsmi_send_string(t, 0, q, c);
}

#endif /* LWGSM_CFG_SMS */

#if LWGSM_CFG_CONN || __DOXYGEN__

/**
 * \brief           Reset all connections
 * \note            Used to notify upper layer stack to close everything and reset the memory if necessary
 * \param[in]       forced: Flag indicating reset was forced by user
 */
static void
reset_connections(uint8_t forced) {
    lwgsm.evt.type = LWGSM_EVT_CONN_CLOSE;
    lwgsm.evt.evt.conn_active_close.forced = forced;
    lwgsm.evt.evt.conn_active_close.res = lwgsmOK;

    for (size_t i = 0; i < LWGSM_CFG_MAX_CONNS; ++i) {  /* Check all connections */
        if (lwgsm.m.conns[i].status.f.active) {
            lwgsm.m.conns[i].status.f.active = 0;

            lwgsm.evt.evt.conn_active_close.conn = &lwgsm.m.conns[i];
            lwgsm.evt.evt.conn_active_close.client = lwgsm.m.conns[i].status.f.client;
            lwgsmi_send_conn_cb(&lwgsm.m.conns[i], NULL);   /* Send callback function */
        }
    }
}

#endif /* LWGSM_CFG_CONN || __DOXYGEN__ */

/**
 * \brief           Reset everything after reset was detected
 * \param[in]       forced: Set to `1` if reset forced by user
 */
void
lwgsmi_reset_everything(uint8_t forced) {
    /**
     * \todo: Put stack to default state:
     *          - Close all the connection in memory
     *          - Clear entire data memory
     *          - Reset GSM structure
     */

#if LWGSM_CFG_CONN
    /* Manually close all connections in memory */
    reset_connections(forced);

    /* Check if IPD active */
    if (lwgsm.m.ipd.buff != NULL) {
        lwgsm_pbuf_free(lwgsm.m.ipd.buff);
        lwgsm.m.ipd.buff = NULL;
    }
#endif /* LWGSM_CFG_CONN */

#if LWGSM_CFG_NETWORK
    /* Notify app about detached network PDP context */
    if (lwgsm.m.network.is_attached) {
        lwgsm.m.network.is_attached = 0;
        lwgsmi_send_cb(LWGSM_EVT_NETWORK_DETACHED);
    }
#endif /* LWGSM_CFG_NETWORK */

    /* Invalid GSM modules */
    LWGSM_MEMSET(&lwgsm.m, 0x00, sizeof(lwgsm.m));

    /* Manually set states */
    lwgsm.m.sim.state = (lwgsm_sim_state_t) -1;
    lwgsm.m.model = LWGSM_DEVICE_MODEL_UNKNOWN;
}

/**
 * \brief           Process callback function to user with specific type
 * \param[in]       type: Callback event type
 * \return          Member of \ref lwgsmr_t enumeration
 */
lwgsmr_t
lwgsmi_send_cb(lwgsm_evt_type_t type) {
    lwgsm.evt.type = type;                      /* Set callback type to process */

    /* Call callback function for all registered functions */
    for (lwgsm_evt_func_t* link = lwgsm.evt_func; link != NULL; link = link->next) {
        link->fn(&lwgsm.evt);
    }
    return lwgsmOK;
}

#if LWGSM_CFG_CONN || __DOXYGEN__

/**
 * \brief           Process connection callback
 * \note            Before calling function, callback structure must be prepared
 * \param[in]       conn: Pointer to connection to use as callback
 * \param[in]       evt: Event callback function for connection
 * \return          Member of \ref lwgsmr_t enumeration
 */
lwgsmr_t
lwgsmi_send_conn_cb(lwgsm_conn_t* conn, lwgsm_evt_fn evt) {
    if (conn->status.f.in_closing && lwgsm.evt.type != LWGSM_EVT_CONN_CLOSE) {  /* Do not continue if in closing mode */
        /* return lwgsmOK; */
    }

    if (evt != NULL) {                          /* Try with user connection */
        return evt(&lwgsm.evt);                 /* Call temporary function */
    } else if (conn != NULL && conn->evt_func != NULL) {/* Connection custom callback? */
        return conn->evt_func(&lwgsm.evt);      /* Process callback function */
    } else if (conn == NULL) {
        return lwgsmOK;
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
    return lwgsm_conn_close(conn, 0);
}

/**
 * \brief           Process and send data from device buffer
 * \return          Member of \ref lwgsmr_t enumeration
 */
static lwgsmr_t
lwgsmi_tcpip_process_send_data(void) {
    lwgsm_conn_t* c = lwgsm.msg->msg.conn_send.conn;
    if (!lwgsm_conn_is_active(c) ||             /* Is the connection already closed? */
        lwgsm.msg->msg.conn_send.val_id != c->val_id/* Did validation ID change after we set parameter? */
       ) {
        /* Send event to user about failed send event */
        CONN_SEND_DATA_SEND_EVT(lwgsm.msg, lwgsmCLOSED);
        return lwgsmERR;
    }
    lwgsm.msg->msg.conn_send.sent = LWGSM_MIN(lwgsm.msg->msg.conn_send.btw, LWGSM_CFG_CONN_MAX_DATA_LEN);

    AT_PORT_SEND_BEGIN_AT();
    AT_PORT_SEND_CONST_STR("+CIPSEND=");
    lwgsmi_send_number(LWGSM_U32(c->num), 0, 0);/* Send connection number */
    lwgsmi_send_number(LWGSM_U32(lwgsm.msg->msg.conn_send.sent), 0, 1); /* Send length number */

    /* On UDP connections, IP address and port may be selected */
    if (c->type == LWGSM_CONN_TYPE_UDP) {
        if (lwgsm.msg->msg.conn_send.remote_ip != NULL && lwgsm.msg->msg.conn_send.remote_port) {
            lwgsmi_send_ip_mac(lwgsm.msg->msg.conn_send.remote_ip, 1, 1, 1);/* Send IP address including quotes */
            lwgsmi_send_port(lwgsm.msg->msg.conn_send.remote_port, 0, 1);   /* Send length number */
        }
    }
    AT_PORT_SEND_END_AT();
    return lwgsmOK;
}

/**
 * \brief           Process data sent and send remaining
 * \param[in]       sent: Status whether data were sent or not,
 *                      info received from GSM with "SEND OK" or "SEND FAIL"
 * \return          `1` in case we should stop sending or `0` if we still have data to process
 */
static uint8_t
lwgsmi_tcpip_process_data_sent(uint8_t sent) {
    if (sent) {                                 /* Data were successfully sent */
        lwgsm.msg->msg.conn_send.sent_all += lwgsm.msg->msg.conn_send.sent;
        lwgsm.msg->msg.conn_send.btw -= lwgsm.msg->msg.conn_send.sent;
        lwgsm.msg->msg.conn_send.ptr += lwgsm.msg->msg.conn_send.sent;
        if (lwgsm.msg->msg.conn_send.bw != NULL) {
            *lwgsm.msg->msg.conn_send.bw += lwgsm.msg->msg.conn_send.sent;
        }
        lwgsm.msg->msg.conn_send.tries = 0;
    } else {                                    /* We were not successful */
        ++lwgsm.msg->msg.conn_send.tries;       /* Increase number of tries */
        if (lwgsm.msg->msg.conn_send.tries == LWGSM_CFG_MAX_SEND_RETRIES) { /* In case we reached max number of retransmissions */
            return 1;                           /* Return 1 and indicate error */
        }
    }
    if (lwgsm.msg->msg.conn_send.btw > 0) {     /* Do we still have data to send? */
        if (lwgsmi_tcpip_process_send_data() != lwgsmOK) {  /* Check if we can continue */
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
 * \param[in,out]   is_error: Pointer to current error status
 */
void
lwgsmi_process_cipsend_response(lwgsm_recv_t* rcv, uint8_t* is_ok, uint16_t* is_error) {
    if (lwgsm.msg->msg.conn_send.wait_send_ok_err) {
        if (LWGSM_CHARISNUM(rcv->data[0]) && rcv->data[1] == ',') {
            uint8_t num = LWGSM_CHARTONUM(rcv->data[0]);
            if (!strncmp(&rcv->data[3], "SEND OK" CRLF, 7 + CRLF_LEN)) {
                lwgsm.msg->msg.conn_send.wait_send_ok_err = 0;
                *is_ok = lwgsmi_tcpip_process_data_sent(1); /* Process as data were sent */
                if (*is_ok && lwgsm.msg->msg.conn_send.conn->status.f.active) {
                    CONN_SEND_DATA_SEND_EVT(lwgsm.msg, lwgsmOK);
                }
            } else if (!strncmp(&rcv->data[3], "SEND FAIL" CRLF, 9 + CRLF_LEN)) {
                lwgsm.msg->msg.conn_send.wait_send_ok_err = 0;
                *is_error = lwgsmi_tcpip_process_data_sent(0);  /* Data were not sent due to SEND FAIL or command didn't even start */
                if (*is_error && lwgsm.msg->msg.conn_send.conn->status.f.active) {
                    CONN_SEND_DATA_SEND_EVT(lwgsm.msg, lwgsmERR);
                }
            }
            LWGSM_UNUSED(num);
        }
        /* Check for an error or if connection closed in the meantime */
    } else if (*is_error) {
        CONN_SEND_DATA_SEND_EVT(lwgsm.msg, lwgsmERR);
    }
}

/**
 * \brief           Send error event to application layer
 * \param[in]       msg: Message from user with connection start
 * \param[in]       error: Error type
 */
static void
lwgsmi_send_conn_error_cb(lwgsm_msg_t* msg, lwgsmr_t error) {
    lwgsm.evt.type = LWGSM_EVT_CONN_ERROR;      /* Connection error */
    lwgsm.evt.evt.conn_error.host = lwgsm.msg->msg.conn_start.host;
    lwgsm.evt.evt.conn_error.port = lwgsm.msg->msg.conn_start.port;
    lwgsm.evt.evt.conn_error.type = lwgsm.msg->msg.conn_start.type;
    lwgsm.evt.evt.conn_error.arg = lwgsm.msg->msg.conn_start.arg;
    lwgsm.evt.evt.conn_error.err = error;

    /* Call callback specified by user on connection startup */
    lwgsm.msg->msg.conn_start.evt_func(&lwgsm.evt);
    LWGSM_UNUSED(msg);
}

/**
 * \brief           Checks if connection pointer has valid address
 * \param[in]       conn: Address to check if valid connection ptr
 * \return          1 on success, 0 otherwise
 */
uint8_t
lwgsmi_is_valid_conn_ptr(lwgsm_conn_p conn) {
    uint8_t i = 0;
    for (i = 0; i < LWGSM_ARRAYSIZE(lwgsm.m.conns); ++i) {
        if (conn == &lwgsm.m.conns[i]) {
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
lwgsmi_conn_closed_process(uint8_t conn_num, uint8_t forced) {
    lwgsm_conn_t* conn = &lwgsm.m.conns[conn_num];

    conn->status.f.active = 0;

    /* Check if write buffer is set */
    if (conn->buff.buff != NULL) {
        LWGSM_DEBUGF(LWGSM_CFG_DBG_CONN | LWGSM_DBG_TYPE_TRACE,
                   "[CONN] Free write buffer: %p\r\n", conn->buff.buff);
        lwgsm_mem_free_s((void**)&conn->buff.buff);
    }

    /* Send event */
    lwgsm.evt.type = LWGSM_EVT_CONN_CLOSE;
    lwgsm.evt.evt.conn_active_close.conn = conn;
    lwgsm.evt.evt.conn_active_close.forced = forced;
    lwgsm.evt.evt.conn_active_close.res = lwgsmOK;
    lwgsm.evt.evt.conn_active_close.client = conn->status.f.client;
    lwgsmi_send_conn_cb(conn, NULL);

    return 1;
}

#endif /* LWGSM_CFG_CONN || __DOXYGEN__ */

/**
 * \brief           Process received string from GSM
 * \param[in]       rcv: Pointer to \ref lwgsm_recv_t structure with input string
 */
static void
lwgsmi_parse_received(lwgsm_recv_t* rcv) {
    uint8_t is_ok = 0;
    uint16_t is_error = 0;

    /* Try to remove non-parsable strings */
    if (rcv->len == 2 && rcv->data[0] == '\r' && rcv->data[1] == '\n') {
        return;
    }

    /* Check OK response */
    is_ok = rcv->len == (2 + CRLF_LEN) && !strcmp(rcv->data, "OK" CRLF);/* Check if received string is OK */
    if (!is_ok) {                               /* Check for SHUT OK string */
        is_ok = rcv->len == (7 + CRLF_LEN) && !strcmp(rcv->data, "SEND OK" CRLF);
    }

    /* Check error response */
    if (!is_ok) {                               /* If still not ok, check if error? */
        is_error = rcv->data[0] == '+' && !strncmp(rcv->data, "+CME ERROR", 10);/* First check +CME coded errors */
        if (!is_error) {                        /* Check basic error aswell */
            is_error = rcv->data[0] == '+' && !strncmp(rcv->data, "+CMS ERROR", 10);/* First check +CME coded errors */
            if (!is_error) {
                is_error = !strcmp(rcv->data, "ERROR" CRLF) || !strcmp(rcv->data, "FAIL" CRLF);
            }
        }
    }

    /* Scan received strings which start with '+' */
    if (rcv->data[0] == '+') {
        if (!strncmp(rcv->data, "+CSQ", 4)) {
            lwgsmi_parse_csq(rcv->data);        /* Parse +CSQ response */
#if LWGSM_CFG_NETWORK
        } else if (!strncmp(rcv->data, "+PDP: DEACT", 11)) {
            /* PDP has been deactivated */
            lwgsm_network_check_status(NULL, NULL, 0);  /* Update status */
#endif /* LWGSM_CFG_NETWORK */
#if LWGSM_CFG_CONN
        } else if (!strncmp(rcv->data, "+RECEIVE", 8)) {
            lwgsmi_parse_ipd(rcv->data);        /* Parse IPD */
#endif /* LWGSM_CFG_CONN */
        } else if (!strncmp(rcv->data, "+CREG", 5)) {   /* Check for +CREG indication */
            lwgsmi_parse_creg(rcv->data, LWGSM_U8(CMD_IS_CUR(LWGSM_CMD_CREG_GET))); /* Parse +CREG response */
        } else if (!strncmp(rcv->data, "+CPIN", 5)) {   /* Check for +CPIN indication for SIM */
            lwgsmi_parse_cpin(rcv->data, 1 /* !CMD_IS_DEF(LWGSM_CMD_CPIN_SET) */);  /* Parse +CPIN response */
        } else if (CMD_IS_CUR(LWGSM_CMD_COPS_GET) && !strncmp(rcv->data, "+COPS", 5)) {
            lwgsmi_parse_cops(rcv->data);       /* Parse current +COPS */
#if LWGSM_CFG_SMS
        } else if (CMD_IS_CUR(LWGSM_CMD_CMGS) && !strncmp(rcv->data, "+CMGS", 5)) {
            lwgsmi_parse_cmgs(rcv->data, &lwgsm.msg->msg.sms_send.pos); /* Parse +CMGS response */
        } else if (CMD_IS_CUR(LWGSM_CMD_CMGR) && !strncmp(rcv->data, "+CMGR", 5)) {
            if (lwgsmi_parse_cmgr(rcv->data)) { /* Parse +CMGR response */
                lwgsm.msg->msg.sms_read.read = 2;   /* Set read flag and process the data */
            } else {
                lwgsm.msg->msg.sms_read.read = 1;   /* Read but ignore data */
            }
        } else if (CMD_IS_CUR(LWGSM_CMD_CMGL) && !strncmp(rcv->data, "+CMGL", 5)) {
            if (lwgsmi_parse_cmgl(rcv->data)) { /* Parse +CMGL response */
                lwgsm.msg->msg.sms_list.read = 2;   /* Set read flag and process the data */
            } else {
                lwgsm.msg->msg.sms_list.read = 1;   /* Read but ignore data */
            }
        } else if (!strncmp(rcv->data, "+CMTI", 5)) {
            lwgsmi_parse_cmti(rcv->data, 1);    /* Parse +CMTI response with received SMS */
        } else if (CMD_IS_CUR(LWGSM_CMD_CPMS_GET_OPT) && !strncmp(rcv->data, "+CPMS", 5)) {
            lwgsmi_parse_cpms(rcv->data, 0);    /* Parse +CPMS with SMS memories info */
        } else if (CMD_IS_CUR(LWGSM_CMD_CPMS_GET) && !strncmp(rcv->data, "+CPMS", 5)) {
            lwgsmi_parse_cpms(rcv->data, 1);    /* Parse +CPMS with SMS memories info */
        } else if (CMD_IS_CUR(LWGSM_CMD_CPMS_SET) && !strncmp(rcv->data, "+CPMS", 5)) {
            lwgsmi_parse_cpms(rcv->data, 2);    /* Parse +CPMS with SMS memories info */
#endif /* LWGSM_CFG_SMS */
#if LWGSM_CFG_CALL
        } else if (!strncmp(rcv->data, "+CLCC", 5)) {
            lwgsmi_parse_clcc(rcv->data, 1);    /* Parse +CLCC response with call info change */
#endif /* LWGSM_CFG_CALL */
#if LWGSM_CFG_PHONEBOOK
        } else if (CMD_IS_CUR(LWGSM_CMD_CPBS_GET_OPT) && !strncmp(rcv->data, "+CPBS", 5)) {
            lwgsmi_parse_cpbs(rcv->data, 0);    /* Parse +CPBS response */
        } else if (CMD_IS_CUR(LWGSM_CMD_CPBS_GET) && !strncmp(rcv->data, "+CPBS", 5)) {
            lwgsmi_parse_cpbs(rcv->data, 1);    /* Parse +CPBS response */
        } else if (CMD_IS_CUR(LWGSM_CMD_CPBS_SET) && !strncmp(rcv->data, "+CPBS", 5)) {
            lwgsmi_parse_cpbs(rcv->data, 2);    /* Parse +CPBS response */
        } else if (CMD_IS_CUR(LWGSM_CMD_CPBR) && !strncmp(rcv->data, "+CPBR", 5)) {
            lwgsmi_parse_cpbr(rcv->data);       /* Parse +CPBR statement */
        } else if (CMD_IS_CUR(LWGSM_CMD_CPBF) && !strncmp(rcv->data, "+CPBF", 5)) {
            lwgsmi_parse_cpbf(rcv->data);       /* Parse +CPBR statement */
#endif /* LWGSM_CFG_PHONEBOOK */
        }

        /* Messages not starting with '+' sign */
    } else {
        if (rcv->data[0] == 'S' && !strncmp(rcv->data, "SHUT OK" CRLF, 7 + CRLF_LEN)) {
            is_ok = 1;
#if LWGSM_CFG_CONN
        } else if (LWGSM_CHARISNUM(rcv->data[0]) && rcv->data[1] == ',' && rcv->data[2] == ' '
                   && (!strncmp(&rcv->data[3], "CLOSE OK" CRLF, 8 + CRLF_LEN) || !strncmp(&rcv->data[3], "CLOSED" CRLF, 6 + CRLF_LEN))) {
            uint8_t forced = 0, num;

            num = LWGSM_CHARTONUM(rcv->data[0]);/* Get connection number */
            if (CMD_IS_CUR(LWGSM_CMD_CIPCLOSE) && lwgsm.msg->msg.conn_close.conn->num == num) {
                forced = 1;
                is_ok = 1;                      /* If forced and connection is closed, command is OK */
            }

            /* Manually stop send command? */
            if (CMD_IS_CUR(LWGSM_CMD_CIPSEND) && lwgsm.msg->msg.conn_send.conn->num == num) {
                /*
                 * If active command is CIPSEND and CLOSED event received,
                 * manually set error and process usual "ERROR" event on senddata
                 */
                is_error = 1;                   /* This is an error in response */
                lwgsmi_process_cipsend_response(rcv, &is_ok, &is_error);
            }
            lwgsmi_conn_closed_process(num, forced);/* Connection closed, process */
#endif /* LWGSM_CFG_CONN */
#if LWGSM_CFG_CALL
        } else if (rcv->data[0] == 'C' && !strncmp(rcv->data, "Call Ready" CRLF, 10 + CRLF_LEN)) {
            lwgsm.m.call.ready = 1;
            lwgsmi_send_cb(LWGSM_EVT_CALL_READY);   /* Send CALL ready event */
        } else if (rcv->data[0] == 'R' && !strncmp(rcv->data, "RING" CRLF, 4 + CRLF_LEN)) {
            lwgsmi_send_cb(LWGSM_EVT_CALL_RING);/* Send call ring */
        } else if (rcv->data[0] == 'N' && !strncmp(rcv->data, "NO CARRIER" CRLF, 10 + CRLF_LEN)) {
            lwgsmi_send_cb(LWGSM_EVT_CALL_NO_CARRIER);  /* Send call no carrier event */
        } else if (rcv->data[0] == 'B' && !strncmp(rcv->data, "BUSY" CRLF, 4 + CRLF_LEN)) {
            lwgsmi_send_cb(LWGSM_EVT_CALL_BUSY);/* Send call busy message */
#endif /* LWGSM_CFG_CALL */
#if LWGSM_CFG_SMS
        } else if (rcv->data[0] == 'S' && !strncmp(rcv->data, "SMS Ready" CRLF, 9 + CRLF_LEN)) {
            lwgsm.m.sms.ready = 1;              /* SMS ready flag */
            lwgsmi_send_cb(LWGSM_EVT_SMS_READY);/* Send SMS ready event */
#endif /* LWGSM_CFG_SMS */
        } else if ((CMD_IS_CUR(LWGSM_CMD_CGMI_GET) || CMD_IS_CUR(LWGSM_CMD_CGMM_GET) || CMD_IS_CUR(LWGSM_CMD_CGSN_GET) || CMD_IS_CUR(LWGSM_CMD_CGMR_GET))
                   && !is_ok && !is_error && strncmp(rcv->data, "AT+", 3)) {
            const char* tmp = rcv->data;
            size_t tocopy;
            if (CMD_IS_CUR(LWGSM_CMD_CGMI_GET)) {   /* Check device manufacturer */
                lwgsmi_parse_string(&tmp, lwgsm.m.model_manufacturer, sizeof(lwgsm.m.model_manufacturer), 1);
                if (CMD_IS_DEF(LWGSM_CMD_CGMI_GET)) {
                    tocopy = LWGSM_MIN(sizeof(lwgsm.m.model_manufacturer), lwgsm.msg->msg.device_info.len);
                    LWGSM_MEMCPY(lwgsm.msg->msg.device_info.str, lwgsm.m.model_manufacturer, tocopy);
                    lwgsm.msg->msg.device_info.str[tocopy - 1] = 0;
                }
            } else if (CMD_IS_CUR(LWGSM_CMD_CGMM_GET)) {/* Check device model number */
                lwgsmi_parse_string(&tmp, lwgsm.m.model_number, sizeof(lwgsm.m.model_number), 1);
                if (CMD_IS_DEF(LWGSM_CMD_CGMM_GET)) {
                    tocopy = LWGSM_MIN(sizeof(lwgsm.m.model_number), lwgsm.msg->msg.device_info.len);
                    LWGSM_MEMCPY(lwgsm.msg->msg.device_info.str, lwgsm.m.model_number, tocopy);
                    lwgsm.msg->msg.device_info.str[tocopy - 1] = 0;
                }
                for (size_t i = 0; i < lwgsm_dev_model_map_size; ++i) {
                    if (strstr(lwgsm.m.model_number, lwgsm_dev_model_map[i].id_str) != NULL) {
                        lwgsm.m.model = lwgsm_dev_model_map[i].model;
                        break;
                    }
                }
            } else if (CMD_IS_CUR(LWGSM_CMD_CGSN_GET)) {/* Check device serial number */
                lwgsmi_parse_string(&tmp, lwgsm.m.model_serial_number, sizeof(lwgsm.m.model_serial_number), 1);
                if (CMD_IS_DEF(LWGSM_CMD_CGSN_GET)) {
                    tocopy = LWGSM_MIN(sizeof(lwgsm.m.model_serial_number), lwgsm.msg->msg.device_info.len);
                    LWGSM_MEMCPY(lwgsm.msg->msg.device_info.str, lwgsm.m.model_serial_number, tocopy);
                    lwgsm.msg->msg.device_info.str[tocopy - 1] = 0;
                }
            } else if (CMD_IS_CUR(LWGSM_CMD_CGMR_GET)) {/* Check device revision */
                if (!strncmp(tmp, "Revision:", 9)) {
                    tmp += 9;
                }
                lwgsmi_parse_string(&tmp, lwgsm.m.model_revision, sizeof(lwgsm.m.model_revision), 1);
                if (CMD_IS_DEF(LWGSM_CMD_CGMR_GET)) {
                    tocopy = LWGSM_MIN(sizeof(lwgsm.m.model_revision), lwgsm.msg->msg.device_info.len);
                    LWGSM_MEMCPY(lwgsm.msg->msg.device_info.str, lwgsm.m.model_revision, tocopy);
                    lwgsm.msg->msg.device_info.str[tocopy - 1] = 0;
                }
            }
        } else if (CMD_IS_CUR(LWGSM_CMD_CIFSR) && LWGSM_CHARISNUM(rcv->data[0])) {
            const char* tmp = rcv->data;
            lwgsmi_parse_ip(&tmp, &lwgsm.m.network.ip_addr);/* Parse IP address */

            is_ok = 1;                          /* Manually set OK flag as we don't expect OK in CIFSR command */
        }
    }

    /* Check general responses for active commands */
    if (lwgsm.msg != NULL) {
        if (0) {
#if LWGSM_CFG_SMS
        } else if (CMD_IS_CUR(LWGSM_CMD_CMGS) && is_ok) {
            /* At this point we have to wait for "> " to send data */
#endif /* LWGSM_CFG_SMS */
#if LWGSM_CFG_CONN
        } else if (CMD_IS_CUR(LWGSM_CMD_CIPSTATUS)) {
            /* For CIPSTATUS, OK is returned before important data */
            if (is_ok) {
                is_ok = 0;
            }
            /* Check if connection data received */
            if (rcv->len > 3) {
                uint8_t continueScan, processed = 0;
                if (rcv->data[0] == 'C' && rcv->data[1] == ':' && rcv->data[2] == ' ') {
                    processed = 1;
                    lwgsmi_parse_cipstatus_conn(rcv->data, 1, &continueScan);

                    if (lwgsm.m.active_conns_cur_parse_num == (LWGSM_CFG_MAX_CONNS - 1)) {
                        is_ok = 1;
                    }
                } else if (!strncmp(rcv->data, "STATE:", 6)) {
                    processed = 1;
                    lwgsmi_parse_cipstatus_conn(rcv->data, 0, &continueScan);
                }

                /* Check if we shall stop processing at this stage */
                if (processed && !continueScan) {
                    is_ok = 1;
                }
            }
        } else if (CMD_IS_CUR(LWGSM_CMD_CIPSTART)) {
            /* For CIPSTART, OK is returned before important data */
            if (is_ok) {
                is_ok = 0;
            }

            /* Wait here for CONNECT status before we cancel connection */
            if (LWGSM_CHARISNUM(rcv->data[0])
                && rcv->data[1] == ',' && rcv->data[2] == ' ') {
                uint8_t num = LWGSM_CHARTONUM(rcv->data[0]);
                if (num < LWGSM_CFG_MAX_CONNS) {
                    uint8_t id;
                    lwgsm_conn_t* conn = &lwgsm.m.conns[num];   /* Get connection handle */

                    if (!strncmp(&rcv->data[3], "CONNECT OK" CRLF, 10 + CRLF_LEN)) {
                        id = conn->val_id;
                        LWGSM_MEMSET(conn, 0x00, sizeof(*conn));/* Reset connection parameters */
                        conn->num = num;
                        conn->status.f.active = 1;
                        conn->val_id = ++id;    /* Set new validation ID */

                        /* Set connection parameters */
                        conn->status.f.client = 1;
                        conn->evt_func = lwgsm.msg->msg.conn_start.evt_func;
                        conn->arg = lwgsm.msg->msg.conn_start.arg;

                        /* Set status */
                        lwgsm.msg->msg.conn_start.conn_res = LWGSM_CONN_CONNECT_OK;
                        is_ok = 1;
                    } else if (!strncmp(&rcv->data[3], "CONNECT FAIL" CRLF, 12 + CRLF_LEN)) {
                        lwgsm.msg->msg.conn_start.conn_res = LWGSM_CONN_CONNECT_ERROR;
                        is_error = 1;
                    } else if (!strncmp(&rcv->data[3], "ALREADY CONNECT" CRLF, 15 + CRLF_LEN)) {
                        lwgsm.msg->msg.conn_start.conn_res = LWGSM_CONN_CONNECT_ALREADY;
                        is_error = 1;
                    }
                }
            }
        } else if (CMD_IS_CUR(LWGSM_CMD_CIPSEND)) {
            if (is_ok) {
                is_ok = 0;
            }
            lwgsmi_process_cipsend_response(rcv, &is_ok, &is_error);
#endif /* LWGSM_CFG_CONN */
#if LWGSM_CFG_USSD
        } else if (CMD_IS_CUR(LWGSM_CMD_CUSD)) {
            /* OK is returned before +CUSD */
            /* Command is not finished yet, unless it was an ERROR */
            if (is_ok) {
                is_ok = 0;
            }

            /* Check for manual CUSTOM OK message */
            if (!strcmp(rcv->data, "CUSTOM_OK\r\n")) {
                is_ok = 1;
            }
#endif /* LWGSM_CFG_USSD */
        }
    }

    /*
     * In case of any of these events, simply release semaphore
     * and proceed with next command
     */
    if (is_ok || is_error) {
        lwgsmr_t res = lwgsmOK;
        if (lwgsm.msg != NULL) {                /* Do we have active message? */
            res = lwgsmi_process_sub_cmd(lwgsm.msg, &is_ok, &is_error);
            if (res != lwgsmCONT) {             /* Shall we continue with next subcommand under this one? */
                if (is_ok) {                    /* Check OK status */
                    res = lwgsm.msg->res = lwgsmOK;
                } else {                        /* Or error status */
                    res = lwgsm.msg->res = res; /* Set the error status */
                }
            } else {
                ++lwgsm.msg->i;                 /* Number of continue calls */
            }

            /*
             * When the command is finished,
             * release synchronization semaphore
             * from user thread and start with next command
             */
            if (res != lwgsmCONT) {             /* Do we have to continue to wait for command? */
                lwgsm_sys_sem_release(&lwgsm.sem_sync); /* Release semaphore */
            }
        }
    }
}

#if !LWGSM_CFG_INPUT_USE_PROCESS || __DOXYGEN__
/**
 * \brief           Process data from input buffer
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsmi_process_buffer(void) {
    void* data;
    size_t len;

    do {
        /*
         * Get length of linear memory in buffer
         * we can process directly as memory
         */
        len = lwgsm_buff_get_linear_block_read_length(&lwgsm.buff);
        if (len > 0) {
            /*
             * Get memory address of first element
             * in linear block of data to process
             */
            data = lwgsm_buff_get_linear_block_read_address(&lwgsm.buff);

            /* Process actual received data */
            lwgsmi_process(data, len);

            /*
             * Once data is processed, simply skip
             * the buffer memory and start over
             */
            lwgsm_buff_skip(&lwgsm.buff, len);
        }
    } while (len);
    return lwgsmOK;
}
#endif /* !LWGSM_CFG_INPUT_USE_PROCESS || __DOXYGEN__ */

/**
 * \brief           Process input data received from GSM device
 * \param[in]       data: Pointer to data to process
 * \param[in]       data_len: Length of data to process in units of bytes
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsmi_process(const void* data, size_t data_len) {
    uint8_t ch;
    const uint8_t* d = data;
    size_t d_len = data_len;
    static uint8_t ch_prev1, ch_prev2;
    static lwgsm_unicode_t unicode;

    /* Check status if device is available */
    if (!lwgsm.status.f.dev_present) {
        return lwgsmERRNODEVICE;
    }

    while (d_len > 0) {                         /* Read entire set of characters from buffer */
        ch = *d;                                /* Get next character */
        ++d;                                    /* Go to next character, must be here as it is used later on */
        --d_len;                                /* Decrease remaining length, must be here as it is decreased later too */

        if (0) {
#if LWGSM_CFG_CONN
        } else if (lwgsm.m.ipd.read) {          /* Read connection data */
            size_t len;

            if (lwgsm.m.ipd.buff != NULL) {     /* Do we have active buffer? */
                lwgsm.m.ipd.buff->payload[lwgsm.m.ipd.buff_ptr] = ch;   /* Save data character */
            }
            ++lwgsm.m.ipd.buff_ptr;
            --lwgsm.m.ipd.rem_len;

            /* Try to read more data directly from buffer */
            len = LWGSM_MIN(d_len, LWGSM_MIN(lwgsm.m.ipd.rem_len, lwgsm.m.ipd.buff != NULL ? (lwgsm.m.ipd.buff->len - lwgsm.m.ipd.buff_ptr) : lwgsm.m.ipd.rem_len));
            LWGSM_DEBUGF(LWGSM_CFG_DBG_IPD | LWGSM_DBG_TYPE_TRACE,
                       "[IPD] New length to read: %d bytes\r\n", (int)len);
            if (len > 0) {
                if (lwgsm.m.ipd.buff != NULL) { /* Is buffer valid? */
                    LWGSM_MEMCPY(&lwgsm.m.ipd.buff->payload[lwgsm.m.ipd.buff_ptr], d, len);
                    LWGSM_DEBUGF(LWGSM_CFG_DBG_IPD | LWGSM_DBG_TYPE_TRACE,
                               "[IPD] Bytes read: %d\r\n", (int)len);
                } else {                        /* Simply skip the data in buffer */
                    LWGSM_DEBUGF(LWGSM_CFG_DBG_IPD | LWGSM_DBG_TYPE_TRACE,
                               "[IPD] Bytes skipped: %d\r\n", (int)len);
                }
                d_len -= len;                   /* Decrease effective length */
                d += len;                       /* Skip remaining length */
                lwgsm.m.ipd.buff_ptr += len;    /* Forward buffer pointer */
                lwgsm.m.ipd.rem_len -= len;     /* Decrease remaining length */
            }

            /* Did we reach end of buffer or no more data? */
            if (lwgsm.m.ipd.rem_len == 0 || (lwgsm.m.ipd.buff != NULL && lwgsm.m.ipd.buff_ptr == lwgsm.m.ipd.buff->len)) {
                lwgsmr_t res = lwgsmOK;

                /* Call user callback function with received data */
                if (lwgsm.m.ipd.buff != NULL) { /* Do we have valid buffer? */
                    lwgsm.m.ipd.conn->total_recved += lwgsm.m.ipd.buff->tot_len;/* Increase number of bytes received */

                    /*
                     * Send data buffer to upper layer
                     *
                     * From this moment, user is responsible for packet
                     * buffer and must free it manually
                     */
                    lwgsm.evt.type = LWGSM_EVT_CONN_RECV;
                    lwgsm.evt.evt.conn_data_recv.buff = lwgsm.m.ipd.buff;
                    lwgsm.evt.evt.conn_data_recv.conn = lwgsm.m.ipd.conn;
                    res = lwgsmi_send_conn_cb(lwgsm.m.ipd.conn, NULL);

                    lwgsm_pbuf_free(lwgsm.m.ipd.buff);  /* Free packet buffer at this point */
                    LWGSM_DEBUGF(LWGSM_CFG_DBG_IPD | LWGSM_DBG_TYPE_TRACE,
                               "[IPD] Free packet buffer\r\n");
                    if (res == lwgsmOKIGNOREMORE) { /* We should ignore more data */
                        LWGSM_DEBUGF(LWGSM_CFG_DBG_IPD | LWGSM_DBG_TYPE_TRACE,
                                   "[IPD] Ignoring more data from this IPD if available\r\n");
                        lwgsm.m.ipd.buff = NULL;/* Set to NULL to ignore more data if possibly available */
                    }

                    /*
                     * Create new data packet if case if:
                     *
                     *  - Previous one was successful and more data to read and
                     *  - Connection is not in closing state
                     */
                    if (lwgsm.m.ipd.buff != NULL && lwgsm.m.ipd.rem_len > 0 && !lwgsm.m.ipd.conn->status.f.in_closing) {
                        size_t new_len = LWGSM_MIN(lwgsm.m.ipd.rem_len, LWGSM_CFG_IPD_MAX_BUFF_SIZE);   /* Calculate new buffer length */

                        LWGSM_DEBUGF(LWGSM_CFG_DBG_IPD | LWGSM_DBG_TYPE_TRACE,
                                   "[IPD] Allocating new packet buffer of size: %d bytes\r\n", (int)new_len);
                        lwgsm.m.ipd.buff = lwgsm_pbuf_new(new_len); /* Allocate new packet buffer */

                        LWGSM_DEBUGW(LWGSM_CFG_DBG_IPD | LWGSM_DBG_TYPE_TRACE | LWGSM_DBG_LVL_WARNING,
                                   lwgsm.m.ipd.buff == NULL, "[IPD] Buffer allocation failed for %d bytes\r\n", (int)new_len);
                    } else {
                        lwgsm.m.ipd.buff = NULL;/* Reset it */
                    }
                }
                if (lwgsm.m.ipd.rem_len == 0) { /* Check if we read everything */
                    lwgsm.m.ipd.buff = NULL;    /* Reset buffer pointer */
                    lwgsm.m.ipd.read = 0;       /* Stop reading data */
                }
                lwgsm.m.ipd.buff_ptr = 0;       /* Reset input buffer pointer */
            }
#endif /* LWGSM_CFG_CONN */
            /*
             * Check if operators scan command is active
             * and if we are ready to read the incoming data
             */
        } else if (CMD_IS_CUR(LWGSM_CMD_COPS_GET_OPT) && lwgsm.msg->msg.cops_scan.read) {
            if (ch == '\n') {
                lwgsm.msg->msg.cops_scan.read = 0;
            } else {
                lwgsmi_parse_cops_scan(ch, 0);  /* Parse character by character */
            }
#if LWGSM_CFG_SMS
        } else if (CMD_IS_CUR(LWGSM_CMD_CMGR) && lwgsm.msg->msg.sms_read.read) {
            lwgsm_sms_entry_t* e = lwgsm.msg->msg.sms_read.entry;
            if (lwgsm.msg->msg.sms_read.read == 2) {/* Read only if set to 2 */
                if (e != NULL) {                /* Check if valid entry */
                    if (e->length < (sizeof(e->data) - 1)) {
                        e->data[e->length++] = ch;
                    }
                } else {
                    lwgsm.msg->msg.sms_read.read = 1;   /* Read but ignore data */
                }
            }
            if (ch == '\n' && ch_prev1 == '\r') {
                if (lwgsm.msg->msg.sms_read.read == 2) {

                }
                lwgsm.msg->msg.sms_read.read = 0;
            }
        } else if (CMD_IS_CUR(LWGSM_CMD_CMGL) && lwgsm.msg->msg.sms_list.read) {
            if (lwgsm.msg->msg.sms_list.read == 2) {
                lwgsm_sms_entry_t* e = &lwgsm.msg->msg.sms_list.entries[lwgsm.msg->msg.sms_list.ei];
                if (e->length < (sizeof(e->data) - 1)) {
                    e->data[e->length++] = ch;
                }
            }
            if (ch == '\n' && ch_prev1 == '\r') {
                if (lwgsm.msg->msg.sms_list.read == 2) {
                    ++lwgsm.msg->msg.sms_list.ei;   /* Go to next entry */
                    if (lwgsm.msg->msg.sms_list.er != NULL) {   /* Check and update user variable */
                        *lwgsm.msg->msg.sms_list.er = lwgsm.msg->msg.sms_list.ei;
                    }
                }
                lwgsm.msg->msg.sms_list.read = 0;
            }
#endif /* LWGSM_CFG_SMS */
#if LWGSM_CFG_USSD
        } else if (CMD_IS_CUR(LWGSM_CMD_CUSD) && lwgsm.msg->msg.ussd.read) {
            if (ch == '"') {
                lwgsm.msg->msg.ussd.resp[lwgsm.msg->msg.ussd.resp_write_ptr] = 0;
                lwgsm.msg->msg.ussd.quote_det = !lwgsm.msg->msg.ussd.quote_det;
            } else if (lwgsm.msg->msg.ussd.quote_det) {
                if (lwgsm.msg->msg.ussd.resp_write_ptr < lwgsm.msg->msg.ussd.resp_len) {
                    lwgsm.msg->msg.ussd.resp[lwgsm.msg->msg.ussd.resp_write_ptr++] = ch;
                    lwgsm.msg->msg.ussd.resp[lwgsm.msg->msg.ussd.resp_write_ptr] = 0;
                }
            } else if (ch == '\n' && ch_prev1 == '\r') {
                /* End of reading, command finished! */
                /* Return OK at this point! */
                strcpy(recv_buff.data, "CUSTOM_OK\r\n");
                recv_buff.len = strlen(recv_buff.data);
                lwgsmi_parse_received(&recv_buff);
            }
#endif /* LWGSM_CFG_USSD */
            /*
             * We are in command mode where we have to process byte by byte
             * Simply check for ASCII and unicode format and process data accordingly
             */
        } else {
            lwgsmr_t res = lwgsmERR;
            if (LWGSM_ISVALIDASCII(ch)) {       /* Manually check if valid ASCII character */
                res = lwgsmOK;
                unicode.t = 1;                  /* Manually set total to 1 */
                unicode.r = 0;                  /* Reset remaining bytes */
            } else if (ch >= 0x80) {            /* Process only if more than ASCII can hold */
                res = lwgsmi_unicode_decode(&unicode, ch);  /* Try to decode unicode format */
            }

            if (res == lwgsmERR) {              /* In case of an ERROR */
                unicode.r = 0;
            }
            if (res == lwgsmOK) {               /* Can we process the character(s) */
                if (unicode.t == 1) {           /* Totally 1 character? */
                    switch (ch) {
                        case '\n':
                            RECV_ADD(ch);       /* Add character to input buffer */
                            lwgsmi_parse_received(&recv_buff);  /* Parse received string */
                            RECV_RESET();       /* Reset received string */
                            break;
                        default:
                            RECV_ADD(ch);       /* Any ASCII valid character */
                            break;
                    }

#if LWGSM_CFG_CONN
                    /* Check if we have to read data */
                    if (ch == '\n' && lwgsm.m.ipd.read) {
                        size_t len;
                        LWGSM_DEBUGF(LWGSM_CFG_DBG_IPD | LWGSM_DBG_TYPE_TRACE,
                                   "[IPD] Data on connection %d with total size %d byte(s)\r\n",
                                   (int)lwgsm.m.ipd.conn->num, (int)lwgsm.m.ipd.tot_len);

                        len = LWGSM_MIN(lwgsm.m.ipd.rem_len, LWGSM_CFG_IPD_MAX_BUFF_SIZE);

                        /*
                         * Read received data in case of:
                         *
                         *  - Connection is active and
                         *  - Connection is not in closing mode
                         */
                        if (lwgsm.m.ipd.conn->status.f.active && !lwgsm.m.ipd.conn->status.f.in_closing) {
                            lwgsm.m.ipd.buff = lwgsm_pbuf_new(len); /* Allocate new packet buffer */
                            LWGSM_DEBUGW(LWGSM_CFG_DBG_IPD | LWGSM_DBG_TYPE_TRACE | LWGSM_DBG_LVL_WARNING, lwgsm.m.ipd.buff == NULL,
                                       "[IPD] Buffer allocation failed for %d byte(s)\r\n", (int)len);
                        } else {
                            lwgsm.m.ipd.buff = NULL;/* Ignore reading on closed connection */
                            LWGSM_DEBUGF(LWGSM_CFG_DBG_IPD | LWGSM_DBG_TYPE_TRACE,
                                       "[IPD] Connection %d closed or in closing, skipping %d byte(s)\r\n",
                                       (int)lwgsm.m.ipd.conn->num, (int)len);
                        }
                        lwgsm.m.ipd.conn->status.f.data_received = 1;   /* We have first received data */

                        lwgsm.m.ipd.buff_ptr = 0;   /* Reset buffer write pointer */
                    }
#endif /* LWGSM_CFG_CONN */

                    /*
                     * Do we have a special sequence "> "?
                     *
                     * Check if any command active which may expect that kind of response
                     */
                    if (ch_prev2 == '\n' && ch_prev1 == '>' && ch == ' ') {
                        if (0) {
#if LWGSM_CFG_CONN
                        } else if (CMD_IS_CUR(LWGSM_CMD_CIPSEND)) {
                            RECV_RESET();       /* Reset received object */

                            /* Now actually send the data prepared before */
                            AT_PORT_SEND_WITH_FLUSH(&lwgsm.msg->msg.conn_send.data[lwgsm.msg->msg.conn_send.ptr], lwgsm.msg->msg.conn_send.sent);
                            lwgsm.msg->msg.conn_send.wait_send_ok_err = 1;  /* Now we are waiting for "SEND OK" or "SEND ERROR" */
#endif /* LWGSM_CFG_CONN */
#if LWGSM_CFG_SMS
                        } else if (CMD_IS_CUR(LWGSM_CMD_CMGS)) {/* Send SMS? */
                            AT_PORT_SEND(lwgsm.msg->msg.sms_send.text, strlen(lwgsm.msg->msg.sms_send.text));
                            AT_PORT_SEND_CTRL_Z();
                            AT_PORT_SEND_FLUSH();
#endif /* LWGSM_CFG_SMS */
                        }
                    } else if (CMD_IS_CUR(LWGSM_CMD_COPS_GET_OPT)) {
                        if (RECV_LEN() > 5 && !strncmp(recv_buff.data, "+COPS:", 6)) {
                            RECV_RESET();       /* Reset incoming buffer */
                            lwgsmi_parse_cops_scan(0, 1);   /* Reset parser state */
                            lwgsm.msg->msg.cops_scan.read = 1;  /* Start reading incoming bytes */
                        }
#if LWGSM_CFG_USSD
                    } else if (CMD_IS_CUR(LWGSM_CMD_CUSD)) {
                        if (RECV_LEN() > 5 && !strncmp(recv_buff.data, "+CUSD:", 6)) {
                            RECV_RESET();       /* Reset incoming buffer */
                            lwgsm.msg->msg.ussd.read = 1;   /* Start reading incoming bytes */
                        }
#endif /* LWGSM_CFG_USSD */
                    }
                } else {                        /* We have sequence of unicode characters */
                    /*
                     * Unicode sequence characters are not "meta" characters
                     * so it is safe to just add them to receive array without checking
                     * what are the actual values
                     */
                    for (uint8_t i = 0; i < unicode.t; ++i) {
                        RECV_ADD(unicode.ch[i]);/* Add character to receive array */
                    }
                }
            } else if (res != lwgsmINPROG) {    /* Not in progress? */
                RECV_RESET();                   /* Invalid character in sequence */
            }
        }

        ch_prev2 = ch_prev1;                    /* Save previous character as previous previous */
        ch_prev1 = ch;                          /* Set current as previous */
    }
    return lwgsmOK;
}

/* Temporary macros, only available for inside lwgsmi_process_sub_cmd function */
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
 * \return          \ref lwgsmCONT if you sent more data and we need to process more data, \ref lwgsmOK on success, or \ref lwgsmERR on error
 */
static lwgsmr_t
lwgsmi_process_sub_cmd(lwgsm_msg_t* msg, uint8_t* is_ok, uint16_t* is_error) {
    lwgsm_cmd_t n_cmd = LWGSM_CMD_IDLE;
    if (CMD_IS_DEF(LWGSM_CMD_RESET)) {
        switch (CMD_GET_CUR()) {                /* Check current command */
            case LWGSM_CMD_RESET: {
                lwgsmi_reset_everything(1);     /* Reset everything */
                SET_NEW_CMD(LWGSM_CFG_AT_ECHO ? LWGSM_CMD_ATE1 : LWGSM_CMD_ATE0);   /* Set ECHO mode */
                lwgsm_delay(LWGSM_CFG_RESET_DELAY_AFTER);    /* Delay for some time before we can continue after reset */
                break;
            }
            case LWGSM_CMD_ATE0:
            case LWGSM_CMD_ATE1:
                SET_NEW_CMD(LWGSM_CMD_CFUN_SET);
                break;                          /* Set full functionality */
            case LWGSM_CMD_CFUN_SET:
                SET_NEW_CMD(LWGSM_CMD_CMEE_SET);
                break;                          /* Set detailed error reporting */
            case LWGSM_CMD_CMEE_SET:
                SET_NEW_CMD(LWGSM_CMD_CGMI_GET);
                break;                          /* Get manufacturer */
            case LWGSM_CMD_CGMI_GET:
                SET_NEW_CMD(LWGSM_CMD_CGMM_GET);
                break;                          /* Get model */
            case LWGSM_CMD_CGMM_GET:
                SET_NEW_CMD(LWGSM_CMD_CGSN_GET);
                break;                          /* Get product serial number */
            case LWGSM_CMD_CGSN_GET:
                SET_NEW_CMD(LWGSM_CMD_CGMR_GET);
                break;                          /* Get product revision */
            case LWGSM_CMD_CGMR_GET: {
                /*
                 * At this point we have modem info.
                 * It is now time to send info to user
                 * to select between device drivers
                 */
                lwgsmi_send_cb(LWGSM_EVT_DEVICE_IDENTIFIED);

                SET_NEW_CMD(LWGSM_CMD_CREG_SET);/* Enable unsolicited code for CREG */
                break;
            }
            case LWGSM_CMD_CREG_SET:
                SET_NEW_CMD(LWGSM_CMD_CLCC_SET);
                break;                          /* Set call state */
            case LWGSM_CMD_CLCC_SET:
                SET_NEW_CMD(LWGSM_CMD_CPIN_GET);
                break;                          /* Get SIM state */
            case LWGSM_CMD_CPIN_GET:
                break;
            default:
                break;
        }

        /* Send event */
        if (n_cmd == LWGSM_CMD_IDLE) {
            RESET_SEND_EVT(msg, lwgsmOK);
        }
    } else if (CMD_IS_DEF(LWGSM_CMD_COPS_GET)) {
        if (CMD_IS_CUR(LWGSM_CMD_COPS_GET)) {
            lwgsm.evt.evt.operator_current.operator_current = &lwgsm.m.network.curr_operator;
            lwgsmi_send_cb(LWGSM_EVT_NETWORK_OPERATOR_CURRENT);
        }
    } else if (CMD_IS_DEF(LWGSM_CMD_COPS_GET_OPT)) {
        if (CMD_IS_CUR(LWGSM_CMD_COPS_GET_OPT)) {
            OPERATOR_SCAN_SEND_EVT(lwgsm.msg, *is_ok ? lwgsmOK : lwgsmERR);
        }
    } else if (CMD_IS_DEF(LWGSM_CMD_SIM_PROCESS_BASIC_CMDS)) {
        if (CMD_IS_CUR(LWGSM_CMD_CNUM)) {
            /* Get own phone number */
            if (!*is_ok) {
                /* Sometimes SIM is not ready just after PIN entered */
                if (msg->msg.sim_info.cnum_tries < 5) {
                    ++msg->msg.sim_info.cnum_tries;
                    SET_NEW_CMD(LWGSM_CMD_CNUM);
                    lwgsm_delay(1000);
                }
            }
        }
    } else if (CMD_IS_DEF(LWGSM_CMD_CPIN_SET)) {/* Set PIN code */
        switch (CMD_GET_CUR()) {
            case LWGSM_CMD_CPIN_GET: {          /* Get own phone number */
                if (*is_ok && lwgsm.m.sim.state == LWGSM_SIM_STATE_PIN) {
                    SET_NEW_CMD(LWGSM_CMD_CPIN_SET);/* Set command to write PIN */
                } else if (lwgsm.m.sim.state != LWGSM_SIM_STATE_READY) {
                    *is_ok = 0;
                    *is_error = 1;
                }
                break;
            }
            case LWGSM_CMD_CPIN_SET: {          /* Set CPIN */
                if (*is_ok) {
                    lwgsm_delay(5000);          /* Make delay to make sure SIM is ready */
                }
                break;
            }
            default:
                break;
        }
#if LWGSM_CFG_SMS
    } else if (CMD_IS_DEF(LWGSM_CMD_SMS_ENABLE)) {
        switch (CMD_GET_CUR()) {
            case LWGSM_CMD_CPMS_GET_OPT:
                SET_NEW_CMD(LWGSM_CMD_CPMS_GET);
                break;
            case LWGSM_CMD_CPMS_GET:
                break;
            default:
                break;
        }
        if (!*is_ok || n_cmd == LWGSM_CMD_IDLE) {   /* Stop execution on any command */
            SET_NEW_CMD(LWGSM_CMD_IDLE);
            lwgsm.m.sms.enabled = n_cmd == LWGSM_CMD_IDLE;  /* Set enabled status */
            lwgsm.evt.evt.sms_enable.status = lwgsm.m.sms.enabled ? lwgsmOK : lwgsmERR;
            lwgsmi_send_cb(LWGSM_EVT_SMS_ENABLE);   /* Send to user */
        }
    } else if (CMD_IS_DEF(LWGSM_CMD_CMGS)) {    /* Send SMS default command */
        if (CMD_IS_CUR(LWGSM_CMD_CMGF) && *is_ok) { /* Set message format current command */
            SET_NEW_CMD(LWGSM_CMD_CMGS);        /* Now send actual message */
        }

        /* Send event on finish */
        if (n_cmd == LWGSM_CMD_IDLE) {
            SMS_SEND_SEND_EVT(lwgsm.msg, *is_ok ? lwgsmOK : lwgsmERR);
        }
    } else if (CMD_IS_DEF(LWGSM_CMD_CMGR)) {    /* Read SMS message */
        if (CMD_IS_CUR(LWGSM_CMD_CPMS_GET) && *is_ok) {
            SET_NEW_CMD(LWGSM_CMD_CPMS_SET);    /* Set memory */
        } else if (CMD_IS_CUR(LWGSM_CMD_CPMS_SET) && *is_ok) {
            SET_NEW_CMD(LWGSM_CMD_CMGF);        /* Set text mode */
        } else if (CMD_IS_CUR(LWGSM_CMD_CMGF) && *is_ok) {  /* Set message format current command*/
            SET_NEW_CMD(LWGSM_CMD_CMGR);        /* Start message read */
        } else if (CMD_IS_CUR(LWGSM_CMD_CMGR) && *is_ok) {
            msg->msg.sms_read.mem = lwgsm.m.sms.mem[0].current; /* Set current memory */
        }

        /* Send event on finish */
        if (n_cmd == LWGSM_CMD_IDLE) {
            SMS_SEND_READ_EVT(lwgsm.msg, *is_ok ? lwgsmOK : lwgsmERR);
        }
    } else if (CMD_IS_DEF(LWGSM_CMD_CMGD)) {    /* Delete SMS message*/
        if (CMD_IS_CUR(LWGSM_CMD_CPMS_GET) && *is_ok) {
            SET_NEW_CMD(LWGSM_CMD_CPMS_SET);    /* Set memory */
        } else if (CMD_IS_CUR(LWGSM_CMD_CPMS_SET) && *is_ok) {
            SET_NEW_CMD(LWGSM_CMD_CMGD);        /* Delete message */
        }

        /* Send event on finish */
        if (n_cmd == LWGSM_CMD_IDLE) {
            SMS_SEND_DELETE_EVT(msg, *is_ok ? lwgsmOK : lwgsmERR);
        }
    } else if (CMD_IS_DEF(LWGSM_CMD_CMGDA)) {
        if (CMD_IS_CUR(LWGSM_CMD_CMGF) && *is_ok) {
            SET_NEW_CMD(LWGSM_CMD_CMGDA);       /* Mass storage */
        }
    } else if (CMD_IS_DEF(LWGSM_CMD_CMGL)) {    /* List SMS messages */
        if (CMD_IS_CUR(LWGSM_CMD_CPMS_GET) && *is_ok) {
            SET_NEW_CMD(LWGSM_CMD_CPMS_SET);    /* Set memory */
        } else if (CMD_IS_CUR(LWGSM_CMD_CPMS_SET) && *is_ok) {
            SET_NEW_CMD(LWGSM_CMD_CMGF);        /* Set text format */
        } else if (CMD_IS_CUR(LWGSM_CMD_CMGF) && *is_ok) {
            SET_NEW_CMD(LWGSM_CMD_CMGL);        /* List messages */
        }

        /* Send event on finish */
        if (n_cmd == LWGSM_CMD_IDLE) {
            SMS_SEND_LIST_EVT(msg, *is_ok ? lwgsmOK : lwgsmERR);
        }
    } else if (CMD_IS_DEF(LWGSM_CMD_CPMS_SET)) {/* Set preferred memory */
        if (CMD_IS_CUR(LWGSM_CMD_CPMS_GET) && *is_ok) {
            SET_NEW_CMD(LWGSM_CMD_CPMS_SET);    /* Now set the command */
        }
#endif /* LWGSM_CFG_SMS */
#if LWGSM_CFG_CALL
    } else if (CMD_IS_DEF(LWGSM_CMD_CALL_ENABLE)) {
        lwgsm.m.call.enabled = *is_ok;          /* Set enabled status */
        lwgsm.evt.evt.call_enable.res = lwgsm.m.call.enabled ? lwgsmOK : lwgsmERR;
        lwgsmi_send_cb(LWGSM_EVT_CALL_ENABLE);  /* Send to user */
#endif /* LWGSM_CFG_CALL */
#if LWGSM_CFG_PHONEBOOK
    } else if (CMD_IS_DEF(LWGSM_CMD_PHONEBOOK_ENABLE)) {
        lwgsm.m.pb.enabled = *is_ok;            /* Set enabled status */
        lwgsm.evt.evt.pb_enable.res = lwgsm.m.pb.enabled ? lwgsmOK : lwgsmERR;
        lwgsmi_send_cb(LWGSM_EVT_PB_ENABLE);    /* Send to user */
    } else if (CMD_IS_DEF(LWGSM_CMD_CPBW_SET)) {/* Write phonebook entry */
        if (CMD_IS_CUR(LWGSM_CMD_CPBS_GET) && *is_ok) { /* Get current memory */
            SET_NEW_CMD(LWGSM_CMD_CPBS_SET);    /* Set current memory */
        } else if (CMD_IS_CUR(LWGSM_CMD_CPBS_SET) && *is_ok) {
            SET_NEW_CMD(LWGSM_CMD_CPBW_SET);    /* Write entry to phonebook */
        }
    } else if (CMD_IS_DEF(LWGSM_CMD_CPBR)) {
        if (CMD_IS_CUR(LWGSM_CMD_CPBS_GET) && *is_ok) { /* Get current memory */
            SET_NEW_CMD(LWGSM_CMD_CPBS_SET);    /* Set current memory */
        } else if (CMD_IS_CUR(LWGSM_CMD_CPBS_SET) && *is_ok) {
            SET_NEW_CMD(LWGSM_CMD_CPBR);        /* Read entries */
        } else if (CMD_IS_CUR(LWGSM_CMD_CPBR)) {
            lwgsm.evt.evt.pb_list.mem = lwgsm.m.pb.mem.current;
            lwgsm.evt.evt.pb_list.entries = lwgsm.msg->msg.pb_list.entries;
            lwgsm.evt.evt.pb_list.size = lwgsm.msg->msg.pb_list.ei;
            lwgsm.evt.evt.pb_list.res = *is_ok ? lwgsmOK : lwgsmERR;
            lwgsmi_send_cb(LWGSM_EVT_PB_LIST);
        }
    } else if (CMD_IS_DEF(LWGSM_CMD_CPBF)) {
        if (CMD_IS_CUR(LWGSM_CMD_CPBS_GET) && *is_ok) { /* Get current memory */
            SET_NEW_CMD(LWGSM_CMD_CPBS_SET);    /* Set current memory */
        } else if (CMD_IS_CUR(LWGSM_CMD_CPBS_SET) && *is_ok) {
            SET_NEW_CMD(LWGSM_CMD_CPBF);        /* Read entries */
        } else if (CMD_IS_CUR(LWGSM_CMD_CPBF)) {
            lwgsm.evt.evt.pb_search.mem = lwgsm.m.pb.mem.current;
            lwgsm.evt.evt.pb_search.search = lwgsm.msg->msg.pb_search.search;
            lwgsm.evt.evt.pb_search.entries = lwgsm.msg->msg.pb_search.entries;
            lwgsm.evt.evt.pb_search.size = lwgsm.msg->msg.pb_search.ei;
            lwgsm.evt.evt.pb_search.res = *is_ok ? lwgsmOK : lwgsmERR;
            lwgsmi_send_cb(LWGSM_EVT_PB_SEARCH);
        }
#endif /* LWGSM_CFG_PHONEBOOK */
#if LWGSM_CFG_NETWORK
    }
    if (CMD_IS_DEF(LWGSM_CMD_NETWORK_ATTACH)) {
        switch (msg->i) {
            case 0:
                SET_NEW_CMD_CHECK_ERROR(LWGSM_CMD_CGACT_SET_0);
                break;
            case 1:
                SET_NEW_CMD(LWGSM_CMD_CGACT_SET_1);
                break;
#if LWGSM_CFG_NETWORK_IGNORE_CGACT_RESULT
            case 2:
                SET_NEW_CMD(LWGSM_CMD_CGATT_SET_0);
                break;
#else /* LWGSM_CFG_NETWORK_IGNORE_CGACT_RESULT */
            case 2:
                SET_NEW_CMD_CHECK_ERROR(LWGSM_CMD_CGATT_SET_0);
                break;
#endif /* !LWGSM_CFG_NETWORK_IGNORE_CGACT_RESULT */
            case 3:
                SET_NEW_CMD(LWGSM_CMD_CGATT_SET_1);
                break;
            case 4:
                SET_NEW_CMD_CHECK_ERROR(LWGSM_CMD_CIPSHUT);
                break;
            case 5:
                SET_NEW_CMD_CHECK_ERROR(LWGSM_CMD_CIPMUX_SET);
                break;
            case 6:
                SET_NEW_CMD_CHECK_ERROR(LWGSM_CMD_CIPRXGET_SET);
                break;
            case 7:
                SET_NEW_CMD_CHECK_ERROR(LWGSM_CMD_CSTT_SET);
                break;
            case 8:
                SET_NEW_CMD_CHECK_ERROR(LWGSM_CMD_CIICR);
                break;
            case 9:
                SET_NEW_CMD_CHECK_ERROR(LWGSM_CMD_CIFSR);
                break;
            case 10:
                SET_NEW_CMD(LWGSM_CMD_CIPSTATUS);
                break;
            default:
                break;
        }
    } else if (CMD_IS_DEF(LWGSM_CMD_NETWORK_DETACH)) {
        switch (msg->i) {
            case 0:
                SET_NEW_CMD(LWGSM_CMD_CGATT_SET_0);
                break;
            case 1:
                SET_NEW_CMD(LWGSM_CMD_CGACT_SET_0);
                break;
#if LWGSM_CFG_CONN
            case 2:
                SET_NEW_CMD(LWGSM_CMD_CIPSTATUS);
                break;
#endif /* LWGSM_CFG_CONN */
            default:
                break;
        }
        if (!n_cmd) {
            *is_ok = 1;
        }
#endif /* LWGSM_CFG_NETWORK */
#if LWGSM_CFG_CONN
    } else if (CMD_IS_DEF(LWGSM_CMD_CIPSTART)) {
        if (!msg->i && CMD_IS_CUR(LWGSM_CMD_CIPSTATUS)) {   /* Was the current command status info? */
            if (*is_ok) {
                SET_NEW_CMD(LWGSM_CMD_CIPSSL);  /* Set SSL */
            }
        } else if (msg->i == 1 && CMD_IS_CUR(LWGSM_CMD_CIPSSL)) {
            SET_NEW_CMD(LWGSM_CMD_CIPSTART);    /* Now actually start connection */
        } else if (msg->i == 2 && CMD_IS_CUR(LWGSM_CMD_CIPSTART)) {
            SET_NEW_CMD(LWGSM_CMD_CIPSTATUS);   /* Go to status mode */
            if (*is_error) {
                msg->msg.conn_start.conn_res = LWGSM_CONN_CONNECT_ERROR;
            }
        } else if (msg->i == 3 && CMD_IS_CUR(LWGSM_CMD_CIPSTATUS)) {
            /* After second CIP status, define what to do next */
            switch (msg->msg.conn_start.conn_res) {
                case LWGSM_CONN_CONNECT_OK: {   /* Successfully connected */
                    lwgsm_conn_t* conn = &lwgsm.m.conns[msg->msg.conn_start.num];   /* Get connection number */

                    lwgsm.evt.type = LWGSM_EVT_CONN_ACTIVE; /* Connection just active */
                    lwgsm.evt.evt.conn_active_close.client = 1;
                    lwgsm.evt.evt.conn_active_close.conn = conn;
                    lwgsm.evt.evt.conn_active_close.forced = 1;
                    lwgsmi_send_conn_cb(conn, NULL);
                    lwgsmi_conn_start_timeout(conn);/* Start connection timeout timer */
                    break;
                }
                case LWGSM_CONN_CONNECT_ERROR: {/* Connection error */
                    lwgsmi_send_conn_error_cb(msg, lwgsmERRCONNFAIL);
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
    } else if (CMD_IS_DEF(LWGSM_CMD_CIPCLOSE)) {
        /*
         * It is unclear in which state connection is when ERROR is received on close command.
         * Stack checks if connection is closed before it allows and sends close command,
         * however it was detected that no automatic close event has been received from device
         * and AT+CIPCLOSE returned ERROR.
         *
         * Is it device firmware bug?
         */
        if (CMD_IS_CUR(LWGSM_CMD_CIPCLOSE) && *is_error) {
            /* Notify upper layer about failed close event */
            lwgsm.evt.type = LWGSM_EVT_CONN_CLOSE;
            lwgsm.evt.evt.conn_active_close.conn = msg->msg.conn_close.conn;
            lwgsm.evt.evt.conn_active_close.forced = 1;
            lwgsm.evt.evt.conn_active_close.res = lwgsmERR;
            lwgsm.evt.evt.conn_active_close.client = msg->msg.conn_close.conn->status.f.active && msg->msg.conn_close.conn->status.f.client;
            lwgsmi_send_conn_cb(msg->msg.conn_close.conn, NULL);
        }
#endif /* LWGSM_CFG_CONN */
#if LWGSM_CFG_USSD
    } else if (CMD_IS_DEF(LWGSM_CMD_CUSD)) {
        if (CMD_IS_CUR(LWGSM_CMD_CUSD_GET)) {
            if (*is_ok) {
                SET_NEW_CMD(LWGSM_CMD_CUSD);    /* Run next command */
            }
        }
        /* The rest is handled in one layer above */
#endif /* LWGSM_CFG_USSD */
    }

    /* Check if new command was set for execution */
    if (n_cmd != LWGSM_CMD_IDLE) {
        lwgsmr_t res;
        msg->cmd = n_cmd;
        if ((res = msg->fn(msg)) == lwgsmOK) {
            return lwgsmCONT;
        } else {
            *is_ok = 0;
            *is_error = 1;
            return res;
        }
    } else {
        msg->cmd = LWGSM_CMD_IDLE;
    }
    return *is_ok ? lwgsmOK : lwgsmERR;
}

/**
 * \brief           Function to initialize every AT command
 * \note            Never call this function directly. Set as initialization function for command and use `msg->fn(msg)`
 * \param[in]       msg: Pointer to \ref lwgsm_msg_t with data
 * \return          Member of \ref lwgsmr_t enumeration
 */
lwgsmr_t
lwgsmi_initiate_cmd(lwgsm_msg_t* msg) {
    switch (CMD_GET_CUR()) {                    /* Check current message we want to send over AT */
        case LWGSM_CMD_RESET: {                 /* Reset modem with AT commands */
            /* Try with hardware reset */
            if (lwgsm.ll.reset_fn != NULL && lwgsm.ll.reset_fn(1)) {
                lwgsm_delay(2);
                lwgsm.ll.reset_fn(0);
                lwgsm_delay(500);
            }

            /* Send manual AT command */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CFUN=1,1");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_RESET_DEVICE_FIRST_CMD: {/* First command for device driver specific reset */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_ATE0:
        case LWGSM_CMD_ATE1: {
            AT_PORT_SEND_BEGIN_AT();
            if (CMD_IS_CUR(LWGSM_CMD_ATE0)) {
                AT_PORT_SEND_CONST_STR("E0");
            } else {
                AT_PORT_SEND_CONST_STR("E1");
            }
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CMEE_SET: {              /* Enable detailed error messages */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CMEE=1");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CLCC_SET: {              /* Enable detailed call info */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CLCC=1");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CGMI_GET: {              /* Get manufacturer */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CGMI");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CGMM_GET: {              /* Get model */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CGMM");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CGSN_GET: {              /* Get serial number */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CGSN");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CGMR_GET: {              /* Get revision */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CGMR");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CREG_SET: {              /* Enable +CREG message */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CREG=1");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CREG_GET: {              /* Get network registration status */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CREG?");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CFUN_SET: {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CFUN=");
            /**
             * \todo: If CFUN command forced, check value
             */
            if (CMD_IS_DEF(LWGSM_CMD_RESET)
                || (CMD_IS_DEF(LWGSM_CMD_CFUN_SET) && msg->msg.cfun.mode)) {
                AT_PORT_SEND_CONST_STR("1");
            } else {
                AT_PORT_SEND_CONST_STR("0");
            }
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CPIN_GET: {              /* Read current SIM status */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CPIN?");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CPIN_SET: {              /* Set SIM pin code */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CPIN=");
            lwgsmi_send_string(msg->msg.cpin_enter.pin, 0, 1, 0);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CPIN_ADD: {              /* Add new pin code */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CLCK=\"SC\",1,");
            lwgsmi_send_string(msg->msg.cpin_add.pin, 0, 1, 0);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CPIN_CHANGE: {           /* Change already active SIM */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CPWD=\"SC\"");
            lwgsmi_send_string(msg->msg.cpin_change.current_pin, 0, 1, 1);
            lwgsmi_send_string(msg->msg.cpin_change.new_pin, 0, 1, 1);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CPIN_REMOVE: {           /* Remove current PIN */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CLCK=\"SC\",0,");
            lwgsmi_send_string(msg->msg.cpin_remove.pin, 0, 1, 0);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CPUK_SET: {              /* Enter PUK and set new PIN */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CPIN=");
            lwgsmi_send_string(msg->msg.cpuk_enter.puk, 0, 1, 0);
            lwgsmi_send_string(msg->msg.cpuk_enter.pin, 0, 1, 1);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_COPS_SET: {              /* Set current operator */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+COPS=");
            lwgsmi_send_number(LWGSM_U32(msg->msg.cops_set.mode), 0, 0);
            if (msg->msg.cops_set.mode != LWGSM_OPERATOR_MODE_AUTO) {
                lwgsmi_send_number(LWGSM_U32(msg->msg.cops_set.format), 0, 1);
                switch (msg->msg.cops_set.format) {
                    case LWGSM_OPERATOR_FORMAT_LONG_NAME:
                    case LWGSM_OPERATOR_FORMAT_SHORT_NAME:
                        lwgsmi_send_string(msg->msg.cops_set.name, 1, 1, 1);
                        break;
                    default:
                        lwgsmi_send_number(LWGSM_U32(msg->msg.cops_set.num), 0, 1);
                }
            }
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_COPS_GET: {              /* Get current operator */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+COPS?");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_COPS_GET_OPT: {          /* Get list of available operators */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+COPS=?");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CSQ_GET: {               /* Get signal strength */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CSQ");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CNUM: {                  /* Get SIM number */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CNUM");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CIPSHUT: {               /* Shut down network connection and put to reset state */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CIPSHUT");
            AT_PORT_SEND_END_AT();
            break;
        }
#if LWGSM_CFG_CONN
        case LWGSM_CMD_CIPMUX: {                /* Enable multiple connections */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CIPMUX=1");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CIPHEAD: {               /* Enable information on receive data about connection and length */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CIPHEAD=1");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CIPSRIP: {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CIPSRIP=1");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CIPSSL: {                /* Set SSL configuration */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CIPSSL=");
            lwgsmi_send_number((msg->msg.conn_start.type == LWGSM_CONN_TYPE_SSL) ? 1 : 0, 0, 0);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CIPSTART: {              /* Start a new connection */
            lwgsm_conn_t* c = NULL;

            /* Do we have network connection? */
            /* Check if we are connected to network */

            msg->msg.conn_start.num = 0;        /* Start with max value = invalidated */
            for (int16_t i = LWGSM_CFG_MAX_CONNS - 1; i >= 0; --i) {/* Find available connection */
                if (!lwgsm.m.conns[i].status.f.active) {
                    c = &lwgsm.m.conns[i];
                    c->num = LWGSM_U8(i);
                    msg->msg.conn_start.num = LWGSM_U8(i);  /* Set connection number for message structure */
                    break;
                }
            }
            if (c == NULL) {
                lwgsmi_send_conn_error_cb(msg, lwgsmERRNOFREECONN);
                return lwgsmERRNOFREECONN;      /* We don't have available connection */
            }

            if (msg->msg.conn_start.conn != NULL) { /* Is user interested about connection info? */
                *msg->msg.conn_start.conn = c;  /* Save connection for user */
            }

            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CIPSTART=");
            lwgsmi_send_number(LWGSM_U32(c->num), 0, 0);
            if (msg->msg.conn_start.type == LWGSM_CONN_TYPE_UDP) {
                lwgsmi_send_string("UDP", 0, 1, 1);
            } else {
                lwgsmi_send_string("TCP", 0, 1, 1);
            }
            lwgsmi_send_string(msg->msg.conn_start.host, 0, 1, 1);
            lwgsmi_send_port(msg->msg.conn_start.port, 0, 1);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CIPCLOSE: {              /* Close the connection */
            lwgsm_conn_p c = msg->msg.conn_close.conn;
            if (c != NULL &&
                /* Is connection already closed or command for this connection is not valid anymore? */
                (!lwgsm_conn_is_active(c) || c->val_id != msg->msg.conn_close.val_id)) {
                return lwgsmERR;
            }
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CIPCLOSE=");
            lwgsmi_send_number(LWGSM_U32(msg->msg.conn_close.conn ? msg->msg.conn_close.conn->num : LWGSM_CFG_MAX_CONNS), 0, 0);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CIPSEND: {               /* Send data to connection */
            return lwgsmi_tcpip_process_send_data();/* Process send data */
        }
        case LWGSM_CMD_CIPSTATUS: {             /* Get status of device and all connections */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CIPSTATUS");
            AT_PORT_SEND_END_AT();
            break;
        }
#endif /* LWGSM_CFG_CONN */
#if LWGSM_CFG_SMS
        case LWGSM_CMD_CMGF: {                  /* Select SMS message format */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CMGF=");
            if (CMD_IS_DEF(LWGSM_CMD_CMGS)) {
                lwgsmi_send_number(LWGSM_U32(!!msg->msg.sms_send.format), 0, 0);
            } else if (CMD_IS_DEF(LWGSM_CMD_CMGR)) {
                lwgsmi_send_number(LWGSM_U32(!!msg->msg.sms_read.format), 0, 0);
            } else if (CMD_IS_DEF(LWGSM_CMD_CMGL)) {
                lwgsmi_send_number(LWGSM_U32(!!msg->msg.sms_list.format), 0, 0);
            } else {
                /* Used for all other operations like delete all messages, etc */
                AT_PORT_SEND_CONST_STR("1");
            }
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CMGS: {                  /* Send SMS */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CMGS=");
            lwgsmi_send_string(msg->msg.sms_send.num, 0, 1, 0);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CMGR: {                  /* Read message */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CMGR=");
            lwgsmi_send_number(LWGSM_U32(msg->msg.sms_read.pos), 0, 0);
            lwgsmi_send_number(LWGSM_U32(!msg->msg.sms_read.update), 0, 1);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CMGD: {                  /* Delete SMS message */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CMGD=");
            lwgsmi_send_number(LWGSM_U32(msg->msg.sms_delete.pos), 0, 0);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CMGDA: {                 /* Mass delete SMS messages */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CMGDA=");
            switch (msg->msg.sms_delete_all.status) {
                case LWGSM_SMS_STATUS_READ:
                    lwgsmi_send_string("DEL READ", 0, 1, 0);
                    break;
                case LWGSM_SMS_STATUS_UNREAD:
                    lwgsmi_send_string("DEL UNREAD", 0, 1, 0);
                    break;
                case LWGSM_SMS_STATUS_SENT:
                    lwgsmi_send_string("DEL SENT", 0, 1, 0);
                    break;
                case LWGSM_SMS_STATUS_UNSENT:
                    lwgsmi_send_string("DEL UNSENT", 0, 1, 0);
                    break;
                case LWGSM_SMS_STATUS_INBOX:
                    lwgsmi_send_string("DEL INBOX", 0, 1, 0);
                    break;
                case LWGSM_SMS_STATUS_ALL:
                    lwgsmi_send_string("DEL ALL", 0, 1, 0);
                    break;
                default:
                    break;
            }
            AT_PORT_SEND_END_AT();
            break;
        }

        case LWGSM_CMD_CMGL: {                  /* Delete SMS message */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CMGL=");
            lwgsmi_send_sms_stat(msg->msg.sms_list.status, 1, 0);
            lwgsmi_send_number(LWGSM_U32(!msg->msg.sms_list.update), 0, 1);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CPMS_GET_OPT: {          /* Get available SMS storages */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CPMS=?");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CPMS_GET: {              /* Get current SMS storage info */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CPMS?");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CPMS_SET: {              /* Set active SMS storage(s) */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CPMS=");
            if (CMD_IS_DEF(LWGSM_CMD_CMGR)) {   /* Read SMS original command? */
                lwgsmi_send_dev_memory(msg->msg.sms_read.mem == LWGSM_MEM_CURRENT ? lwgsm.m.sms.mem[0].current : msg->msg.sms_read.mem, 1, 0);
            } else if (CMD_IS_DEF(LWGSM_CMD_CMGD)) {/* Delete SMS original command? */
                lwgsmi_send_dev_memory(msg->msg.sms_delete.mem == LWGSM_MEM_CURRENT ? lwgsm.m.sms.mem[0].current : msg->msg.sms_delete.mem, 1, 0);
            } else if (CMD_IS_DEF(LWGSM_CMD_CMGL)) {/* List SMS original command? */
                lwgsmi_send_dev_memory(msg->msg.sms_list.mem == LWGSM_MEM_CURRENT ? lwgsm.m.sms.mem[0].current : msg->msg.sms_list.mem, 1, 0);
            } else if (CMD_IS_DEF(LWGSM_CMD_CPMS_SET)) {/* Do we want to set memory for read/delete,sent/write,receive? */
                for (size_t i = 0; i < 3; ++i) {/* Write 3 memories */
                    lwgsmi_send_dev_memory(msg->msg.sms_memory.mem[i] == LWGSM_MEM_CURRENT ? lwgsm.m.sms.mem[i].current : msg->msg.sms_memory.mem[i], 1, !!i);
                }
            }
            AT_PORT_SEND_END_AT();
            break;
        }
#endif /* LWGSM_CFG_SMS */
#if LWGSM_CFG_CALL
        case LWGSM_CMD_ATD: {                   /* Start new call */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("D");
            lwgsmi_send_string(msg->msg.call_start.number, 0, 0, 0);
            AT_PORT_SEND_CONST_STR(";");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_ATA: {                   /* Answer phone call */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("A");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_ATH: {                   /* Disconnect existing connection (hang-up phone call) */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("H");
            AT_PORT_SEND_END_AT();
            break;
        }
#endif /* LWGSM_CFG_CALL */
#if LWGSM_CFG_PHONEBOOK
        case LWGSM_CMD_CPBS_GET_OPT: {          /* Get available phonebook storages */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CPBS=?");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CPBS_GET: {              /* Get current memory info */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CPBS?");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CPBS_SET: {              /* Get current memory info */
            lwgsm_mem_t mem;
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CPBS=");
            switch (CMD_GET_DEF()) {
                case LWGSM_CMD_CPBW_SET:
                    mem = msg->msg.pb_write.mem;
                    break;
                case LWGSM_CMD_CPBR:
                    mem = msg->msg.pb_list.mem;
                    break;
                case LWGSM_CMD_CPBF:
                    mem = msg->msg.pb_search.mem;
                    break;
                default:
                    break;
            }
            lwgsmi_send_dev_memory(mem == LWGSM_MEM_CURRENT ? lwgsm.m.pb.mem.current : mem, 1, 0);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CPBW_SET: {              /* Write/Delete new/old entry */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CPBW=");
            if (msg->msg.pb_write.pos > 0) {    /* Write number if more than 0 */
                lwgsmi_send_number(LWGSM_U32(msg->msg.pb_write.pos), 0, 0);
            }
            if (!msg->msg.pb_write.del) {
                lwgsmi_send_string(msg->msg.pb_write.num, 0, 1, 1);
                lwgsmi_send_number(LWGSM_U32(msg->msg.pb_write.type), 0, 1);
                lwgsmi_send_string(msg->msg.pb_write.name, 0, 1, 1);
            }
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CPBR: {                  /* Read entires */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CPBR=");
            lwgsmi_send_number(LWGSM_U32(msg->msg.pb_list.start_index), 0, 0);
            lwgsmi_send_number(LWGSM_U32(msg->msg.pb_list.etr), 0, 1);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CPBF: {                  /* Find entires */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CPBF=");
            lwgsmi_send_string(msg->msg.pb_search.search, 1, 1, 0);
            AT_PORT_SEND_END_AT();
            break;
        }
#endif /* LWGSM_CFG_PHONEBOOK */
#if LWGSM_CFG_NETWORK
        case LWGSM_CMD_NETWORK_ATTACH:
        case LWGSM_CMD_CGACT_SET_0: {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CGACT=0");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CGACT_SET_1: {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CGACT=1");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_NETWORK_DETACH:
        case LWGSM_CMD_CGATT_SET_0: {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CGATT=0");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CGATT_SET_1: {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CGATT=1");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CIPMUX_SET: {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CIPMUX=1");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CIPRXGET_SET: {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CIPRXGET=0");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CSTT_SET: {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CSTT=");
            lwgsmi_send_string(msg->msg.network_attach.apn, 1, 1, 0);
            lwgsmi_send_string(msg->msg.network_attach.user, 1, 1, 1);
            lwgsmi_send_string(msg->msg.network_attach.pass, 1, 1, 1);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CIICR: {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CIICR");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CIFSR: {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CIFSR");
            AT_PORT_SEND_END_AT();
            break;
        }
#endif /* LWGSM_CFG_NETWORK */
#if LWGSM_CFG_USSD
        case LWGSM_CMD_CUSD_GET: {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CUSD?");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CUSD: {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CUSD=1,");
            lwgsmi_send_string(msg->msg.ussd.code, 1, 1, 0);
            AT_PORT_SEND_END_AT();
            break;
        }
#endif /* LWGSM_CFG_USSD */
        default:
            return lwgsmERR;                    /* Invalid command */
    }
    return lwgsmOK;                             /* Valid command */
}

/**
 * \brief           Send message from API function to producer queue for further processing
 * \param[in]       msg: New message to process
 * \param[in]       process_fn: callback function used to process message
 * \param[in]       max_block_time: Maximal time command can block in units of milliseconds
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsmi_send_msg_to_producer_mbox(lwgsm_msg_t* msg, lwgsmr_t (*process_fn)(lwgsm_msg_t*), uint32_t max_block_time) {
    lwgsmr_t res = msg->res = lwgsmOK;

    /* Check here if stack is even enabled or shall we disable new command entry? */
    lwgsm_core_lock();
    /* If locked more than 1 time, means we were called from callback or internally */
    if (lwgsm.locked_cnt > 1 && msg->is_blocking) {
        res = lwgsmERRBLOCKING;                 /* Blocking mode not allowed */
    }
    /* Check if device present */
    if (res == lwgsmOK && !lwgsm.status.f.dev_present) {
        res = lwgsmERRNODEVICE;                 /* No device connected */
    }
    lwgsm_core_unlock();
    if (res != lwgsmOK) {
        LWGSM_MSG_VAR_FREE(msg);                /* Free memory and return */
        return res;
    }

    if (msg->is_blocking) {                     /* In case message is blocking */
        if (!lwgsm_sys_sem_create(&msg->sem, 0)) {  /* Create semaphore and lock it immediately */
            LWGSM_MSG_VAR_FREE(msg);            /* Release memory and return */
            return lwgsmERRMEM;
        }
    }
    if (!msg->cmd) {                            /* Set start command if not set by user */
        msg->cmd = msg->cmd_def;                /* Set it as default */
    }
    msg->block_time = max_block_time;           /* Set blocking status if necessary */
    msg->fn = process_fn;                       /* Save processing function to be called as callback */
    if (msg->is_blocking) {
        lwgsm_sys_mbox_put(&lwgsm.mbox_producer, msg);  /* Write message to producer queue and wait forever */
    } else {
        if (!lwgsm_sys_mbox_putnow(&lwgsm.mbox_producer, msg)) {/* Write message to producer queue immediately */
            LWGSM_MSG_VAR_FREE(msg);            /* Release message */
            return lwgsmERRMEM;
        }
    }
    if (res == lwgsmOK && msg->is_blocking) {   /* In case we have blocking request */
        uint32_t time;
        time = lwgsm_sys_sem_wait(&msg->sem, 0);/* Wait forever for semaphore */
        if (time == LWGSM_SYS_TIMEOUT) {        /* If semaphore was not accessed within given time */
            res = lwgsmTIMEOUT;                 /* Semaphore not released in time */
        } else {
            res = msg->res;                     /* Set response status from message response */
        }
        LWGSM_MSG_VAR_FREE(msg);                /* Release message */
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
lwgsmi_process_events_for_timeout_or_error(lwgsm_msg_t* msg, lwgsmr_t err) {
    switch (msg->cmd_def) {
        case LWGSM_CMD_RESET: {
            /* Reset command error */
            RESET_SEND_EVT(msg, err);
            break;
        }

        case LWGSM_CMD_RESTORE: {
            /* Restore command error */
            RESTORE_SEND_EVT(msg, err);
            break;
        }

        case LWGSM_CMD_COPS_GET_OPT: {
            /* Operator scan command error */
            OPERATOR_SCAN_SEND_EVT(msg, err);
            break;
        }

#if LWGSM_CFG_CONN
        case LWGSM_CMD_CIPSTART: {
            /* Start connection error */
            lwgsmi_send_conn_error_cb(msg, err);
            break;
        }

        case LWGSM_CMD_CIPSEND: {
            /* Send data error event */
            CONN_SEND_DATA_SEND_EVT(msg, err);
            break;
        }
#endif /* LWGSM_CFG_CONN */

#if LWGSM_CFG_SMS
        case LWGSM_CMD_CMGS: {
            /* Send error event */
            SMS_SEND_SEND_EVT(msg, err);
            break;
        }

        case LWGSM_CMD_CMGR: {
            /* Read error event */
            SMS_SEND_READ_EVT(msg, err);
            break;
        }

        case LWGSM_CMD_CMGL: {
            /* List error event */
            SMS_SEND_LIST_EVT(msg, err);
            break;
        }

        case LWGSM_CMD_CMGD: {
            /* Delete error event */
            SMS_SEND_DELETE_EVT(msg, err);
            break;
        }
#endif /* LWGSM_CFG_SMS */

        default:
            break;
    }
}
