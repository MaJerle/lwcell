/**
 * \file            gsm_device_sim800.c
 * \brief           SIM800 device driver implementation
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
#include "gsm/gsm_device.h"
#include "gsm/gsm_parser.h"

static gsmr_t   at_send_cmd(gsm_msg_t* msg);
static uint8_t  at_line_recv(gsm_recv_t* recv, uint8_t* is_ok, uint16_t* is_error);
static gsmr_t   at_process_sub_cmd(gsm_msg_t* msg, uint8_t is_ok, uint16_t is_error);

/**
 * \brief           Custom CMD dedicated for current modem
 */
typedef enum {
    GSM_CMD_CSTM_START = GSM_CMD_END,

    /* Network based subcommands */
    GSM_CMD_CSTM_CGACT_SET_0,
    GSM_CMD_CSTM_CGACT_SET_1,
    GSM_CMD_CSTM_CGATT_SET_0,
    GSM_CMD_CSTM_CGATT_SET_1,
    GSM_CMD_CSTM_CIPSHUT,
    GSM_CMD_CSTM_CIPMUX_SET,
    GSM_CMD_CSTM_CIPRXGET_SET,
    GSM_CMD_CSTM_CSTT_SET,
    GSM_CMD_CSTM_CIICR,
    GSM_CMD_CSTM_CIFSR,
} gsm_cmd_custom_t;

/**
 * \brief           Device driver control structure
 */
gsm_device_driver_t
gsm_device = {
    .features = GSM_DEVICE_FEATURE_SMS | GSM_DEVICE_FEATURE_CALL |
                GSM_DEVICE_FEATURE_PB | GSM_DEVICE_FEATURE_TCPIP,
    .at_start_cmd_fn = at_send_cmd,
    .at_line_recv_fn = at_line_recv,
    .at_process_sub_cmd_fn = at_process_sub_cmd,
};

/**
 * \brief           Process sub command
 */
static gsmr_t
at_process_sub_cmd(gsm_msg_t* msg, uint8_t is_ok, uint16_t is_error) {
    uint32_t cmd = 0;
    
#if GSM_CFG_NETWORK
    if (CMD_IS_DEF(GSM_CMD_NETWORK_ATTACH)) {
        switch (msg->i) {
            case 0: cmd = GSM_CMD_CSTM_CGACT_SET_1; break;
            case 1: cmd = GSM_CMD_CSTM_CGATT_SET_0; break;
            case 2: cmd = GSM_CMD_CSTM_CGATT_SET_1; break;
            case 3: cmd = GSM_CMD_CSTM_CIPSHUT; break;
            case 4: cmd = GSM_CMD_CSTM_CIPMUX_SET; break;
            case 5: cmd = GSM_CMD_CSTM_CIPRXGET_SET; break;
            case 6: cmd = GSM_CMD_CSTM_CSTT_SET; break;
            case 7: cmd = GSM_CMD_CSTM_CIICR; break;
            case 8: cmd = GSM_CMD_CSTM_CIFSR; break;
            default: break;
        }
    } else if (CMD_IS_DEF(GSM_CMD_NETWORK_DETACH)) {
        switch (msg->i) {
            case 0: cmd = GSM_CMD_CSTM_CGACT_SET_0; break;
            default: break;
        }
    }
#endif /* GSM_CFG_NETWORK */
    if (cmd) {
        msg->cmd = (gsm_cmd_t)cmd;
        return msg->fn(msg) == gsmOK ? gsmCONT : gsmERR;
    }
    return is_ok ? gsmOK : gsmERR;
}

/**
 * \brief           Send AT command to device
 * \param[in]       msg: Current active message
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
static gsmr_t
at_send_cmd(gsm_msg_t* msg) {
    switch (CMD_GET_CUR()) {
        case GSM_CMD_NETWORK_ATTACH:
        case GSM_CMD_CSTM_CGACT_SET_0: {
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CGACT=0");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CSTM_CGACT_SET_1: {
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CGACT=1");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_NETWORK_DETACH:
        case GSM_CMD_CSTM_CGATT_SET_0: {
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CGATT=0");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CSTM_CGATT_SET_1: {
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CGATT=1");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CSTM_CIPSHUT: {
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CIPSHUT");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CSTM_CIPMUX_SET: {         /* Set multiple connections */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CIPMUX=1");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CSTM_CIPRXGET_SET: {
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CIPRXGET=1");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CSTM_CSTT_SET: {
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CSTT=");
            send_string(msg->msg.network_attach.apn, 1, 1, 0);
            send_string(msg->msg.network_attach.user, 1, 1, 1);
            send_string(msg->msg.network_attach.pass, 1, 1, 1);
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CSTM_CIICR: {
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CIICR");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        case GSM_CMD_CSTM_CIFSR: {              /* Acquire IP address */
            GSM_AT_PORT_SEND_BEGIN();           /* Begin AT command string */
            GSM_AT_PORT_SEND_STR("+CIFSR");
            GSM_AT_PORT_SEND_END();             /* End AT command string */
            break;
        }
        default:
            return gsmERR;
    }
    return gsmOK;
}

/**
 * \brief           Process received line over AT port
 * \param[in]       rcv: Received line
 * \param[in,out]   is_ok: Pointer to OK status
 * \param[in,out]   is_error: Pointer to ERROR status
 * \return          1 if process, 0 otherwise
 */
static uint8_t
at_line_recv(gsm_recv_t* rcv, uint8_t* is_ok, uint16_t* is_error) {
    if (rcv->data[0] == '+') {
    
    } else {
        if (rcv->data[0] == 'S' && !strncmp(rcv->data, "SHUT OK" CRLF, 7 + CRLF_LEN)) {
            *is_ok = 1;
#if GSM_CFG_SMS
        } else if (rcv->data[0] == 'S' && !strncmp(rcv->data, "SMS Ready" CRLF, 9 + CRLF_LEN)) {
            gsmi_device_set_sms_ready(1);       /* Set SMS ready */
#endif /* GSM_CFG_SMS */
#if GSM_CFG_CALL
        } else if (rcv->data[0] == 'C' && !strncmp(rcv->data, "Call Ready" CRLF, 10 + CRLF_LEN)) {
            gsmi_device_set_call_ready(1);      /* Set call ready */
#endif /* GSM_CFG_CALL */
        } else if (CMD_IS_CUR(GSM_CMD_CSTM_CIFSR) && GSM_CHARISNUM(rcv->data[0])) {
            gsm_ip_t ip;
            const char* tmp = rcv->data;
            gsmi_parse_ip(&tmp, &ip);
            gsmi_device_set_ip(&ip);            /* Set device IP */
            *is_ok = 1;                         /* Manually set OK flag as we don't expect OK in command */
        }
    }

    return 1;
}
