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
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
static gsmr_t
check_sms_mem(gsm_mem_t mem) {
    gsmr_t res = gsmERRMEM;
    GSM_CORE_PROTECT();
    if (mem < GSM_MEM_END &&
            gsm.mem_list_sms[GSM_SMS_READ_IDX] & (1 << (uint32_t)mem)) {
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
 * \brief           Read SMS entry
 * \param[in]       mem: Memory used to read message from
 * \param[in]       pos: Position number in memory to read
 * \param[out]      sms_entry: Pointer to SMS entry structure to fill data to
 * \param[in]       update: Flag indicates update. Set to `1` to change `UNREAD` messages to `READ` or `0` to leave as is
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
gsmr_t
gsm_sms_read(gsm_mem_t mem, uint16_t pos, gsm_sms_entry_t* sms_entry, uint8_t update, uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);                    /* Define variable for message */

    GSM_ASSERT("sms_entry != NULL", sms_entry != NULL); /* Assert input parameters */
    GSM_ASSERT("sms_mem", check_sms_mem(mem) == gsmOK); /* Assert input parameters */

    memset(sms_entry, 0x00, sizeof(*sms_entry));/* Reset data structure */

    GSM_MSG_VAR_ALLOC(msg);                     /* Allocate memory for variable */
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_CMGR;
    GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CMGF;
    GSM_MSG_VAR_REF(msg).msg.sms_read.mem = mem;
    GSM_MSG_VAR_REF(msg).msg.sms_read.pos = pos;
    GSM_MSG_VAR_REF(msg).msg.sms_read.entry = sms_entry;
    GSM_MSG_VAR_REF(msg).msg.sms_read.update = update;
    GSM_MSG_VAR_REF(msg).msg.sms_read.format = 1;   /* Send as plain text */

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, blocking, 60000);   /* Send message to producer queue */
}


#endif /* GSM_CFG_SMS || __DOXYGEN__ */
