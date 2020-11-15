/**
 * \file            lwgsm_sms.c
 * \brief           SMS API
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
#include "lwgsm/lwgsm_sms.h"
#include "lwgsm/lwgsm_mem.h"

#if LWGSM_CFG_SMS || __DOXYGEN__

#define LWGSM_SMS_OPERATION_IDX           0     /*!< Operation index for memory array (read, delete, list) */
#define LWGSM_SMS_SEND_IDX                1     /*!< Send index for memory array */
#define LWGSM_SMS_RECEIVE_IDX             2     /*!< Receive index for memory array */

#if !__DOXYGEN__
#define CHECK_ENABLED()                 if (!(check_enabled() == lwgsmOK)) { return lwgsmERRNOTENABLED; }
#define CHECK_READY()                   if (!(check_ready() == lwgsmOK)) { return lwgsmERR; }
#endif /* !__DOXYGEN__ */

/**
 * \brief           Check if sms is enabled
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
static lwgsmr_t
check_enabled(void) {
    lwgsmr_t res;
    lwgsm_core_lock();
    res = lwgsm.m.sms.enabled ? lwgsmOK : lwgsmERR;
    lwgsm_core_unlock();
    return res;
}

/**
 * \brief           Check if SMS is available
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
static lwgsmr_t
check_ready(void) {
    lwgsmr_t res;
    lwgsm_core_lock();
    res = lwgsm.m.sms.ready ? lwgsmOK : lwgsmERR;
    lwgsm_core_unlock();
    return res;
}

/**
 * \brief           Check if input memory is available in modem
 * \param[in]       mem: Memory to test
 * \param[in]       can_curr: Flag indicates if \ref LWGSM_MEM_CURRENT option can be used
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
static lwgsmr_t
check_sms_mem(lwgsm_mem_t mem, uint8_t can_curr) {
    lwgsmr_t res = lwgsmERRMEM;
    lwgsm_core_lock();
    if ((mem < LWGSM_MEM_END && lwgsm.m.sms.mem[LWGSM_SMS_OPERATION_IDX].mem_available & (1 << (uint32_t)mem)) ||
        (can_curr && mem == LWGSM_MEM_CURRENT)) {
        res = lwgsmOK;
    }
    lwgsm_core_unlock();
    return res;
}

/**
 * \brief           Enable SMS functionality
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_sms_enable(const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_SMS_ENABLE;
    LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CPMS_GET_OPT;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

/**
 * \brief           Disable SMS functionality
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_sms_disable(const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    lwgsm_core_lock();
    lwgsm.m.sms.enabled = 0;
    if (evt_fn != NULL) {
        evt_fn(lwgsmOK, evt_arg);
    }
    lwgsm_core_unlock();
    LWGSM_UNUSED(blocking);
    return lwgsmOK;
}

/**
 * \brief           Send SMS text to phone number
 * \param[in]       num: String number
 * \param[in]       text: Text to send. Maximal `160` characters
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_sms_send(const char* num, const char* text,
             const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_ASSERT("num != NULL && num[0] > 0", num != NULL && num[0] > 0);
    LWGSM_ASSERT("text != NULL && text[0] > 0 && strlen(text) <= 160", text != NULL && text[0] > 0 && strlen(text) <= 160);
    CHECK_ENABLED();                            /* Check if enabled */
    CHECK_READY();                              /* Check if ready */

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_CMGS;
    LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CMGF;
    LWGSM_MSG_VAR_REF(msg).msg.sms_send.num = num;
    LWGSM_MSG_VAR_REF(msg).msg.sms_send.text = text;
    LWGSM_MSG_VAR_REF(msg).msg.sms_send.format = 1; /* Send as plain text */

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

/**
 * \brief           Read SMS entry at specific memory and position
 * \param[in]       mem: Memory used to read message from
 * \param[in]       pos: Position number in memory to read
 * \param[out]      entry: Pointer to SMS entry structure to fill data to
 * \param[in]       update: Flag indicates update. Set to `1` to change `UNREAD` messages to `READ` or `0` to leave as is
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_sms_read(lwgsm_mem_t mem, size_t pos, lwgsm_sms_entry_t* entry, uint8_t update,
             const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_ASSERT("entry != NULL", entry != NULL);
    CHECK_ENABLED();                            /* Check if enabled */
    CHECK_READY();                              /* Check if ready */
    LWGSM_ASSERT("check_sms_mem() == lwgsmOK", check_sms_mem(mem, 1) == lwgsmOK);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);

    LWGSM_MEMSET(entry, 0x00, sizeof(*entry));  /* Reset data structure */

    entry->mem = mem;                           /* Set memory */
    entry->pos = pos;                           /* Set device position */
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_CMGR;
    if (mem == LWGSM_MEM_CURRENT) {             /* Should be always false */
        LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CPMS_GET;/* First get memory */
    } else {
        LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CPMS_SET;/* First set memory */
    }
    LWGSM_MSG_VAR_REF(msg).msg.sms_read.mem = mem;
    LWGSM_MSG_VAR_REF(msg).msg.sms_read.pos = pos;
    LWGSM_MSG_VAR_REF(msg).msg.sms_read.entry = entry;
    LWGSM_MSG_VAR_REF(msg).msg.sms_read.update = update;
    LWGSM_MSG_VAR_REF(msg).msg.sms_read.format = 1; /* Send as plain text */

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

/**
 * \brief           Delete SMS entry at specific memory and position
 * \param[in]       mem: Memory used to read message from
 * \param[in]       pos: Position number in memory to read
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_sms_delete(lwgsm_mem_t mem, size_t pos,
               const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    CHECK_ENABLED();                            /* Check if enabled */
    CHECK_READY();                              /* Check if ready */
    LWGSM_ASSERT("check_sms_mem() == lwgsmOK", check_sms_mem(mem, 1) == lwgsmOK);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_CMGD;
    if (mem == LWGSM_MEM_CURRENT) {             /* Should be always false */
        LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CPMS_GET;/* First get memory */
    } else {
        LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CPMS_SET;/* First set memory */
    }
    LWGSM_MSG_VAR_REF(msg).msg.sms_delete.mem = mem;
    LWGSM_MSG_VAR_REF(msg).msg.sms_delete.pos = pos;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 1000);
}

/**
 * \brief           Delete all SMS entries with specific status
 * \param[in]       status: SMS status. This parameter can be one of all possible types in \ref lwgsm_sms_status_t enumeration
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_sms_delete_all(lwgsm_sms_status_t status,
                   const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    CHECK_ENABLED();                            /* Check if enabled */
    CHECK_READY();                              /* Check if ready */

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_CMGDA;
    LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CMGF;/* By default format = 1 */
    LWGSM_MSG_VAR_REF(msg).msg.sms_delete_all.status = status;

    /* This command may take a while */
    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

/**
 * \brief           List SMS from SMS memory
 * \param[in]       mem: Memory to read entries from. Use \ref LWGSM_MEM_CURRENT to read from current memory
 * \param[in]       stat: SMS status to read, either `read`, `unread`, `sent`, `unsent` or `all`
 * \param[out]      entries: Pointer to array to save SMS entries
 * \param[in]       etr: Number of entries to read
 * \param[out]      er: Pointer to output variable to save number of entries in array
 * \param[in]       update: Flag indicates update. Set to `1` to change `UNREAD` messages to `READ` or `0` to leave as is
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_sms_list(lwgsm_mem_t mem, lwgsm_sms_status_t stat, lwgsm_sms_entry_t* entries, size_t etr, size_t* er, uint8_t update,
             const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_ASSERT("entires != NULL", entries != NULL);
    LWGSM_ASSERT("etr > 0", etr > 0);
    CHECK_ENABLED();                            /* Check if enabled */
    CHECK_READY();                              /* Check if ready */
    LWGSM_ASSERT("check_sms_mem() == lwgsmOK", check_sms_mem(mem, 1) == lwgsmOK);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);

    if (er != NULL) {
        *er = 0;
    }
    LWGSM_MEMSET(entries, 0x00, sizeof(*entries) * etr);/* Reset data structure */
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_CMGL;
    if (mem == LWGSM_MEM_CURRENT) {             /* Should be always false */
        LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CPMS_GET;/* First get memory */
    } else {
        LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CPMS_SET;/* First set memory */
    }
    LWGSM_MSG_VAR_REF(msg).msg.sms_list.mem = mem;
    LWGSM_MSG_VAR_REF(msg).msg.sms_list.status = stat;
    LWGSM_MSG_VAR_REF(msg).msg.sms_list.entries = entries;
    LWGSM_MSG_VAR_REF(msg).msg.sms_list.etr = etr;
    LWGSM_MSG_VAR_REF(msg).msg.sms_list.er = er;
    LWGSM_MSG_VAR_REF(msg).msg.sms_list.update = update;
    LWGSM_MSG_VAR_REF(msg).msg.sms_list.format = 1; /* Send as plain text */

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

/**
 * \brief           Set preferred storage for SMS
 * \param[in]       mem1: Preferred memory for read/delete SMS operations. Use \ref LWGSM_MEM_CURRENT to keep it as is
 * \param[in]       mem2: Preferred memory for sent/write SMS operations. Use \ref LWGSM_MEM_CURRENT to keep it as is
 * \param[in]       mem3: Preferred memory for received SMS entries. Use \ref LWGSM_MEM_CURRENT to keep it as is
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_sms_set_preferred_storage(lwgsm_mem_t mem1, lwgsm_mem_t mem2, lwgsm_mem_t mem3,
                              const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    CHECK_ENABLED();                            /* Check if enabled */
    CHECK_READY();                              /* Check if ready */
    LWGSM_ASSERT("check_sms_mem(1) == lwgsmOK", check_sms_mem(mem1, 1) == lwgsmOK);
    LWGSM_ASSERT("check_sms_mem(2) == lwgsmOK", check_sms_mem(mem2, 1) == lwgsmOK);
    LWGSM_ASSERT("check_sms_mem(3) == lwgsmOK", check_sms_mem(mem3, 1) == lwgsmOK);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_CPMS_SET;

    /* In case any of memories is set to current, read current status first from device */
    if (mem1 == LWGSM_MEM_CURRENT || mem2 == LWGSM_MEM_CURRENT || mem3 == LWGSM_MEM_CURRENT) {
        LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CPMS_GET;
    }
    LWGSM_MSG_VAR_REF(msg).msg.sms_memory.mem[0] = mem1;
    LWGSM_MSG_VAR_REF(msg).msg.sms_memory.mem[1] = mem2;
    LWGSM_MSG_VAR_REF(msg).msg.sms_memory.mem[2] = mem3;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

#endif /* LWGSM_CFG_SMS || __DOXYGEN__ */
