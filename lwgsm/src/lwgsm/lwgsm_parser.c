/**
 * \file            lwgsm_parser.c
 * \brief           Parse incoming data from AT port
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
#include "lwgsm/lwgsm_parser.h"
#include "lwgsm/lwgsm_mem.h"

/**
 * \brief           Parse number from string
 * \note            Input string pointer is changed and number is skipped
 * \param[in,out]   str: Pointer to pointer to string to parse
 * \return          Parsed number
 */
int32_t
lwgsmi_parse_number(const char** str) {
    int32_t val = 0;
    uint8_t minus = 0;
    const char* p = *str;                       /*  */

    if (*p == '"') {                            /* Skip leading quotes */
        ++p;
    }
    if (*p == ',') {                            /* Skip leading comma */
        ++p;
    }
    if (*p == '"') {                            /* Skip leading quotes */
        ++p;
    }
    if (*p == '/') {                            /* Skip '/' character, used in datetime */
        ++p;
    }
    if (*p == ':') {                            /* Skip ':' character, used in datetime */
        ++p;
    }
    if (*p == '+') {                            /* Skip '+' character, used in datetime */
        ++p;
    }
    if (*p == '-') {                            /* Check negative number */
        minus = 1;
        ++p;
    }
    while (LWGSM_CHARISNUM(*p)) {               /* Parse until character is valid number */
        val = val * 10 + LWGSM_CHARTONUM(*p);
        ++p;
    }
    if (*p == '"') {                            /* Skip trailling quotes */
        ++p;
    }
    *str = p;                                   /* Save new pointer with new offset */

    return minus ? -val : val;
}

/**
 * \brief           Parse number from string as hex
 * \note            Input string pointer is changed and number is skipped
 * \param[in,out]   str: Pointer to pointer to string to parse
 * \return          Parsed number
 */
uint32_t
lwgsmi_parse_hexnumber(const char** str) {
    int32_t val = 0;
    const char* p = *str;                       /*  */

    if (*p == '"') {                            /* Skip leading quotes */
        ++p;
    }
    if (*p == ',') {                            /* Skip leading comma */
        ++p;
    }
    if (*p == '"') {                            /* Skip leading quotes */
        ++p;
    }
    while (LWGSM_CHARISHEXNUM(*p)) {            /* Parse until character is valid number */
        val = val * 16 + LWGSM_CHARHEXTONUM(*p);
        ++p;
    }
    if (*p == ',') {                            /* Go to next entry if possible */
        ++p;
    }
    *str = p;                                   /* Save new pointer with new offset */
    return val;
}

/**
 * \brief           Parse input string as string part of AT command
 * \param[in,out]   src: Pointer to pointer to string to parse from
 * \param[in]       dst: Destination pointer.
 *                      Set to `NULL` in case you want to skip string in source
 * \param[in]       dst_len: Length of distance buffer,
 *                      including memory for `NULL` termination
 * \param[in]       trim: Set to `1` to process entire string,
 *                      even if no memory anymore
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsmi_parse_string(const char** src, char* dst, size_t dst_len, uint8_t trim) {
    const char* p = *src;
    size_t i;

    if (*p == ',') {
        ++p;
    }
    if (*p == '"') {
        ++p;
    }
    i = 0;
    if (dst_len > 0) {
        --dst_len;
    }
    while (*p) {
        if ((*p == '"' && (p[1] == ',' || p[1] == '\r' || p[1] == '\n'))
            || (*p == '\r' || *p == '\n')) {
            ++p;
            break;
        }
        if (dst != NULL) {
            if (i < dst_len) {
                *dst++ = *p;
                ++i;
            } else if (!trim) {
                break;
            }
        }
        ++p;
    }
    if (dst != NULL) {
        *dst = 0;
    }
    *src = p;
    return 1;
}

/**
 * \brief           Check current string position and trim to the next entry
 * \param[in]       src: Pointer to pointer to input string
 */
void
lwgsmi_check_and_trim(const char** src) {
    const char* t = *src;
    if (*t != '"' && *t != '\r' && *t != ',') { /* Check if trim required */
        lwgsmi_parse_string(src, NULL, 0, 1);   /* Trim to the end */
    }
}

/**
 * \brief           Parse string as IP address
 * \param[in,out]   src: Pointer to pointer to string to parse from
 * \param[out]      ip: Pointer to IP memory
 * \return          `1 on success, 0 otherwise
 */
uint8_t
lwgsmi_parse_ip(const char** src, lwgsm_ip_t* ip) {
    const char* p = *src;

    if (*p == ',') {
        ++p;
    }
    if (*p == '"') {
        ++p;
    }
    if (LWGSM_CHARISNUM(*p)) {
        ip->ip[0] = lwgsmi_parse_number(&p);
        ++p;
        ip->ip[1] = lwgsmi_parse_number(&p);
        ++p;
        ip->ip[2] = lwgsmi_parse_number(&p);
        ++p;
        ip->ip[3] = lwgsmi_parse_number(&p);
    }
    if (*p == '"') {
        ++p;
    }

    *src = p;                                   /* Set new pointer */
    return 1;
}

/**
 * \brief           Parse string as MAC address
 * \param[in,out]   src: Pointer to pointer to string to parse from
 * \param[out]      mac: Pointer to MAC memory
 * \return          1 on success, 0 otherwise
 */
uint8_t
lwgsmi_parse_mac(const char** src, lwgsm_mac_t* mac) {
    const char* p = *src;

    if (*p == '"') {
        ++p;
    }
    mac->mac[0] = lwgsmi_parse_hexnumber(&p);
    ++p;
    mac->mac[1] = lwgsmi_parse_hexnumber(&p);
    ++p;
    mac->mac[2] = lwgsmi_parse_hexnumber(&p);
    ++p;
    mac->mac[3] = lwgsmi_parse_hexnumber(&p);
    ++p;
    mac->mac[4] = lwgsmi_parse_hexnumber(&p);
    ++p;
    mac->mac[5] = lwgsmi_parse_hexnumber(&p);
    if (*p == '"') {
        ++p;
    }
    if (*p == ',') {
        ++p;
    }
    *src = p;
    return 1;
}

/**
 * \brief           Parse memory string, ex. "SM", "ME", "MT", etc
 * \param[in,out]   src: Pointer to pointer to string to parse from
 * \return          Parsed memory
 */
lwgsm_mem_t
lwgsmi_parse_memory(const char** src) {
    size_t i, sl;
    lwgsm_mem_t mem = LWGSM_MEM_UNKNOWN;
    const char* s = *src;

    if (*s == ',') {
        ++s;
    }
    if (*s == '"') {
        ++s;
    }

    /* Scan all memories available for modem */
    for (i = 0; i < lwgsm_dev_mem_map_size; ++i) {
        sl = strlen(lwgsm_dev_mem_map[i].mem_str);
        if (!strncmp(s, lwgsm_dev_mem_map[i].mem_str, sl)) {
            mem = lwgsm_dev_mem_map[i].mem;
            s += sl;
            break;
        }
    }

    if (mem == LWGSM_MEM_UNKNOWN) {
        lwgsmi_parse_string(&s, NULL, 0, 1);    /* Skip string */
    }
    if (*s == '"') {
        ++s;
    }
    *src = s;
    return mem;
}


/**
 * \brief           Parse a string of memories in format "M1","M2","M3","M4",...
 * \param[in,out]   src: Pointer to pointer to string to parse from
 * \param[out]      mem_dst: Output result with memory list as bit field
 * \return          1 on success, 0 otherwise
 */
uint8_t
lwgsmi_parse_memories_string(const char** src, uint32_t* mem_dst) {
    const char* str = *src;
    lwgsm_mem_t mem;

    *mem_dst = 0;
    if (*str == ',') {
        ++str;
    }
    if (*str == '(') {
        ++str;
    }
    do {
        mem = lwgsmi_parse_memory(&str);        /* Parse memory string */
        *mem_dst |= LWGSM_U32(1 << LWGSM_U32(mem)); /* Set as bit field */
    } while (*str && *str != ')');
    if (*str == ')') {
        ++str;
    }
    *src = str;
    return 1;
}

/**
 * \brief           Parse received +CREG message
 * \param[in]       str: Input string to parse from
 * \param[in]       skip_first: Set to `1` to skip first number
 * \return          1 on success, 0 otherwise
 */
uint8_t
lwgsmi_parse_creg(const char* str, uint8_t skip_first) {
    if (*str == '+') {
        str += 7;
    }

    if (skip_first) {
        lwgsmi_parse_number(&str);
    }
    lwgsm.m.network.status = (lwgsm_network_reg_status_t)lwgsmi_parse_number(&str);

    /*
     * In case we are connected to network,
     * scan for current network info
     */
    if (lwgsm.m.network.status == LWGSM_NETWORK_REG_STATUS_CONNECTED ||
        lwgsm.m.network.status == LWGSM_NETWORK_REG_STATUS_CONNECTED_ROAMING) {
        /* Try to get operator */
        /* Notify user in case we are not able to add new command to queue */
        lwgsm_operator_get(&lwgsm.m.network.curr_operator, NULL, NULL, 0);
#if LWGSM_CFG_NETWORK
    } else if (lwgsm_network_is_attached()) {
        lwgsm_network_check_status(NULL, NULL, 0);  /* Do the update */
#endif /* LWGSM_CFG_NETWORK */
    }

    /* Send callback event */
    lwgsmi_send_cb(LWGSM_EVT_NETWORK_REG_CHANGED);

    return 1;
}

/**
 * \brief           Parse received +CSQ signal value
 * \param[in]       str: Input string
 * \return          1 on success, 0 otherwise
 */
uint8_t
lwgsmi_parse_csq(const char* str) {
    int16_t rssi;
    if (*str == '+') {
        str += 6;
    }

    rssi = lwgsmi_parse_number(&str);
    if (rssi < 32) {
        rssi = -(113 - (rssi * 2));
    } else {
        rssi = 0;
    }
    lwgsm.m.rssi = rssi;                        /* Save RSSI to global variable */
    if (lwgsm.msg->cmd_def == LWGSM_CMD_CSQ_GET &&
        lwgsm.msg->msg.csq.rssi != NULL) {
        *lwgsm.msg->msg.csq.rssi = rssi;        /* Save to user variable */
    }

    /* Report CSQ status */
    lwgsm.evt.evt.rssi.rssi = rssi;
    lwgsmi_send_cb(LWGSM_EVT_SIGNAL_STRENGTH);  /* RSSI event type */

    return 1;
}

/**
 * \brief           Parse received +CPIN status value
 * \param[in]       str: Input string
 * \param[in]       send_evt: Send event about new CPIN status
 * \return          1 on success, 0 otherwise
 */
uint8_t
lwgsmi_parse_cpin(const char* str, uint8_t send_evt) {
    lwgsm_sim_state_t state;
    if (*str == '+') {
        str += 7;
    }
    if (!strncmp(str, "READY", 5)) {
        state = LWGSM_SIM_STATE_READY;
    } else if (!strncmp(str, "NOT READY", 9)) {
        state = LWGSM_SIM_STATE_NOT_READY;
    } else if (!strncmp(str, "NOT INSERTED", 14)) {
        state = LWGSM_SIM_STATE_NOT_INSERTED;
    } else if (!strncmp(str, "SIM PIN", 7)) {
        state = LWGSM_SIM_STATE_PIN;
    } else if (!strncmp(str, "SIM PUK", 7)) {
        state = LWGSM_SIM_STATE_PUK;
    } else {
        state = LWGSM_SIM_STATE_NOT_READY;
    }

    /* React only on change */
    if (state != lwgsm.m.sim.state) {
        lwgsm.m.sim.state = state;
        /*
         * In case SIM is ready,
         * start with basic info about SIM
         */
        if (lwgsm.m.sim.state == LWGSM_SIM_STATE_READY) {
            lwgsmi_get_sim_info(0);
        }

        if (send_evt) {
            lwgsm.evt.evt.cpin.state = lwgsm.m.sim.state;
            lwgsmi_send_cb(LWGSM_EVT_SIM_STATE_CHANGED);
        }
    }
    return 1;
}

/**
 * \brief           Parse +COPS string from COPS? command
 * \param[in]       str: Input string
 * \return          1 on success, 0 otherwise
 */
uint8_t
lwgsmi_parse_cops(const char* str) {
    if (*str == '+') {
        str += 7;
    }

    lwgsm.m.network.curr_operator.mode = (lwgsm_operator_mode_t)lwgsmi_parse_number(&str);
    if (*str != '\r') {
        lwgsm.m.network.curr_operator.format = (lwgsm_operator_format_t)lwgsmi_parse_number(&str);
        if (*str != '\r') {
            switch (lwgsm.m.network.curr_operator.format) {
                case LWGSM_OPERATOR_FORMAT_LONG_NAME:
                    lwgsmi_parse_string(&str, lwgsm.m.network.curr_operator.data.long_name, sizeof(lwgsm.m.network.curr_operator.data.long_name), 1);
                    break;
                case LWGSM_OPERATOR_FORMAT_SHORT_NAME:
                    lwgsmi_parse_string(&str, lwgsm.m.network.curr_operator.data.short_name, sizeof(lwgsm.m.network.curr_operator.data.short_name), 1);
                    break;
                case LWGSM_OPERATOR_FORMAT_NUMBER:
                    lwgsm.m.network.curr_operator.data.num = LWGSM_U32(lwgsmi_parse_number(&str));
                    break;
                default:
                    break;
            }
        }
    } else {
        lwgsm.m.network.curr_operator.format = LWGSM_OPERATOR_FORMAT_INVALID;
    }

    if (CMD_IS_DEF(LWGSM_CMD_COPS_GET) &&
        lwgsm.msg->msg.cops_get.curr != NULL) { /* Check and copy to user variable */
        LWGSM_MEMCPY(lwgsm.msg->msg.cops_get.curr, &lwgsm.m.network.curr_operator, sizeof(*lwgsm.msg->msg.cops_get.curr));
    }
    return 1;
}

/**
 * \brief           Parse +COPS received statement byte by byte
 * \note            Command must be active and message set to use this function
 * \param[in]       ch: New character to parse
 * \param[in]       reset: Flag to reset state machine
 * \return          1 on success, 0 otherwise
 */
uint8_t
lwgsmi_parse_cops_scan(uint8_t ch, uint8_t reset) {
    static union {
        struct {
            uint8_t bo: 1;                      /*!< Bracket open flag (Bracket Open) */
            uint8_t ccd: 1;                     /*!< 2 consecutive commas detected in a row (Comma Comma Detected) */
            uint8_t tn: 2;                      /*!< Term number in response, 2 bits for 4 diff values */
            uint8_t tp;                         /*!< Current term character position */
            uint8_t ch_prev;                    /*!< Previous character */
        } f;
    } u;

    if (reset) {                                /* Check for reset status */
        LWGSM_MEMSET(&u, 0x00, sizeof(u));      /* Reset everything */
        u.f.ch_prev = 0;
        return 1;
    }

    if (u.f.ch_prev == 0) {                     /* Check if this is first character */
        if (ch == ' ') {                        /* Skip leading spaces */
            return 1;
        } else if (ch == ',') {                 /* If first character is comma, no operators available */
            u.f.ccd = 1;                        /* Fake double commas in a row */
        }
    }

    if (u.f.ccd ||                              /* Ignore data after 2 commas in a row */
        lwgsm.msg->msg.cops_scan.opsi >= lwgsm.msg->msg.cops_scan.opsl) {   /* or if array is full */
        return 1;
    }

    if (u.f.bo) {                               /* Bracket already open */
        if (ch == ')') {                        /* Close bracket check */
            u.f.bo = 0;                         /* Clear bracket open flag */
            u.f.tn = 0;                         /* Go to next term */
            u.f.tp = 0;                         /* Go to beginning of next term */
            ++lwgsm.msg->msg.cops_scan.opsi;    /* Increase index */
            if (lwgsm.msg->msg.cops_scan.opf != NULL) {
                *lwgsm.msg->msg.cops_scan.opf = lwgsm.msg->msg.cops_scan.opsi;
            }
        } else if (ch == ',') {
            ++u.f.tn;                           /* Go to next term */
            u.f.tp = 0;                         /* Go to beginning of next term */
        } else if (ch != '"') {                 /* We have valid data */
            size_t i = lwgsm.msg->msg.cops_scan.opsi;
            switch (u.f.tn) {
                case 0: {                       /* Parse status info */
                    lwgsm.msg->msg.cops_scan.ops[i].stat = (lwgsm_operator_status_t)(10 * (size_t)lwgsm.msg->msg.cops_scan.ops[i].stat + (ch - '0'));
                    break;
                }
                case 1: {                       /*!< Parse long name */
                    if (u.f.tp < sizeof(lwgsm.msg->msg.cops_scan.ops[i].long_name) - 1) {
                        lwgsm.msg->msg.cops_scan.ops[i].long_name[u.f.tp] = ch;
                        lwgsm.msg->msg.cops_scan.ops[i].long_name[++u.f.tp] = 0;
                    }
                    break;
                }
                case 2: {                       /*!< Parse short name */
                    if (u.f.tp < sizeof(lwgsm.msg->msg.cops_scan.ops[i].short_name) - 1) {
                        lwgsm.msg->msg.cops_scan.ops[i].short_name[u.f.tp] = ch;
                        lwgsm.msg->msg.cops_scan.ops[i].short_name[++u.f.tp] = 0;
                    }
                    break;
                }
                case 3: {                       /*!< Parse number */
                    lwgsm.msg->msg.cops_scan.ops[i].num = (10 * lwgsm.msg->msg.cops_scan.ops[i].num) + (ch - '0');
                    break;
                }
                default:
                    break;
            }
        }
    } else {
        if (ch == '(') {                        /* Check for opening bracket */
            u.f.bo = 1;
        } else if (ch == ',' && u.f.ch_prev == ',') {
            u.f.ccd = 1;                        /* 2 commas in a row */
        }
    }
    u.f.ch_prev = ch;
    return 1;
}

/**
 * \brief           Parse datetime in format dd/mm/yy,hh:mm:ss
 * \param[in]       src: Pointer to pointer to input string
 * \param[out]      dt: Date time structure
 * \return          1 on success, 0 otherwise
 */
uint8_t
lwgsmi_parse_datetime(const char** src, lwgsm_datetime_t* dt) {
    dt->date = lwgsmi_parse_number(src);
    dt->month = lwgsmi_parse_number(src);
    dt->year = LWGSM_U16(2000) + lwgsmi_parse_number(src);
    dt->hours = lwgsmi_parse_number(src);
    dt->minutes = lwgsmi_parse_number(src);
    dt->seconds = lwgsmi_parse_number(src);

    lwgsmi_check_and_trim(src);                 /* Trim text to the end */
    return 1;
}

#if LWGSM_CFG_CALL || __DOXYGEN__

/**
 * \brief           Parse received +CLCC with call status info
 * \param[in]       str: Input string
 * \param[in]       send_evt: Send event about new CPIN status
 * \return          1 on success, 0 otherwise
 */
uint8_t
lwgsmi_parse_clcc(const char* str, uint8_t send_evt) {
    if (*str == '+') {
        str += 7;
    }

    lwgsm.m.call.id = lwgsmi_parse_number(&str);
    lwgsm.m.call.dir = (lwgsm_call_dir_t)lwgsmi_parse_number(&str);
    lwgsm.m.call.state = (lwgsm_call_state_t)lwgsmi_parse_number(&str);
    lwgsm.m.call.type = (lwgsm_call_type_t)lwgsmi_parse_number(&str);
    lwgsm.m.call.is_multipart = (lwgsm_call_type_t)lwgsmi_parse_number(&str);
    lwgsmi_parse_string(&str, lwgsm.m.call.number, sizeof(lwgsm.m.call.number), 1);
    lwgsm.m.call.addr_type = lwgsmi_parse_number(&str);
    lwgsmi_parse_string(&str, lwgsm.m.call.name, sizeof(lwgsm.m.call.name), 1);

    if (send_evt) {
        lwgsm.evt.evt.call_changed.call = &lwgsm.m.call;
        lwgsmi_send_cb(LWGSM_EVT_CALL_CHANGED);
    }
    return 1;
}

#endif /* LWGSM_CFG_CALL || __DOXYGEN__ */

#if LWGSM_CFG_SMS || __DOXYGEN__

/**
 * \brief           Parse string and check for type of SMS state
 * \param[in]       src: Pointer to pointer to string to parse
 * \param[out]      stat: Output status variable
 * \return          1 on success, 0 otherwise
 */
uint8_t
lwgsmi_parse_sms_status(const char** src, lwgsm_sms_status_t* stat) {
    lwgsm_sms_status_t s;
    char t[11];

    lwgsmi_parse_string(src, t, sizeof(t), 1);  /* Parse string and advance */
    if (!strcmp(t, "REC UNREAD")) {
        s = LWGSM_SMS_STATUS_UNREAD;
    } else if (!strcmp(t, "REC READ")) {
        s = LWGSM_SMS_STATUS_READ;
    } else if (!strcmp(t, "STO UNSENT")) {
        s = LWGSM_SMS_STATUS_UNSENT;
    } else if (!strcmp(t, "REC SENT")) {
        s = LWGSM_SMS_STATUS_SENT;
    } else {
        s = LWGSM_SMS_STATUS_ALL;               /* Error! */
    }
    if (s != LWGSM_SMS_STATUS_ALL) {
        *stat = s;
        return 1;
    }
    return 0;
}

/**
 * \brief           Parse received +CMGS with last sent SMS memory info
 * \param[in]       str: Input string
 * \param[in]       num: Parsed number in memory
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsmi_parse_cmgs(const char* str, size_t* num) {
    if (*str == '+') {
        str += 7;
    }

    if (num != NULL) {
        *num = (size_t)lwgsmi_parse_number(&str);
    }
    return 1;
}

/**
 * \brief           Parse +CMGR statement
 * \todo            Parse date and time from SMS entry
 * \param[in]       str: Input string
 * \return          1 on success, 0 otherwise
 */
uint8_t
lwgsmi_parse_cmgr(const char* str) {
    lwgsm_sms_entry_t* e;
    if (*str == '+') {
        str += 7;
    }

    e = lwgsm.msg->msg.sms_read.entry;
    e->length = 0;
    lwgsmi_parse_sms_status(&str, &e->status);
    lwgsmi_parse_string(&str, e->number, sizeof(e->number), 1);
    lwgsmi_parse_string(&str, e->name, sizeof(e->name), 1);
    lwgsmi_parse_datetime(&str, &e->datetime);

    return 1;
}

/**
 * \brief           Parse +CMGL statement
 * \todo            Parse date and time from SMS entry
 * \param[in]       str: Input string
 * \return          1 on success, 0 otherwise
 */
uint8_t
lwgsmi_parse_cmgl(const char* str) {
    lwgsm_sms_entry_t* e;

    if (!CMD_IS_DEF(LWGSM_CMD_CMGL) ||
        lwgsm.msg->msg.sms_list.ei >= lwgsm.msg->msg.sms_list.etr) {
        return 0;
    }

    if (*str == '+') {
        str += 7;
    }

    e = &lwgsm.msg->msg.sms_list.entries[lwgsm.msg->msg.sms_list.ei];
    e->length = 0;
    e->mem = lwgsm.msg->msg.sms_list.mem;       /* Manually set memory */
    e->pos = LWGSM_SZ(lwgsmi_parse_number(&str));   /* Scan position */
    lwgsmi_parse_sms_status(&str, &e->status);
    lwgsmi_parse_string(&str, e->number, sizeof(e->number), 1);
    lwgsmi_parse_string(&str, e->name, sizeof(e->name), 1);
    lwgsmi_parse_datetime(&str, &e->datetime);

    return 1;
}

/**
 * \brief           Parse received +CMTI with received SMS info
 * \param[in]       str: Input string
 * \param[in]       send_evt: Send event about new CPIN status
 * \return          1 on success, 0 otherwise
 */
uint8_t
lwgsmi_parse_cmti(const char* str, uint8_t send_evt) {
    if (*str == '+') {
        str += 7;
    }

    lwgsm.evt.evt.sms_recv.mem = lwgsmi_parse_memory(&str); /* Parse memory string */
    lwgsm.evt.evt.sms_recv.pos = lwgsmi_parse_number(&str); /* Parse number */

    if (send_evt) {
        lwgsmi_send_cb(LWGSM_EVT_SMS_RECV);
    }
    return 1;
}

/**
 * \brief           Parse +CPMS statement
 * \param[in]       str: Input string
 * \param[in]       opt: Expected input: 0 = CPMS_OPT, 1 = CPMS_GET, 2 = CPMS_SET
 * \return          1 on success, 0 otherwise
 */
uint8_t
lwgsmi_parse_cpms(const char* str, uint8_t opt) {
    uint8_t i;
    if (*str == '+') {
        str += 7;
    }
    switch (opt) {                              /* Check expected input string */
        case 0: {                               /* Get list of CPMS options: +CPMS: (("","","",..),("....")("...")) */
            for (i = 0; i < 3; ++i) {           /* 3 different memories for "operation","receive","sent" */
                if (!lwgsmi_parse_memories_string(&str, &lwgsm.m.sms.mem[i].mem_available)) {
                    return 0;
                }
            }
            break;
        }
        case 1: {                               /* Received statement of current info: +CPMS: "ME",10,20,"SE",2,20,"... */
            for (i = 0; i < 3; ++i) {           /* 3 memories expected */
                lwgsm.m.sms.mem[i].current = lwgsmi_parse_memory(&str); /* Parse memory string and save it as current */
                lwgsm.m.sms.mem[i].used = lwgsmi_parse_number(&str);/* Get used memory size */
                lwgsm.m.sms.mem[i].total = lwgsmi_parse_number(&str);   /* Get total memory size */
            }
            break;
        }
        case 2: {                               /* Received statement of set info: +CPMS: 10,20,2,20 */
            for (i = 0; i < 3; ++i) {           /* 3 memories expected */
                lwgsm.m.sms.mem[i].used = lwgsmi_parse_number(&str);/* Get used memory size */
                lwgsm.m.sms.mem[i].total = lwgsmi_parse_number(&str);   /* Get total memory size */
            }
            break;
        }
        default:
            break;
    }
    return 1;
}

#endif /* LWGSM_CFG_SMS || __DOXYGEN__ */

#if LWGSM_CFG_PHONEBOOK || __DOXYGEN__

/**
 * \brief           Parse +CPBS statement
 * \param[in]       str: Input string
 * \param[in]       opt: Expected input: 0 = CPBS_OPT, 1 = CPBS_GET, 2 = CPBS_SET
 * \return          1 on success, 0 otherwise
 */
uint8_t
lwgsmi_parse_cpbs(const char* str, uint8_t opt) {
    if (*str == '+') {
        str += 7;
    }
    switch (opt) {                              /* Check expected input string */
        case 0: {                               /* Get list of CPBS options: ("M1","M2","M3",...) */
            return lwgsmi_parse_memories_string(&str, &lwgsm.m.pb.mem.mem_available);
        }
        case 1: {                               /* Received statement of current info: +CPBS: "ME",10,20 */
            lwgsm.m.pb.mem.current = lwgsmi_parse_memory(&str); /* Parse memory string and save it as current */
            lwgsm.m.pb.mem.used = lwgsmi_parse_number(&str);/* Get used memory size */
            lwgsm.m.pb.mem.total = lwgsmi_parse_number(&str);   /* Get total memory size */
            break;
        }
        case 2: {                               /* Received statement of set info: +CPBS: 10,20 */
            lwgsm.m.pb.mem.used = lwgsmi_parse_number(&str);/* Get used memory size */
            lwgsm.m.pb.mem.total = lwgsmi_parse_number(&str);   /* Get total memory size */
            break;
        }
    }
    return 1;
}

/**
 * \brief           Parse +CPBR statement
 * \param[in]       str: Input string
 * \return          1 on success, 0 otherwise
 */
uint8_t
lwgsmi_parse_cpbr(const char* str) {
    lwgsm_pb_entry_t* e;

    if (!CMD_IS_DEF(LWGSM_CMD_CPBR) ||
        lwgsm.msg->msg.pb_list.ei >= lwgsm.msg->msg.pb_list.etr) {
        return 0;
    }

    if (*str == '+') {
        str += 7;
    }

    e = &lwgsm.msg->msg.pb_list.entries[lwgsm.msg->msg.pb_list.ei];
    e->pos = LWGSM_SZ(lwgsmi_parse_number(&str));
    lwgsmi_parse_string(&str, e->number, sizeof(e->number), 1);
    e->type = (lwgsm_number_type_t)lwgsmi_parse_number(&str);
    lwgsmi_parse_string(&str, e->name, sizeof(e->name), 1);

    ++lwgsm.msg->msg.pb_list.ei;
    if (lwgsm.msg->msg.pb_list.er != NULL) {
        *lwgsm.msg->msg.pb_list.er = lwgsm.msg->msg.pb_list.ei;
    }
    return 1;
}

/**
 * \brief           Parse +CPBF statement
 * \param[in]       str: Input string
 * \return          1 on success, 0 otherwise
 */
uint8_t
lwgsmi_parse_cpbf(const char* str) {
    lwgsm_pb_entry_t* e;

    if (!CMD_IS_DEF(LWGSM_CMD_CPBF) ||
        lwgsm.msg->msg.pb_search.ei >= lwgsm.msg->msg.pb_search.etr) {
        return 0;
    }

    if (*str == '+') {
        str += 7;
    }

    e = &lwgsm.msg->msg.pb_search.entries[lwgsm.msg->msg.pb_search.ei];
    e->pos = LWGSM_SZ(lwgsmi_parse_number(&str));
    lwgsmi_parse_string(&str, e->name, sizeof(e->name), 1);
    e->type = (lwgsm_number_type_t)lwgsmi_parse_number(&str);
    lwgsmi_parse_string(&str, e->number, sizeof(e->number), 1);

    ++lwgsm.msg->msg.pb_search.ei;
    if (lwgsm.msg->msg.pb_search.er != NULL) {
        *lwgsm.msg->msg.pb_search.er = lwgsm.msg->msg.pb_search.ei;
    }
    return 1;
}

#endif /* LWGSM_CFG_PHONEBOOK || __DOXYGEN__ */

#if LWGSM_CFG_CONN

/**
 * \brief           Parse connection info line from CIPSTATUS command
 * \param[in]       str: Input string
 * \param[in]       is_conn_line: Set to `1` for connection, `0` for general status
 * \param[out]      continueScan: Pointer to output variable holding continue processing state
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsmi_parse_cipstatus_conn(const char* str, uint8_t is_conn_line, uint8_t* continueScan) {
    uint8_t num;
    lwgsm_conn_t* conn;
    char s_tmp[16];
    uint8_t tmp_pdp_state;

    *continueScan = 1;
    if (is_conn_line && (*str == 'C' || *str == 'S')) {
        str += 3;
    } else {
        /* Check if PDP context is deactivated or not */
        tmp_pdp_state = 1;
        if (!strncmp(&str[7], "IP INITIAL", 10)) {
            *continueScan = 0;                  /* Stop command execution at this point (no OK,ERROR received after this line) */
            tmp_pdp_state = 0;
        } else if (!strncmp(&str[7], "PDP DEACT", 9)) {
            /* Deactivated */
            tmp_pdp_state = 0;
        }

        /* Check if we have to update status for application */
        if (lwgsm.m.network.is_attached != tmp_pdp_state) {
            lwgsm.m.network.is_attached = tmp_pdp_state;

            /* Notify upper layer */
            lwgsmi_send_cb(lwgsm.m.network.is_attached ? LWGSM_EVT_NETWORK_ATTACHED : LWGSM_EVT_NETWORK_DETACHED);
        }

        return 1;
    }

    /* Parse connection line */
    num = LWGSM_U8(lwgsmi_parse_number(&str));
    conn = &lwgsm.m.conns[num];

    conn->status.f.bearer = LWGSM_U8(lwgsmi_parse_number(&str));
    lwgsmi_parse_string(&str, s_tmp, sizeof(s_tmp), 1); /* Parse TCP/UPD */
    if (strlen(s_tmp)) {
        if (!strcmp(s_tmp, "TCP")) {
            conn->type = LWGSM_CONN_TYPE_TCP;
        } else if (!strcmp(s_tmp, "UDP")) {
            conn->type = LWGSM_CONN_TYPE_UDP;
        }
    }
    lwgsmi_parse_ip(&str, &conn->remote_ip);
    conn->remote_port = lwgsmi_parse_number(&str);

    /* Get connection status */
    lwgsmi_parse_string(&str, s_tmp, sizeof(s_tmp), 1);

    /* TODO: Implement all connection states */
    if (!strcmp(s_tmp, "INITIAL")) {

    } else if (!strcmp(s_tmp, "CONNECTING")) {

    } else if (!strcmp(s_tmp, "CONNECTED")) {

    } else if (!strcmp(s_tmp, "REMOTE CLOSING")) {

    } else if (!strcmp(s_tmp, "CLOSING")) {

    } else if (!strcmp(s_tmp, "CLOSED")) {      /* Connection closed */
        if (conn->status.f.active) {            /* Check if connection is not */
            lwgsmi_conn_closed_process(conn->num, 0);   /* Process closed event */
        }
    }

    /* Save last parsed connection */
    lwgsm.m.active_conns_cur_parse_num = num;

    return 1;
}

/**
 * \brief           Parse IPD or RECEIVE statements
 * \param[in]       str: Input string
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsmi_parse_ipd(const char* str) {
    uint8_t conn;
    size_t len;
    lwgsm_conn_p c;

    if (*str == '+') {
        ++str;
        if (*str == 'R') {
            str += 8;                           /* Advance for RECEIVE */
        } else {
            str += 4;                           /* Advance for IPD */
        }
    }

    conn = lwgsmi_parse_number(&str);           /* Parse number for connection number */
    len = lwgsmi_parse_number(&str);            /* Parse number for number of bytes to read */

    c = conn < LWGSM_CFG_MAX_CONNS ? &lwgsm.m.conns[conn] : NULL;   /* Get connection handle */
    if (c == NULL) {                            /* Invalid connection number */
        return 0;
    }

    lwgsm.m.ipd.read = 1;                       /* Start reading network data */
    lwgsm.m.ipd.tot_len = len;                  /* Total number of bytes in this received packet */
    lwgsm.m.ipd.rem_len = len;                  /* Number of remaining bytes to read */
    lwgsm.m.ipd.conn = c;                       /* Pointer to connection we have data for */

    return 1;
}

#endif /* LWGSM_CFG_CONN */
