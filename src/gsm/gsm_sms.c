/**	
 * \file            gsm_sms.c
 * \brief           SMS API
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
#include "gsm/gsm_sms.h"
#include "gsm/gsm_mem.h"

#if GSM_CFG_SMS || __DOXYGEN__

#define GSM_SMS_READ_IDX                0   /*!< Read index for memory array */
#define GSM_SMS_SEND_IDX                1   /*!< Send index for memory array */
#define GSM_SMS_RECEIVE_IDX             2   /*!< Receive index for memory array */

/**
 * \brief           Check if input memory is available in modem
 * \param[in]       mem: Memory to test
 * \param[in]       can_curr: Flag indicates if \ref GSM_MEM_CURRENT option can be used
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
static gsmr_t
check_sms_mem(gsm_mem_t mem, uint8_t can_curr) {
    gsmr_t res = gsmERRMEM;
    GSM_CORE_PROTECT();
    if ((mem < GSM_MEM_END && gsm.mem_list_sms[GSM_SMS_READ_IDX] & (1 << (uint32_t)mem)) ||
        (can_curr && mem == GSM_MEM_CURRENT)) {
        res = gsmOK;
    }
    GSM_CORE_UNPROTECT();
    return res;
}

/**
 * \brief           Send SMS text to phone number
 * \param[in]       num: String number
 * \param[in]       text: Text to send. Maximal `160` characters
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
gsmr_t
gsm_sms_send(const char* num, const char* text, uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);                    /* Define variable for message */

    GSM_ASSERT("num != NULL", num != NULL);     /* Assert input parameters */
    GSM_ASSERT("text != NULL && strlen(text) <= 160", 
        num != NULL && strlen(text) <= 160);    /* Assert input parameters */

    GSM_MSG_VAR_ALLOC(msg);                     /* Allocate memory for variable */
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_CMGS;
    GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CMGF;
    GSM_MSG_VAR_REF(msg).msg.sms_send.num = num;
    GSM_MSG_VAR_REF(msg).msg.sms_send.text = text;
    GSM_MSG_VAR_REF(msg).msg.sms_send.format = 1;   /* Send as plain text */

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, blocking, 60000);   /* Send message to producer queue */
}

/**
 * \brief           Read SMS entry at specific memory and position
 * \param[in]       mem: Memory used to read message from
 * \param[in]       pos: Position number in memory to read
 * \param[out]      entry: Pointer to SMS entry structure to fill data to
 * \param[in]       update: Flag indicates update. Set to `1` to change `UNREAD` messages to `READ` or `0` to leave as is
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
gsmr_t
gsm_sms_read(gsm_mem_t mem, size_t pos, gsm_sms_entry_t* entry, uint8_t update, uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);                    /* Define variable for message */

    GSM_ASSERT("sms_entry != NULL", entry != NULL); /* Assert input parameters */
    GSM_ASSERT("mem", check_sms_mem(mem, 1) == gsmOK);  /* Assert input parameters */

    memset(entry, 0x00, sizeof(*entry));        /* Reset data structure */

    /**
     * \todo: Get current memory if \ref GSM_MEM_CURRENT is used
     */
    entry->mem = mem;                           /* Set device memory */
    entry->pos = pos;                           /* Set device position */

    GSM_MSG_VAR_ALLOC(msg);                     /* Allocate memory for variable */
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_CMGR;
    if (mem != GSM_MEM_CURRENT) {
        GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CPMS_SET;    /* First set memory */
    } else {
        GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CMGF;/* Start with text mode option */
    }
    GSM_MSG_VAR_REF(msg).msg.sms_read.mem = mem;
    GSM_MSG_VAR_REF(msg).msg.sms_read.pos = pos;
    GSM_MSG_VAR_REF(msg).msg.sms_read.entry = entry;
    GSM_MSG_VAR_REF(msg).msg.sms_read.update = update;
    GSM_MSG_VAR_REF(msg).msg.sms_read.format = 1;   /* Send as plain text */

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, blocking, 60000);   /* Send message to producer queue */
}

/**
 * \brief           Delete SMS entry at specific memory and position
 * \param[in]       mem: Memory used to read message from
 * \param[in]       pos: Position number in memory to read
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
gsmr_t
gsm_sms_delete(gsm_mem_t mem, size_t pos, uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);                    /* Define variable for message */

    GSM_ASSERT("mem", check_sms_mem(mem, 1) == gsmOK);  /* Assert input parameters */

    GSM_MSG_VAR_ALLOC(msg);                     /* Allocate memory for variable */
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_CMGD;
    if (mem != GSM_MEM_CURRENT) {
        GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CPMS_SET;    /* First set memory */
    }
    GSM_MSG_VAR_REF(msg).msg.sms_delete.mem = mem;
    GSM_MSG_VAR_REF(msg).msg.sms_delete.pos = pos;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, blocking, 60000);   /* Send message to producer queue */
}

gsmr_t
gsm_sms_list(gsm_mem_t mem, gsm_sms_status_t stat, gsm_sms_entry_t* entries, size_t etr, size_t* er, uint8_t update, uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);                    /* Define variable for message */

    GSM_ASSERT("mem", check_sms_mem(mem, 1) == gsmOK);  /* Assert input parameters */
    GSM_ASSERT("entires != NULL", entries != NULL); /* Assert input parameters */
    GSM_ASSERT("etr > 0", etr > 0);             /* Assert input parameters */

    memset(entries, 0x00, sizeof(*entries) * etr);  /* Reset data structure */

    GSM_MSG_VAR_ALLOC(msg);                     /* Allocate memory for variable */
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_CMGL;
    if (mem != GSM_MEM_CURRENT) {
        GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CPMS_SET;    /* First set memory */
    } else {
        GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CMGF;/* Set mode first */
    }
    GSM_MSG_VAR_REF(msg).msg.sms_list.mem = mem;
    GSM_MSG_VAR_REF(msg).msg.sms_list.status = stat;
    GSM_MSG_VAR_REF(msg).msg.sms_list.entries = entries;
    GSM_MSG_VAR_REF(msg).msg.sms_list.etr = etr;
    GSM_MSG_VAR_REF(msg).msg.sms_list.er = er;
    GSM_MSG_VAR_REF(msg).msg.sms_list.update = update;
    GSM_MSG_VAR_REF(msg).msg.sms_list.format = 1;   /* Send as plain text */

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, blocking, 60000);   /* Send message to producer queue */
}

#endif /* GSM_CFG_SMS || __DOXYGEN__ */
