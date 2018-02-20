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
 * This file is part of GSM-AT.
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

#define IS_CURR_CMD(c)      (gsm.msg != NULL && gsm.msg->cmd == (c))
#define CH_CTRL_Z           (0x1A)
#define CH_ESC              (0x1A)

#if !__DOXYGEN__
typedef struct {
    char data[128];
    uint8_t len;
} gsm_recv_t;
static gsm_recv_t recv_buff;
#endif /* !__DOXYGEN__ */

#define CRLF                "\r\n"
#define CRLF_LEN            2

#define RECV_ADD(ch)        do { recv_buff.data[recv_buff.len++] = ch; recv_buff.data[recv_buff.len] = 0; } while (0)
#define RECV_RESET()        do { recv_buff.len = 0; recv_buff.data[0] = 0; } while (0)
#define RECV_LEN()          recv_buff.len
#define RECV_IDX(index)     recv_buff.data[index]

#define GSM_AT_PORT_SEND_BEGIN()        do { GSM_AT_PORT_SEND_STR("AT"); } while (0)
#define GSM_AT_PORT_SEND_END()          do { GSM_AT_PORT_SEND_STR(CRLF); } while (0)

#define GSM_AT_PORT_SEND_STR(str)       gsm.ll.send_fn((const uint8_t *)(str), (uint16_t)strlen(str))
#define GSM_AT_PORT_SEND_CHR(ch)        gsm.ll.send_fn((const uint8_t *)(ch), (uint16_t)1)
#define GSM_AT_PORT_SEND(d, l)          gsm.ll.send_fn((const uint8_t *)(d), (uint16_t)(l))

#define GSM_AT_PORT_SEND_QUOTE_COND(q)  do { if ((q)) { GSM_AT_PORT_SEND_STR("\""); } } while (0)
#define GSM_AT_PORT_SEND_COMMA_COND(c)  do { if ((c)) { GSM_AT_PORT_SEND_STR(","); } } while (0)
#define GSM_AT_PORT_SEND_EQUAL_COND(e)  do { if ((e)) { GSM_AT_PORT_SEND_STR("="); } } while (0)

#define GSM_AT_PORT_SEND_CTRL_Z()       GSM_AT_PORT_SEND_STR("\x1A")
#define GSM_AT_PORT_SEND_ESC()          GSM_AT_PORT_SEND_STR("\x1B")

#define GSM_PORT2NUM(port)              ((uint32_t)(port))

static gsmr_t gsmi_process_sub_cmd(gsm_msg_t* msg, uint8_t is_ok, uint16_t is_error, uint8_t is_ready);

/**
 * \brief           Memory map
 */
gsm_dev_mem_map_t
gsm_dev_mem_map[] = {
#define GSM_DEV_MEMORY_ENTRY(name, str_code)    { GSM_MEM_ ## name, str_code },
#include "gsm/gsm_memories.h"
};

/**
 * \brief           Size of device memory mapping array
 */
size_t
gsm_dev_mem_map_size = GSM_ARRAYSIZE(gsm_dev_mem_map);

/**
 * \brief           Free connection send data memory
 * \param[in]       m: Send data message type
 */
#define CONN_SEND_DATA_FREE(m)    do {              \
    if ((m) != NULL && (m)->msg.conn_send.fau) {    \
        (m)->msg.conn_send.fau = 0;                 \
        GSM_DEBUGF(GSM_CFG_DBG_CONN | GSM_DBG_TYPE_TRACE, "CONN: Free write buffer fau: %p\r\n", (void *)(m)->msg.conn_send.data);   \
        gsm_mem_free((void *)(m)->msg.conn_send.data);    \
    }                                               \
} while (0)

/**
 * \brief           Get SIM info when SIM is ready
 * \param[in]       blocking: Blocking command
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
gsmr_t
gsmi_get_sim_info(uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);                    /* Define variable for message */

    GSM_MSG_VAR_ALLOC(msg);                     /* Allocate memory for variable */
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_SIM_PROCESS_BASIC_CMDS;
    GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CNUM;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, blocking, 60000);   /* Send message to producer queue */
}

/**
 * \brief           Create 2-characters long hex from byte
 * \param[in]       num: Number to convert to string
 * \param[out]      str: Pointer to string to save result to
 */
static void
byte_to_str(uint8_t num, char* str) {
    sprintf(str, "%02X", (unsigned)num);        /* Currently use sprintf only */
}

/**
 * \brief           Create string from number
 * \param[in]       num: Number to convert to string
 * \param[out]      str: Pointer to string to save result to
 */
static void
number_to_str(uint32_t num, char* str) {
    sprintf(str, "%u", (unsigned)num);          /* Currently use sprintf only */
}

/**
 * \brief           Create string from signed number
 * \param[in]       num: Number to convert to string
 * \param[out]      str: Pointer to string to save result to
 */
static void
signed_number_to_str(int32_t num, char* str) {
    sprintf(str, "%d", (signed)num);            /* Currently use sprintf only */
}

/**
 * \brief           Send IP or MAC address to AT port
 * \param[in]       d: Pointer to IP or MAC address
 * \param[in]       is_ip: Set to `1` when sending IP, `0` when MAC
 * \param[in]       q: Set to `1` to include start and ending quotes
 * \param[in]       c: Set to `1` to include comma before string
 */
static void
send_ip_mac(const void* d, uint8_t is_ip, uint8_t q, uint8_t c) {
    uint8_t i, ch;
    char str[4];
    const gsm_mac_t* mac = d;
    const gsm_ip_t* ip = d;

    GSM_AT_PORT_SEND_COMMA_COND(c);             /* Send comma */
    if (d == NULL) {
        return;
    }
    GSM_AT_PORT_SEND_QUOTE_COND(q);             /* Send quote */
    ch = is_ip ? '.' : ':';                     /* Get delimiter character */
    for (i = 0; i < (is_ip ? 4 : 6); i++) {     /* Process byte by byte */
        if (is_ip) {                            /* In case of IP ... */
            number_to_str(ip->ip[i], str);      /* ... go to decimal format ... */
        } else {                                /* ... in case of MAC ... */
            byte_to_str(mac->mac[i], str);      /* ... go to HEX format */
        }
        GSM_AT_PORT_SEND_STR(str);              /* Send str */
        if (i < (is_ip ? 4 : 6) - 1) {          /* Check end if characters */
            GSM_AT_PORT_SEND_CHR(&ch);          /* Send character */
        }
    }
    GSM_AT_PORT_SEND_QUOTE_COND(q);             /* Send quote */
}

/**
 * \brief           Send string to AT port, either plain or escaped
 * \param[in]       str: Pointer to input string to string
 * \param[in]       e: Value to indicate string send format, escaped (`1`) or plain (`0`)
 * \param[in]       q: Value to indicate starting and ending quotes, enabled (`1`) or disabled (`0`)
 * \param[in]       c: Set to `1` to include comma before string
 */
static void
send_string(const char* str, uint8_t e, uint8_t q, uint8_t c) {
    char special = '\\';
    
    GSM_AT_PORT_SEND_COMMA_COND(c);             /* Send comma */
    GSM_AT_PORT_SEND_QUOTE_COND(q);             /* Send quote */
    if (str != NULL) {
        if (e) {                                /* Do we have to escape string? */
            while (*str) {                      /* Go through string */
                if (*str == ',' || *str == '"' || *str == '\\') {   /* Check for special character */    
                    GSM_AT_PORT_SEND_CHR(&special); /* Send special character */
                }
                GSM_AT_PORT_SEND_CHR(str);      /* Send character */
                str++;
            }
        } else {
            GSM_AT_PORT_SEND_STR(str);          /* Send plain string */
        }
    }
    GSM_AT_PORT_SEND_QUOTE_COND(q);             /* Send quote */
}

/**
 * \brief           Send number (decimal) to AT port
 * \param[in]       num: Number to send to AT port
 * \param[in]       q: Value to indicate starting and ending quotes, enabled (`1`) or disabled (`0`)
 * \param[in]       c: Set to `1` to include comma before string
 */
static void
send_number(uint32_t num, uint8_t q, uint8_t c) {
    char str[11];

    number_to_str(num, str);                    /* Convert digit to decimal string */
    
    GSM_AT_PORT_SEND_COMMA_COND(c);             /* Send comma */
    GSM_AT_PORT_SEND_QUOTE_COND(q);             /* Send quote */
    GSM_AT_PORT_SEND_STR(str);                  /* Send string with number */
    GSM_AT_PORT_SEND_QUOTE_COND(q);             /* Send quote */
}

/**
 * \brief           Send port number to AT port
 * \param[in]       port: Port number to send
 * \param[in]       q: Value to indicate starting and ending quotes, enabled (`1`) or disabled (`0`)
 * \param[in]       c: Set to `1` to include comma before string
 */
static void
send_port(gsm_port_t port, uint8_t q, uint8_t c) {
    char str[6];

    number_to_str(GSM_PORT2NUM(port), str);     /* Convert digit to decimal string */
    
    GSM_AT_PORT_SEND_COMMA_COND(c);             /* Send comma */
    GSM_AT_PORT_SEND_QUOTE_COND(q);             /* Send quote */
    GSM_AT_PORT_SEND_STR(str);                  /* Send string with number */
    GSM_AT_PORT_SEND_QUOTE_COND(q);             /* Send quote */
}

/**
 * \brief           Send signed number to AT port
 * \param[in]       num: Number to send to AT port
 * \param[in]       q: Value to indicate starting and ending quotes, enabled (`1`) or disabled (`0`)
 * \param[in]       c: Set to `1` to include comma before string
 */
void
send_signed_number(int32_t num, uint8_t q, uint8_t c) {
    char str[11];
    
    signed_number_to_str(num, str);             /* Convert digit to decimal string */
    
    GSM_AT_PORT_SEND_COMMA_COND(c);             /* Send comma */
    GSM_AT_PORT_SEND_QUOTE_COND(q);             /* Send quote */
    GSM_AT_PORT_SEND_STR(str);                  /* Send string with number */
    GSM_AT_PORT_SEND_QUOTE_COND(q);             /* Send quote */
}

/**
 * \brief           Send memory string to device
 * \param[in]       mem: Memory index to send
 * \param[in]       q: Value to indicate starting and ending quotes, enabled (`1`) or disabled (`0`)
 * \param[in]       c: Set to `1` to include comma before string
 */
void
send_dev_memory(gsm_mem_t mem, uint8_t q, uint8_t c) {
    size_t i = GSM_SZ(mem);
    if (mem < GSM_MEM_END) {                    /* Check valid range */
        send_string(gsm_dev_mem_map[i].mem_str, 0, q, c);
    }
}

#if GSM_CFG_SMS || __DOXYGEN__

/**
 * \brief           Send SMS status text
 * \param[in]       q: Value to indicate starting and ending quotes, enabled (`1`) or disabled (`0`)
 * \param[in]       c: Set to `1` to include comma before string
 */
static void
send_sms_stat(gsm_sms_status_t status, uint8_t q, uint8_t c) {
    const char* t = NULL;
    switch (status) {
        case GSM_SMS_STATUS_UNREAD: t = "REC UNREAD";   break;
        case GSM_SMS_STATUS_READ:   t = "REC READ";     break;
        case GSM_SMS_STATUS_UNSENT: t = "STO UNSENT";   break;
        case GSM_SMS_STATUS_SENT:   t = "STO SENT";     break;
        case GSM_SMS_STATUS_ALL:
        default:                    t = "ALL";          break;
    }
    send_string(t, 0, q, c);
}

#endif /* GSM_CFG_SMS */

/**
 * \brief           Reset all connections
 * \note            Used to notify upper layer stack to close everything and reset the memory if necessary
 * \param[in]       forced: Flag indicating reset was forced by user
 */
static void
reset_connections(uint8_t forced) {
    //size_t i;
    //
    //gsm.cb.type = GSM_CB_CONN_CLOSED;
    //gsm.cb.cb.conn_active_closed.forced = forced;
    //
    //for (i = 0; i < GSM_CFG_MAX_CONNS; i++) {   /* Check all connections */
    //    if (gsm.conns[i].status.f.active) {
    //        gsm.conns[i].status.f.active = 0;
    //        
    //        gsm.cb.cb.conn_active_closed.conn = &gsm.conns[i];
    //        gsm.cb.cb.conn_active_closed.client = gsm.conns[i].status.f.client;
    //        gsmi_send_conn_cb(&gsm.conns[i], NULL); /* Send callback function */
    //    }
    //}
}

/**
 * \brief           Process callback function to user with specific type
 * \param[in]       type: Callback event type
 * \return          Member of \ref gsmr_t enumeration
 */
gsmr_t
gsmi_send_cb(gsm_cb_type_t type) {
    gsm_cb_func_t* link;
    gsm.cb.type = type;                         /* Set callback type to process */
    
    /*
     * Call callback function for all registered functions
     */
    for (link = gsm.cb_func; link != NULL; link = link->next) {
        link->fn(&gsm.cb);
    }
    return gsmOK;
}

///**
// * \brief           Process and send data from device buffer
// * \return          Member of \ref gsmr_t enumeration
// */
//static gsmr_t
//gsmi_tcpip_process_send_data(void) {
//    if (!gsm_conn_is_active(gsm.msg->msg.conn_send.conn) || /* Is the connection already closed? */
//        gsm.msg->msg.conn_send.val_id != gsm.msg->msg.conn_send.conn->val_id    /* Did validation ID change after we set parameter? */
//    ) {
//        CONN_SEND_DATA_FREE(gsm.msg);           /* Free message data */
//        return gsmERR;
//    }
//    GSM_AT_PORT_SEND_STR("AT+CIPSEND=");
//    send_number(gsm.msg->msg.conn_send.conn->num, 0);
//    GSM_AT_PORT_SEND_STR(",");
//    gsm.msg->msg.conn_send.sent = GSM_MIN(gsm.msg->msg.conn_send.btw, GSM_CFG_CONN_MAX_DATA_LEN);
//    send_number(gsm.msg->msg.conn_send.sent, 0);    /* Send length number */
//    
//    /*
//     * On UDP connections, IP address and port may be selected
//     */
//    if (gsm.msg->msg.conn_send.conn->type == GSM_CONN_TYPE_UDP) {
//        const gsm_ip_t* ip = gsm.msg->msg.conn_send.remote_ip;  /* Get remote IP */
//        gsm_port_t port = gsm.msg->msg.conn_send.remote_port;
//        
//        if (ip != NULL && port) {
//            GSM_AT_PORT_SEND_STR(",");
//            send_ip_mac(ip, 1, 1);              /* Send IP address including quotes */
//            GSM_AT_PORT_SEND_STR(",");
//            send_number(port, 0);               /* Send length number */
//        }
//    }
//    GSM_AT_PORT_SEND_STR(CRLF);
//    return gsmOK;
//}
//
///**
// * \brief           Process data sent and send remaining
// * \param[in]       sent: Status whether data were sent or not, info received from GSM with "SEND OK" or "SEND FAIL" 
// * \return          1 in case we should stop sending or 0 if we still have data to process
// */
//static uint8_t
//gsmi_tcpip_process_data_sent(uint8_t sent) {
//    if (sent) {                                 /* Data were successfully sent */
//        gsm.msg->msg.conn_send.sent_all += gsm.msg->msg.conn_send.sent;
//        gsm.msg->msg.conn_send.btw -= gsm.msg->msg.conn_send.sent;
//        gsm.msg->msg.conn_send.ptr += gsm.msg->msg.conn_send.sent;
//        if (gsm.msg->msg.conn_send.bw) {
//            *gsm.msg->msg.conn_send.bw += gsm.msg->msg.conn_send.sent;
//        }
//        gsm.msg->msg.conn_send.tries = 0;
//    } else {                                    /* We were not successful */
//        gsm.msg->msg.conn_send.tries++;         /* Increase number of tries */
//        if (gsm.msg->msg.conn_send.tries == GSM_CFG_MAX_SEND_RETRIES) { /* In case we reached max number of retransmissions */
//            return 1;                           /* Return 1 and indicate error */
//        }
//    }
//    if (gsm.msg->msg.conn_send.btw) {           /* Do we still have data to send? */
//        if (gsmi_tcpip_process_send_data() != gsmOK) {  /* Check if we can continue */
//            return 1;                           /* Finish at this point */
//        }
//        return 0;                               /* We still have data to send */
//    }
//    return 1;                                   /* Everything was sent, we can stop execution */
//}
//
///**
// * \brief           Send error event to application layer
// * \param[in]       msg: Message from user with connection start
// */
//static void
//gsmi_send_conn_error_cb(gsm_msg_t* msg) {
//    gsm_conn_t* conn = &gsm.conns[gsm.msg->msg.conn_start.num];
//    gsm.cb.type = GSM_CB_CONN_ERROR;            /* Connection error */
//    gsm.cb.cb.conn_error.host = gsm.msg->msg.conn_start.host;
//    gsm.cb.cb.conn_error.port = gsm.msg->msg.conn_start.port;
//    gsm.cb.cb.conn_error.type = gsm.msg->msg.conn_start.type;
//    gsm.cb.cb.conn_error.arg = gsm.msg->msg.conn_start.arg;
//    gsmi_send_conn_cb(conn, gsm.msg->msg.conn_start.cb_func);   /* Send event */
//}

/**
 * \brief           Process received string from GSM
 * \param[in]       recv: Pointer to \ref gsm_rect_t structure with input string
 */
static void
gsmi_parse_received(gsm_recv_t* rcv) {
    uint8_t is_ok = 0, is_ready = 0;
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

    /* Check for ready, if available */
    if (!is_ok && !is_error) {
        is_ready = !strcmp(rcv->data, "ready" CRLF);    /* Check if received string is ready */
    }

    /* Scan received strings which start with '+' */
    if (rcv->data[0] == '+') {
        if (!strncmp(rcv->data, "+CPIN", 5)) {  /* Check for +CPIN indication for SIM */
            gsmi_parse_cpin(rcv->data, 1);      /* Parse +CPIN response */
        } else if (!strncmp(rcv->data, "+CREG", 5)) {   /* Check for +CREG indication */
            gsmi_parse_creg(rcv->data, GSM_U8(IS_CURR_CMD(GSM_CMD_CREG_GET)));  /* Parse +CREG response */
        } else if (IS_CURR_CMD(GSM_CMD_COPS_GET) && !strncmp(rcv->data, "+COPS", 5)) {
            gsmi_parse_cops(rcv->data);         /* Parse current +COPS */
#if GSM_CFG_SMS
        } else if (IS_CURR_CMD(GSM_CMD_CMGS) && !strncmp(rcv->data, "+CMGS", 5)) {
            gsmi_parse_cmgs(rcv->data, 1);      /* Parse +CMGS response */
        } else if (IS_CURR_CMD(GSM_CMD_CMGR) && !strncmp(rcv->data, "+CMGR", 5)) {
            if (gsmi_parse_cmgr(rcv->data)) {   /* Parse +CMGR response */
                gsm.msg->msg.sms_read.read = 2; /* Set read flag and process the data */
            } else {
                gsm.msg->msg.sms_read.read = 1; /* Read but ignore data */
            }
        } else if (IS_CURR_CMD(GSM_CMD_CMGL) && !strncmp(rcv->data, "+CMGL", 5)) {
            if (gsmi_parse_cmgl(rcv->data)) {   /* Parse +CMGL response */
                gsm.msg->msg.sms_list.read = 2; /* Set read flag and process the data */
            } else {
                gsm.msg->msg.sms_list.read = 1; /* Read but ignore data */
            }
        } else if (!strncmp(rcv->data, "+CMTI", 5)) {
            gsmi_parse_cmti(rcv->data, 1);      /* Parse +CMTI response with received SMS */
        } else if (IS_CURR_CMD(GSM_CMD_CPMS_GET_OPT) && !strncmp(rcv->data, "+CPMS", 5)) {
            gsmi_parse_cpms(rcv->data, 0);      /* Parse +CPMS with SMS memories info */
        } else if (IS_CURR_CMD(GSM_CMD_CPMS_GET) && !strncmp(rcv->data, "+CPMS", 5)) {
            gsmi_parse_cpms(rcv->data, 1);      /* Parse +CPMS with SMS memories info */
        } else if (IS_CURR_CMD(GSM_CMD_CPMS_SET) && !strncmp(rcv->data, "+CPMS", 5)) {
            gsmi_parse_cpms(rcv->data, 2);      /* Parse +CPMS with SMS memories info */
#endif /* GSM_CFG_SMS */
#if GSM_CFG_CALL
        } else if (!strncmp(rcv->data, "+CLCC", 5)) {
            gsmi_parse_clcc(rcv->data, 1);      /* Parse +CLCC response with call info change */
#endif /* GSM_CFG_CALL */
#if GSM_CFG_PHONEBOOK
        } else if (IS_CURR_CMD(GSM_CMD_CPBS_GET_OPT) && !strncmp(rcv->data, "+CPBS", 5)) {
            gsmi_parse_cpbs(rcv->data, 0);      /* Parse +CPBS response */
        } else if (IS_CURR_CMD(GSM_CMD_CPBS_GET) && !strncmp(rcv->data, "+CPBS", 5)) {
            gsmi_parse_cpbs(rcv->data, 1);      /* Parse +CPBS response */
        } else if (IS_CURR_CMD(GSM_CMD_CPBS_SET) && !strncmp(rcv->data, "+CPBS", 5)) {
            gsmi_parse_cpbs(rcv->data, 2);      /* Parse +CPBS response */
        } else if (IS_CURR_CMD(GSM_CMD_CPBR) && !strncmp(rcv->data, "+CPBR", 5)) {
            gsmi_parse_cpbr(rcv->data);         /* Parse +CPBR statement */
        } else if (IS_CURR_CMD(GSM_CMD_CPBF) && !strncmp(rcv->data, "+CPBF", 5)) {
            gsmi_parse_cpbf(rcv->data);         /* Parse +CPBR statement */
#endif /* GSM_CFG_PHONEBOOK */
        }
    }

    /*
     * Check messages which do not start with '+' sign
     */
    if (rcv->data[0] != '+') {
#if GSM_CFG_SMS
        if (rcv->data[0] == 'S' && !strncmp(rcv->data, "SMS Ready" CRLF, 9 + CRLF_LEN)) {
            gsm.status.f.sms_ready = 1;         /* SMS ready flag */
            gsmi_send_cb(GSM_CB_SMS_READY);     /* Send SMS ready event */
        }
#endif /* GSM_CFG_SMS */
#if GSM_CFG_CALL
        if (rcv->data[0] == 'C' && !strncmp(rcv->data, "Call Ready" CRLF, 10 + CRLF_LEN)) {
            gsm.status.f.call_ready = 1;        /* Call ready flag */
            gsmi_send_cb(GSM_CB_CALL_READY);    /* Send call ready event */
        } else if (rcv->data[0] == 'R' && !strncmp(rcv->data, "RING" CRLF, 4 + CRLF_LEN)) {
            gsmi_send_cb(GSM_CB_CALL_RING);     /* Send call ring */
        } else if (rcv->data[0] == 'R' && !strncmp(rcv->data, "NO CARRIER" CRLF, 10 + CRLF_LEN)) {
            gsmi_send_cb(GSM_CB_CALL_NO_CARRIER);   /* Send call no carrier event */
        } else if (rcv->data[0] == 'R' && !strncmp(rcv->data, "BUSY" CRLF, 4 + CRLF_LEN)) {
            gsmi_send_cb(GSM_CB_CALL_BUSY);     /* Send call busy message */
        }
#endif /* GSM_CFG_CALL */
    }

    /*
     * Check general responses for active commands
     */
    if (gsm.msg != NULL) {
#if GSM_CFG_SMS
        if (IS_CURR_CMD(GSM_CMD_CMGS) && is_ok) {
            /* At this point we have to wait for "> " to send data */
        } else if (IS_CURR_CMD(GSM_CMD_CMGS) && is_error) {
            gsmi_send_cb(GSM_CB_SMS_SEND_ERROR);    /* SIM card event */
        }
#endif /* GSM_CFG_SMS */
    }
    
    /*
     * In case of any of these events, simply release semaphore
     * and proceed with next command
     */
    if (is_ok || is_error || is_ready) {
        gsmr_t res = gsmOK;
        if (gsm.msg != NULL) {                  /* Do we have active message? */
            res = gsmi_process_sub_cmd(gsm.msg, is_ok, is_error, is_ready);
            if (res != gsmCONT) {               /* Shall we continue with next subcommand under this one? */
                if (is_ok || is_ready) {        /* Check ready or ok status */
                    res = gsm.msg->res = gsmOK;
                } else {                        /* Or error status */
                    res = gsm.msg->res = res;   /* Set the error status */
                }
            } else {
                gsm.msg->i++;                   /* Number of continue calls */
            }
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
        len = gsm_buff_get_linear_block_length(&gsm.buff);
        if (len) {
            /*
             * Get memory address of first element
             * in linear block to process
             */
            data = gsm_buff_get_linear_block_address(&gsm.buff);
            
            /*
             * Process actual received data
             */
            gsmi_process(data, len);
            
            /*
             * Once they are processed, simply skip
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
    size_t d_len = data_len;
    const uint8_t* d;
    static uint8_t ch_prev1, ch_prev2;
    static gsm_unicode_t unicode;
    
    d = data;                                   /* Go to byte format */
    d_len = data_len;
    while (d_len) {                             /* Read entire set of characters from buffer */
        ch = *d++;                              /* Get next character */
        d_len--;                                /* Decrease remaining length */
        
        /*
         * Check if operators scan command is active
         * and if we are ready to read the incoming data
         */
        if (IS_CURR_CMD(GSM_CMD_COPS_GET_OPT) && gsm.msg->msg.cops_scan.read) {
            if (ch == '\n') {
                gsm.msg->msg.cops_scan.read = 0;
            } else {
                gsmi_parse_cops_scan(ch, 0);    /* Parse character by character */
            }
#if GSM_CFG_SMS
        } else if (IS_CURR_CMD(GSM_CMD_CMGR) && gsm.msg->msg.sms_read.read) {
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
                    gsm.cb.cb.sms_read.entry = e;
                    gsmi_send_cb(GSM_CB_SMS_READ);
                }
                gsm.msg->msg.sms_read.read = 0;
            }
        } else if (IS_CURR_CMD(GSM_CMD_CMGL) && gsm.msg->msg.sms_list.read) {
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

                    /*
                     * Do we have a special sequence "> "?
                     *
                     * Check if any command active which may expect that kind of rgsmonse
                     */
                    if (ch_prev2 == '\n' && ch_prev1 == '>' && ch == ' ') {
#if GSM_CFG_SMS
                        if (IS_CURR_CMD(GSM_CMD_CMGS)) {    /* Send SMS? */
                            GSM_AT_PORT_SEND(gsm.msg->msg.sms_send.text, strlen(gsm.msg->msg.sms_send.text));
                            GSM_AT_PORT_SEND_CTRL_Z();
                        }
#endif /* GSM_CFG_SMS */
                    } else if (IS_CURR_CMD(GSM_CMD_COPS_GET_OPT)) {
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
                    uint8_t i;
                    for (i = 0; i < unicode.t; i++) {
                        RECV_ADD(unicode.ch[i]);    /* Add character to receive array */
                    }
                }
            } else if (res != gsmINPROG) {      /* Not in progress? */
                RECV_RESET();                   /* Invalid character in sequence */
            }
        }
        
        ch_prev2 = ch_prev1;                    /* Save previous character to previous previous */
        ch_prev1 = ch;                          /* Char current to previous */
    }
    return gsmOK;
}

/**
 * \brief           Process current command with known execution status and start another if necessary
 * \param[in]       msg: Pointer to current message
 * \param[in]       is_ok: Status whether last command result was OK
 * \param[in]       is_error: Status whether last command result was ERROR
 * \param[in]       is_ready: Status whether last command result was ready
 * \return          gsmCONT if you sent more data and we need to process more data, or gsmOK on success, or gsmERR on error
 */
static gsmr_t
gsmi_process_sub_cmd(gsm_msg_t* msg, uint8_t is_ok, uint16_t is_error, uint8_t is_ready) {
    gsm_cmd_t n_cmd = GSM_CMD_IDLE;
    if (msg->cmd_def == GSM_CMD_RESET) {
        switch (msg->cmd) {                     /* Check current command */
            case GSM_CMD_RESET: {
                n_cmd = GSM_CFG_AT_ECHO ? GSM_CMD_ATE1 : GSM_CMD_ATE0;  /* Set ECHO mode */
                gsm_delay(3000);                /* Delay for some time before we can continue after reset */
                break;
            }
            case GSM_CMD_ATE0:
            case GSM_CMD_ATE1: {
                n_cmd = GSM_CMD_CFUN_SET;       /* Set full functionality */
                break;
            }
            case GSM_CMD_CFUN_SET: {
                n_cmd = GSM_CMD_CMEE;           /* Set detailed error reporting */
                break;
            }
            case GSM_CMD_CMEE: {
                n_cmd = GSM_CMD_CREG_SET;       /* Enable unsolicited code for CREG */
                break;
            }
            case GSM_CMD_CREG_SET: {
#if GSM_CFG_CALL
                n_cmd = GSM_CMD_CLCC;           /* Enable unsolicited code for call */
                break;
            }
            case GSM_CMD_CLCC: {
#endif /* GSM_CFG_CALL */
            }
            default: break;
        }
    } else if (msg->cmd_def == GSM_CMD_COPS_GET) {
        if (msg->cmd == GSM_CMD_COPS_GET) {
            gsm.cb.cb.operator_current.operator_current = &gsm.network.curr_operator;
            gsmi_send_cb(GSM_CB_OPERATOR_CURRENT);
        }
    } else if (msg->cmd_def == GSM_CMD_SIM_PROCESS_BASIC_CMDS) {
        switch (msg->cmd) {
            case GSM_CMD_CNUM: {                /* Get own phone number */
                if (!is_ok) {
                    gsm_delay(1000);            /* Process delay first */
                    n_cmd = GSM_CMD_CNUM;
                } else 
#if GSM_CFG_PHONEBOOK
                n_cmd = GSM_CMD_CPBS_GET_OPT;   /* Get phonebook storages */
                break;
            }
            case GSM_CMD_CPBS_GET_OPT: {
#endif /* GSM_CFG_PHONEBOOK */
#if GSM_CFG_SMS
                n_cmd = GSM_CMD_CPMS_GET_OPT;   /* Get SMS storages */
                break;
            }
            case GSM_CMD_CPMS_GET_OPT: {
                if (!is_ok) {                   /* In case of failure, try again */
                    gsm_delay(1000);
                    n_cmd = GSM_CMD_CPMS_GET_OPT;   /* Get memory options */
                } else {
                    n_cmd = GSM_CMD_CPMS_GET;   /* Get current status */
                }
                break;
            }
            case GSM_CMD_CPMS_GET: {
#endif /* GSM_CFG_SMS */
            }
            default: break;
        }
    } else if (msg->cmd_def == GSM_CMD_CPIN_SET) {  /* Set PIN code */
        if (msg->cmd == GSM_CMD_CPIN_GET && is_ok) {
            /**
             * \todo: Check if pin already OK, then decide what to do next
             */
            n_cmd = GSM_CMD_CPIN_SET;           /* Set command to write PIN */
        }
#if GSM_CFG_NETWORK
    } else if (msg->cmd_def == GSM_CMD_NETWORK_ATTACH) {
        switch (msg->cmd) {
            case GSM_CMD_CGACT_SET_0: n_cmd = GSM_CMD_CGACT_SET_1; break;
            case GSM_CMD_CGACT_SET_1: {
                if (is_ok) {
                    n_cmd = GSM_CMD_CGATT_SET_0;
                }
                break;
            }
            case GSM_CMD_CGATT_SET_0: n_cmd = GSM_CMD_CGATT_SET_1; break;
            case GSM_CMD_CGATT_SET_1: {
                if (is_ok) {
                    n_cmd = GSM_CMD_CGATT_SET_0;
                }
                break;
            }
        }
#endif /* GSM_CFG_NETWORK */
#if GSM_CFG_SMS
    } else if (msg->cmd_def == GSM_CMD_CMGS) {  /* Send SMS default command */
        if (msg->cmd == GSM_CMD_CMGF && is_ok) {/* Set message format current command*/
            n_cmd = GSM_CMD_CMGS;               /* Now send actual message */
        }
    } else if (msg->cmd_def == GSM_CMD_CMGR) {  /* Read SMS message */
        if (msg->cmd == GSM_CMD_CPMS_GET && is_ok) {
            n_cmd = GSM_CMD_CPMS_SET;           /* Set memory */
        } else if (msg->cmd == GSM_CMD_CPMS_SET && is_ok) {
            n_cmd = GSM_CMD_CMGF;               /* Set text mode */
        } else if (msg->cmd == GSM_CMD_CMGF && is_ok) {/* Set message format current command*/
            n_cmd = GSM_CMD_CMGR;               /* Start message read */
        } else if (n_cmd == GSM_CMD_CMGR && is_ok) {
            msg->msg.sms_read.mem = gsm.sms.mem[0].current; /* Set current memory */
        }
    } else if (msg->cmd_def == GSM_CMD_CMGD) {  /* Delete SMS message*/
        if (msg->cmd == GSM_CMD_CPMS_GET && is_ok) {
            n_cmd = GSM_CMD_CPMS_SET;           /* Set memory */
        } else if (msg->cmd == GSM_CMD_CPMS_SET && is_ok) {
            n_cmd = GSM_CMD_CMGD;               /* Delete message */
        }
    } else if (msg->cmd_def == GSM_CMD_CMGL) {  /* List SMS messages */
        if (msg->cmd == GSM_CMD_CPMS_GET && is_ok) {
            n_cmd = GSM_CMD_CPMS_SET;           /* Set memory */
        } else if (msg->cmd == GSM_CMD_CPMS_SET && is_ok) {
            n_cmd = GSM_CMD_CMGF;               /* Set text format */
        } else if (msg->cmd == GSM_CMD_CMGF && is_ok) {
            n_cmd = GSM_CMD_CMGL;               /* List messages */
        } else if (GSM_CMD_CMGL) {
            gsm.cb.cb.sms_list.mem = gsm.sms.mem[0].current;
            gsm.cb.cb.sms_list.entries = gsm.msg->msg.sms_list.entries;
            gsm.cb.cb.sms_list.size = gsm.msg->msg.sms_list.ei;
            gsm.cb.cb.sms_list.err = is_ok ? gsmOK : gsmERR;
            gsmi_send_cb(GSM_CB_SMS_LIST);
        }
    } else if (msg->cmd_def == GSM_CMD_CPMS_SET) {  /* Set preferred memory */
        if (msg->cmd == GSM_CMD_CPMS_GET && is_ok) {
            n_cmd = GSM_CMD_CPMS_SET;           /* Now set the command */
        }
#endif /* GSM_CFG_SMS */
#if GSM_CFG_PHONEBOOK
    } else if (msg->cmd_def == GSM_CMD_CPBW_SET) {  /* Write phonebook entry */
        if (msg->cmd == GSM_CMD_CPBS_GET && is_ok) {/* Get current memory */
            n_cmd = GSM_CMD_CPBS_SET;           /* Set current memory */
        } else if (msg->cmd == GSM_CMD_CPBS_SET && is_ok) {
            n_cmd = GSM_CMD_CPBW_SET;           /* Write entry to phonebook */
        }
    } else if (msg->cmd_def == GSM_CMD_CPBR) {
        if (msg->cmd == GSM_CMD_CPBS_GET && is_ok) {/* Get current memory */
            n_cmd = GSM_CMD_CPBS_SET;           /* Set current memory */
        } else if (msg->cmd == GSM_CMD_CPBS_SET && is_ok) {
            n_cmd = GSM_CMD_CPBR;               /* Read entries */
        } else if (msg->cmd == GSM_CMD_CPBR) {
            gsm.cb.cb.pb_list.mem = gsm.pb.mem.current;
            gsm.cb.cb.pb_list.entries = gsm.msg->msg.pb_list.entries;
            gsm.cb.cb.pb_list.size = gsm.msg->msg.pb_list.ei;
            gsm.cb.cb.pb_list.err = is_ok ? gsmOK : gsmERR;
            gsmi_send_cb(GSM_CB_PB_LIST);
        }
    } else if (msg->cmd_def == GSM_CMD_CPBF) {
        if (msg->cmd == GSM_CMD_CPBS_GET && is_ok) {/* Get current memory */
            n_cmd = GSM_CMD_CPBS_SET;           /* Set current memory */
        } else if (msg->cmd == GSM_CMD_CPBS_SET && is_ok) {
            n_cmd = GSM_CMD_CPBF;               /* Read entries */
        } else if (msg->cmd == GSM_CMD_CPBF) {
            gsm.cb.cb.pb_search.mem = gsm.pb.mem.current;
            gsm.cb.cb.pb_search.search = gsm.msg->msg.pb_search.search;
            gsm.cb.cb.pb_search.entries = gsm.msg->msg.pb_search.entries;
            gsm.cb.cb.pb_search.size = gsm.msg->msg.pb_search.ei;
            gsm.cb.cb.pb_search.err = is_ok ? gsmOK : gsmERR;
            gsmi_send_cb(GSM_CB_PB_SEARCH);
        }
#endif /* GSM_CFG_PHONEBOOK */
    }

    /*
     * Check if new command was set for execution
     */
    if (n_cmd != GSM_CMD_IDLE) {
        msg->cmd = n_cmd;
        if (msg->fn(msg) == gsmOK) {
            return gsmCONT;
        }
    }
    return is_ok || is_ready ? gsmOK : gsmERR;
}

/**
 * \brief           Function to initialize every AT command
 * \note            Never call this function directly. Set as initialization function for command and use `msg->fn(msg)`
 * \param[in]       msg: Pointer to \ref gsm_msg_t with data
 * \return          Member of \ref gsmr_t enumeration
 */
gsmr_t
gsmi_initiate_cmd(gsm_msg_t* msg) {
    switch (msg->cmd) {                         /* Check current message we want to send over AT */
        case GSM_CMD_RESET: {                   /* Reset modem with AT commands */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CFUN=1,1");  /* Second "1" means reset */
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_ATE0:
        case GSM_CMD_ATE1: {
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            if (msg->cmd == GSM_CMD_ATE0) {
                GSM_AT_PORT_SEND_STR("E0");
            } else {
                GSM_AT_PORT_SEND_STR("E1");
            }
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CMEE: {                    /* Enable detailed error messages */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CMEE=1");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CREG_SET: {                /* Enable +CREG message */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CREG=1");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CREG_GET: {                /* Get network registration status */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CREG?");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CFUN_SET: {
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CFUN=");
            /**
             * \todo: If CFUN command forced, check value
             */
            if (msg->cmd_def == GSM_CMD_RESET || 
                (msg->cmd_def == GSM_CMD_CFUN_SET && msg->msg.cfun.mode)) {
                GSM_AT_PORT_SEND_STR("1");
            } else {
                GSM_AT_PORT_SEND_STR("0");
            }
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CPIN_GET: {                /* Read current SIM status */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CPIN?");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CPIN_SET: {                /* Set SIM pin code */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CPIN=");
            send_string(msg->msg.cpin.pin, 0, 1, 0);    /* Send pin with quotes */
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_COPS_SET: {                /* Set current operator */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+COPS=");
            send_number(GSM_U32(msg->msg.cops_set.mode), 0, 0);
            if (msg->msg.cops_set.mode != GSM_OPERATOR_MODE_AUTO) {
                send_number(GSM_U32(msg->msg.cops_set.format), 0, 1);
                switch (msg->msg.cops_set.format) {
                    case GSM_OPERATOR_FORMAT_LONG_NAME:
                    case GSM_OPERATOR_FORMAT_SHORT_NAME:
                        send_string(msg->msg.cops_set.name, 1, 1, 1);
                        break;
                    default: 
                        send_number(GSM_U32(msg->msg.cops_set.num), 0, 1);
                }
            }
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_COPS_GET: {                /* Get current operator */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+COPS?");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_COPS_GET_OPT: {            /* Get list of available operators */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+COPS=?");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CNUM: {                    /* Get SIM number */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CNUM");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
#if GSM_CFG_CONN
        case GSM_CMD_CIPSHUT: {                 /* Shut down network connection and put to reset state */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CIPSHUT");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CIPMUX: {                  /* Enable multiple connections */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CIPMUX=1");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CIPHEAD: {                 /* Enable information on receive data about connection and length */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CIPHEAD=1");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CIPSRIP: {
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CIPSRIP=1");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
#endif /* GSM_CFG_CONN */
#if GSM_CFG_SMS
        case GSM_CMD_CMGF: {                    /* Select SMS message format */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CMGF=");
            if (msg->cmd_def == GSM_CMD_CMGS) {
                send_number(GSM_U32(!!msg->msg.sms_send.format), 0, 0);
            } else if (msg->cmd_def == GSM_CMD_CMGR) {
                send_number(GSM_U32(!!msg->msg.sms_read.format), 0, 0);
            } else if (msg->cmd_def == GSM_CMD_CMGL) {
                send_number(GSM_U32(!!msg->msg.sms_list.format), 0, 0);
            } else {
                GSM_AT_PORT_SEND_STR("1");      /* Force text mode */
            }
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CMGS: {                    /* Send SMS */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CMGS=");
            send_string(msg->msg.sms_send.num, 0, 1, 0);
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CMGR: {                    /* Read message */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CMGR=");
            send_number(GSM_U32(msg->msg.sms_read.pos), 0, 0);
            send_number(GSM_U32(!msg->msg.sms_read.update), 0, 1);
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CMGD: {                    /* Delete SMS message */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CMGD=");
            send_number(GSM_U32(msg->msg.sms_delete.pos), 0, 0);
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CMGL: {                    /* Delete SMS message */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CMGL=");
            send_sms_stat(msg->msg.sms_list.status, 1, 0);
            send_number(GSM_U32(!msg->msg.sms_list.update), 0, 1);
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CPMS_GET_OPT: {            /* Get available SMS storages */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CPMS=?");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CPMS_GET: {                /* Get current SMS storage info */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CPMS?");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CPMS_SET: {                /* Set active SMS storage(s) */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CPMS=");
            if (msg->cmd_def == GSM_CMD_CMGR) { /* Read SMS original command? */
                send_dev_memory(msg->msg.sms_read.mem == GSM_MEM_CURRENT ? gsm.sms.mem[0].current : msg->msg.sms_read.mem, 1, 0);
            } else if(msg->cmd_def == GSM_CMD_CMGD) {   /* Delete SMS original command? */
                send_dev_memory(msg->msg.sms_delete.mem == GSM_MEM_CURRENT ? gsm.sms.mem[0].current : msg->msg.sms_delete.mem, 1, 0);
            } else if(msg->cmd_def == GSM_CMD_CMGL) {   /* List SMS original command? */
                send_dev_memory(msg->msg.sms_list.mem == GSM_MEM_CURRENT ? gsm.sms.mem[0].current : msg->msg.sms_list.mem, 1, 0);
            } else if (msg->cmd_def == GSM_CMD_CPMS_SET) {  /* Do we want to set memory for read/delete,sent/write,receive? */
                size_t i;
                for (i = 0; i < 3; i++) {       /* Write 3 memories */
                    send_dev_memory(msg->msg.sms_memory.mem[i] == GSM_MEM_CURRENT ? gsm.sms.mem[i].current : msg->msg.sms_memory.mem[i], 1, !!i);
                }
            }
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
#endif /* GSM_CFG_SMS */
#if GSM_CFG_CALL
        case GSM_CMD_ATD: {                     /* Start new call */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("D");
            send_string(msg->msg.call_start.number, 0, 0, 0);
            GSM_AT_PORT_SEND_STR(";");          /* Voice call includes semicolon at the end */
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_ATA: {                     /* Answer phone call */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("A");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_ATH: {                     /* Disconnect existing connection (hang-up phone call) */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("H");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CLCC: {                    /* Enable auto notification on received call */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CLCC=1");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
#endif /* GSM_CFG_CALL */
#if GSM_CFG_PHONEBOOK
        case GSM_CMD_CPBS_GET_OPT: {            /* Get available phonebook storages */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CPBS=?");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CPBS_GET: {                /* Get current memory info */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CPBS?");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CPBS_SET: {                /* Get current memory info */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CPBS=");
            send_dev_memory(msg->msg.pb_write.mem == GSM_MEM_CURRENT ? gsm.pb.mem.current : msg->msg.pb_write.mem, 1, 0);
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CPBW_SET: {                /* Write/Delete new/old entry */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CPBW=");
            if (msg->msg.pb_write.pos) {        /* Write number if more than 0 */
                send_number(GSM_U32(msg->msg.pb_write.pos), 0, 0);
            }
            if (!msg->msg.pb_write.del) {
                send_string(msg->msg.pb_write.num, 0, 1, 1);
                send_number(GSM_U32(msg->msg.pb_write.type), 0, 1);
                send_string(msg->msg.pb_write.name, 0, 1, 1);
            }
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CPBR: {                    /* Read entires */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CPBR=");
            send_number(GSM_U32(msg->msg.pb_list.start_index), 0, 0);
            send_number(GSM_U32(msg->msg.pb_list.etr), 0, 1);
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CPBF: {                    /* Find entires */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CPBF=");
            send_string(msg->msg.pb_search.search, 1, 1, 0);
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
#endif /* GSM_CFG_PHONEBOOK */
        default: 
            return gsmERR;                      /* Invalid command */
    }
    return gsmOK;                               /* Valid command */
}

/**
 * \brief           Checks if connection pointer has valid address
 * \param[in]       conn: Address to check if valid connection ptr
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_is_valid_conn_ptr(gsm_conn_p conn) {
    uint8_t i = 0;
    for (i = 0; i < sizeof(gsm.conns) / sizeof(gsm.conns[0]); i++) {
        if (conn == &gsm.conns[i]) {
            return 1;
        }
    }
    return 0;
}

/**
 * \brief           Send message from API function to producer queue for further processing
 * \param[in]       msg: New message to process
 * \param[in]       process_fn: callback function used to process message
 * \param[in]       block_time: Time used to block function. Use 0 for non-blocking call
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsmi_send_msg_to_producer_mbox(gsm_msg_t* msg, gsmr_t (*process_fn)(gsm_msg_t *), uint32_t block, uint32_t max_block_time) {
    gsmr_t res = msg->res = gsmOK;
    if (block) {                                /* In case message is blocking */
        if (!gsm_sys_sem_create(&msg->sem, 0)) {/* Create semaphore and lock it immediatelly */
            GSM_MSG_VAR_FREE(msg);              /* Release memory and return */
            return gsmERRMEM;
        }
    }
    if (!msg->cmd) {                            /* Set start command if not set by user */
        msg->cmd = msg->cmd_def;                /* Set it as default */
    }
    msg->is_blocking = block;                   /* Set status if message is blocking */
    msg->block_time = max_block_time;           /* Set blocking status if necessary */
    msg->fn = process_fn;                       /* Save processing function to be called as callback */
    if (block) {
        gsm_sys_mbox_put(&gsm.mbox_producer, msg);  /* Write message to producer queue and wait until written */
    } else {
        if (!gsm_sys_mbox_putnow(&gsm.mbox_producer, msg)) {    /* Write message to producer queue immediatelly */
            GSM_MSG_VAR_FREE(msg);              /* Release message */
            res = gsmERR;
        }
    }
    if (block && res == gsmOK) {                /* In case we have blocking request */
        uint32_t time;
        time = gsm_sys_sem_wait(&msg->sem, max_block_time); /* Wait forever for semaphore access for max block time */
        if (GSM_SYS_TIMEOUT == time) {          /* If semaphore was not accessed in given time */
            res = gsmERR;                       /* Semaphore not released in time */
        } else {
            res = msg->res;                     /* Set rgsmonse status from message rgsmonse */
        }
        if (gsm_sys_sem_isvalid(&msg->sem)) {   /* In case we have valid semaphore */
            gsm_sys_sem_delete(&msg->sem);      /* Delete semaphore object */
            gsm_sys_sem_invalid(&msg->sem);     /* Invalidate semaphore object */
        }
        GSM_MSG_VAR_FREE(msg);                  /* Release message */
    }
    return res;
}
