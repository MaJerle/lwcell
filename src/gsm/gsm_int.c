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

#define IS_CURR_CMD(c)        (gsm.msg != NULL && gsm.msg->cmd == (c))

#if !__DOXYGEN__
typedef struct {
    char data[128];
    uint8_t len;
} gsm_recv_t;
static gsm_recv_t recv_buff;
#endif /* !__DOXYGEN__ */

#define CRLF                "\r\n"

#define RECV_ADD(ch)        do { recv_buff.data[recv_buff.len++] = ch; recv_buff.data[recv_buff.len] = 0; } while (0)
#define RECV_RESET()        do { recv_buff.len = 0; recv_buff.data[0] = 0; } while (0)
#define RECV_LEN()          recv_buff.len
#define RECV_IDX(index)     recv_buff.data[index]

#define GSM_AT_PORT_SEND_STR(str)       gsm.ll.send_fn((const uint8_t *)(str), (uint16_t)strlen(str))
#define GSM_AT_PORT_SEND_CHR(str)       gsm.ll.send_fn((const uint8_t *)(str), (uint16_t)1)
#define GSM_AT_PORT_SEND(d, l)          gsm.ll.send_fn((const uint8_t *)(d), (uint16_t)l)

static gsmr_t gsmi_process_sub_cmd(gsm_msg_t* msg, uint8_t is_ok, uint8_t is_error, uint8_t is_ready);

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
 * \param[in]       is_ip: Set to 1 when sending IP, or 0 when MAC
 * \param[in]       q: Set to 1 to include start and ending quotes
 */
static void
send_ip_mac(const void* d, uint8_t is_ip, uint8_t q) {
    uint8_t i, ch;
    char str[4];
    const gsm_mac_t* mac = d;
    const gsm_ip_t* ip = d;
    
    if (d == NULL) {
        return;
    }
    if (q) {
        GSM_AT_PORT_SEND_STR("\"");             /* Send starting quote character */
    }
    ch = is_ip ? '.' : ':';                     /* Get delimiter character */
    for (i = 0; i < (is_ip ? 4 : 6); i++) {     /* Process byte by byte */
        if (is_ip) {                            /* In case of IP ... */
            number_to_str(is_ip ? ip->ip[i] : mac->mac[i], str);    /* ... go to decimal format ... */
        } else {                                /* ... in case of MAC ... */
            byte_to_str(is_ip ? ip->ip[i] : mac->mac[i], str);  /* ... go to HEX format */
        }
        GSM_AT_PORT_SEND_STR(str);              /* Send str */
        if (i < (is_ip ? 4 : 6) - 1) {          /* Check end if characters */
            GSM_AT_PORT_SEND_CHR(&ch);          /* Send character */
        }
    }
    if (q) {
        GSM_AT_PORT_SEND_STR("\"");             /* Send ending quote character */
    }
}

/**
 * \brief           Send string to AT port, either plain or escaped
 * \param[in]       str: Pointer to input string to string
 * \param[in]       e: Value to indicate string send format, escaped (1) or plain (0)
 * \param[in]       q: Value to indicate starting and ending quotes, enabled (1) or disabled (0)
 */
static void
send_string(const char* str, uint8_t e, uint8_t q) {
    char special = '\\';
    if (q) {
        GSM_AT_PORT_SEND_STR("\"");
    }
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
    if (q) {
        GSM_AT_PORT_SEND_STR("\"");
    }
}

/**
 * \brief           Send number (decimal) to AT port
 * \param[in]       num: Number to send to AT port
 * \param[in]       q: Value to indicate starting and ending quotes, enabled (1) or disabled (0)
 */
static void
send_number(uint32_t num, uint8_t q) {
    char str[11];
    
    number_to_str(num, str);                    /* Convert digit to decimal string */
    if (q) {
        GSM_AT_PORT_SEND_STR("\"");
    }
    GSM_AT_PORT_SEND_STR(str);                  /* Send string with number */
    if (q) {
        GSM_AT_PORT_SEND_STR("\"");
    }
}

/**
 * \brief           Send signed number to AT port
 * \param[in]       num: Number to send to AT port
 * \param[in]       q: Value to indicate starting and ending quotes, enabled (1) or disabled (0)
 */
void
send_signed_number(int32_t num, uint8_t q) {
    char str[11];
    
    signed_number_to_str(num, str);             /* Convert digit to decimal string */
    if (q) {
        GSM_AT_PORT_SEND_STR("\"");
    }
    GSM_AT_PORT_SEND_STR(str);                  /* Send string with number */
    if (q) {
        GSM_AT_PORT_SEND_STR("\"");
    }
}

/**
 * \brief           Reset all connections
 * \note            Used to notify upper layer stack to close everything and reset the memory if necessary
 * \param[in]       forced: Flag indicating reset was forced by user
 */
static void
reset_connections(uint8_t forced) {
    size_t i;
    
    gsm.cb.type = GSM_CB_CONN_CLOSED;
    gsm.cb.cb.conn_active_closed.forced = forced;
    
    for (i = 0; i < GSM_CFG_MAX_CONNS; i++) {   /* Check all connections */
        if (gsm.conns[i].status.f.active) {
            gsm.conns[i].status.f.active = 0;
            
            gsm.cb.cb.conn_active_closed.conn = &gsm.conns[i];
            gsm.cb.cb.conn_active_closed.client = gsm.conns[i].status.f.client;
            gsmi_send_conn_cb(&gsm.conns[i], NULL); /* Send callback function */
        }
    }
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
    uint8_t is_ok = 0, is_error = 0, is_ready = 0;

    /* Try to remove non-parsable strings */
    if ((rcv->len == 2 && rcv->data[0] == '\r' && rcv->data[1] == '\n') {
        return;
    }
    
    /* Detect most common responses from device */
    is_ok = !strcmp(rcv->data, "OK" CRLF);      /* Check if received string is OK */
    if (!is_ok) {
        is_error = !strcmp(rcv->data, "ERROR" CRLF) || !strcmp(rcv->data, "FAIL" CRLF); /* Check if received string is error */
        if (!is_error) {
            is_ready = !strcmp(rcv->data, "ready" CRLF);    /* Check if received string is ready */
        }
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
        
 
        if (0) {
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
gsmi_process_sub_cmd(gsm_msg_t* msg, uint8_t is_ok, uint8_t is_error, uint8_t is_ready) {
    if (msg->cmd_def == GSM_CMD_RESET) {
        gsm_cmd_t n_cmd = GSM_CMD_IDLE;
        switch (msg->cmd) {                     /* Check current command */
            case GSM_CMD_RESET: {
                n_cmd = GSM_CFG_AT_ECHO ? GSM_CMD_ATE1 : GSM_CMD_ATE0;  /* Set ECHO mode */
                break;
            }
            case GSM_CMD_ATE0:
            case GSM_CMD_ATE1: {
#if GSM_CFG_CONN
                n_cmd = GSM_CMD_CIPSHUT;        /* Shutdown any connection */
                break;
            }
            case GSM_CMD_CIPSHUT: {
                n_cmd = GSM_CMD_CIPMUX;         /* Enable multiple connections */
                break;
            }
            case GSM_CMD_CIPMUX: {
                n_cmd = GSM_CMD_CIPHEAD;        /* Enable more info in received data */
                break;
            }
            case GSM_CMD_CIPHEAD: {
#endif /* GSM_CFG_CONN */
            }
            default: break;
        }
    } else
    {

    }
    return is_ok || is_ready ? gsmOK : gsmERR;
}

/**
 * \brief           Function to initialize every AT command
 * \param[in]       msg: Pointer to \ref gsm_msg_t with data
 * \return          Member of \ref gsmr_t enumeration
 */
gsmr_t
gsmi_initiate_cmd(gsm_msg_t* msg) {
    switch (msg->cmd) {                         /* Check current message we want to send over AT */
        case GSM_CMD_RESET: {                   /* Reset MCU with AT commands */
            GSM_AT_PORT_SEND_STR("AT+RST" CRLF);
            break;
        }
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
        gsm_sys_mbox_put(&gsm.mbox_producer, msg);  /* Write message to producer queue and wait forever */
    } else {
        if (!gsm_sys_mbox_putnow(&gsm.mbox_producer, msg)) {    /* Write message to producer queue immediatelly */
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
