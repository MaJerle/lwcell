/**
 * \file            lwcell_sms.c
 * \brief           SMS API
 */

/*
 * Copyright (c) 2023 Tilen MAJERLE
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
 * This file is part of LwCELL - Lightweight cellular modem AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.1
 */
#include "lwcell/lwcell_sms.h"
#include "lwcell/lwcell_private.h"

#if LWCELL_CFG_SMS || __DOXYGEN__

#define LWCELL_SMS_OPERATION_IDX 0 /*!< Operation index for memory array (read, delete, list) */
#define LWCELL_SMS_SEND_IDX      1 /*!< Send index for memory array */
#define LWCELL_SMS_RECEIVE_IDX   2 /*!< Receive index for memory array */

#if !__DOXYGEN__
#define CHECK_ENABLED()                                                                                                \
    if (!(check_enabled() == lwcellOK)) {                                                                              \
        return lwcellERRNOTENABLED;                                                                                    \
    }
#define CHECK_READY()                                                                                                  \
    if (!(check_ready() == lwcellOK)) {                                                                                \
        return lwcellERR;                                                                                              \
    }
#endif /* !__DOXYGEN__ */

/**
 * \brief           Check if sms is enabled
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
static lwcellr_t
check_enabled(void) {
    lwcellr_t res;
    lwcell_core_lock();
    res = lwcell.m.sms.enabled ? lwcellOK : lwcellERR;
    lwcell_core_unlock();
    return res;
}

/**
 * \brief           Check if SMS is available
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
static lwcellr_t
check_ready(void) {
    lwcellr_t res;
    lwcell_core_lock();
    res = lwcell.m.sms.ready ? lwcellOK : lwcellERR;
    lwcell_core_unlock();
    return res;
}

/**
 * \brief           Check if input memory is available in modem
 * \param[in]       mem: Memory to test
 * \param[in]       can_curr: Flag indicates if \ref LWCELL_MEM_CURRENT option can be used
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
static lwcellr_t
check_sms_mem(lwcell_mem_t mem, uint8_t can_curr) {
    lwcellr_t res = lwcellERRMEM;
    lwcell_core_lock();
    if ((mem < LWCELL_MEM_END && lwcell.m.sms.mem[LWCELL_SMS_OPERATION_IDX].mem_available & (1 << (uint32_t)mem))
        || (can_curr && mem == LWCELL_MEM_CURRENT)) {
        res = lwcellOK;
    }
    lwcell_core_unlock();
    return res;
}

/**
 * \brief           Enable SMS functionality
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_sms_enable(const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_SMS_ENABLE;
    LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CPMS_GET_OPT;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 60000);
}

/**
 * \brief           Disable SMS functionality
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_sms_disable(const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    lwcell_core_lock();
    lwcell.m.sms.enabled = 0;
    if (evt_fn != NULL) {
        evt_fn(lwcellOK, evt_arg);
    }
    lwcell_core_unlock();
    LWCELL_UNUSED(blocking);
    return lwcellOK;
}

/**
 * \brief           Send SMS text to phone number
 * \param[in]       num: String number
 * \param[in]       text: Text to send. Maximal `160` characters
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_sms_send(const char* num, const char* text, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_ASSERT(num != NULL && num[0] > 0);
    LWCELL_ASSERT(text != NULL && text[0] > 0 && strlen(text) <= 160);
    CHECK_ENABLED(); /* Check if enabled */
    CHECK_READY();   /* Check if ready */

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CMGS;
    LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CMGF;
    LWCELL_MSG_VAR_REF(msg).msg.sms_send.num = num;
    LWCELL_MSG_VAR_REF(msg).msg.sms_send.text = text;
    LWCELL_MSG_VAR_REF(msg).msg.sms_send.format = 1; /* Send as plain text */

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 60000);
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
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_sms_read(lwcell_mem_t mem, size_t pos, lwcell_sms_entry_t* entry, uint8_t update,
                const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_ASSERT(entry != NULL);
    CHECK_ENABLED(); /* Check if enabled */
    CHECK_READY();   /* Check if ready */
    LWCELL_ASSERT(check_sms_mem(mem, 1) == lwcellOK);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);

    LWCELL_MEMSET(entry, 0x00, sizeof(*entry)); /* Reset data structure */

    entry->mem = mem; /* Set memory */
    entry->pos = pos; /* Set device position */
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CMGR;
    if (mem == LWCELL_MEM_CURRENT) {                       /* Should be always false */
        LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CPMS_GET; /* First get memory */
    } else {
        LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CPMS_SET; /* First set memory */
    }
    LWCELL_MSG_VAR_REF(msg).msg.sms_read.mem = mem;
    LWCELL_MSG_VAR_REF(msg).msg.sms_read.pos = pos;
    LWCELL_MSG_VAR_REF(msg).msg.sms_read.entry = entry;
    LWCELL_MSG_VAR_REF(msg).msg.sms_read.update = update;
    LWCELL_MSG_VAR_REF(msg).msg.sms_read.format = 1; /* Send as plain text */

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 60000);
}

/**
 * \brief           Delete SMS entry at specific memory and position
 * \param[in]       mem: Memory used to read message from
 * \param[in]       pos: Position number in memory to read
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_sms_delete(lwcell_mem_t mem, size_t pos, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                  const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    CHECK_ENABLED(); /* Check if enabled */
    CHECK_READY();   /* Check if ready */
    LWCELL_ASSERT(check_sms_mem(mem, 1) == lwcellOK);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CMGD;
    if (mem == LWCELL_MEM_CURRENT) {                       /* Should be always false */
        LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CPMS_GET; /* First get memory */
    } else {
        LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CPMS_SET; /* First set memory */
    }
    LWCELL_MSG_VAR_REF(msg).msg.sms_delete.mem = mem;
    LWCELL_MSG_VAR_REF(msg).msg.sms_delete.pos = pos;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 1000);
}

/**
 * \brief           Delete all SMS entries with specific status
 * \param[in]       status: SMS status. This parameter can be one of all possible types in \ref lwcell_sms_status_t enumeration
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_sms_delete_all(lwcell_sms_status_t status, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                      const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    CHECK_ENABLED(); /* Check if enabled */
    CHECK_READY();   /* Check if ready */

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CMGDA;
    LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CMGF; /* By default format = 1 */
    LWCELL_MSG_VAR_REF(msg).msg.sms_delete_all.status = status;

    /* This command may take a while */
    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 60000);
}

/**
 * \brief           List SMS from SMS memory
 * \param[in]       mem: Memory to read entries from. Use \ref LWCELL_MEM_CURRENT to read from current memory
 * \param[in]       stat: SMS status to read, either `read`, `unread`, `sent`, `unsent` or `all`
 * \param[out]      entries: Pointer to array to save SMS entries
 * \param[in]       etr: Number of entries to read
 * \param[out]      er: Pointer to output variable to save number of entries in array
 * \param[in]       update: Flag indicates update. Set to `1` to change `UNREAD` messages to `READ` or `0` to leave as is
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_sms_list(lwcell_mem_t mem, lwcell_sms_status_t stat, lwcell_sms_entry_t* entries, size_t etr, size_t* er,
                uint8_t update, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_ASSERT(entries != NULL);
    LWCELL_ASSERT(etr > 0);
    CHECK_ENABLED(); /* Check if enabled */
    CHECK_READY();   /* Check if ready */
    LWCELL_ASSERT(check_sms_mem(mem, 1) == lwcellOK);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);

    if (er != NULL) {
        *er = 0;
    }
    LWCELL_MEMSET(entries, 0x00, sizeof(*entries) * etr); /* Reset data structure */
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CMGL;
    if (mem == LWCELL_MEM_CURRENT) {                       /* Should be always false */
        LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CPMS_GET; /* First get memory */
    } else {
        LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CPMS_SET; /* First set memory */
    }
    LWCELL_MSG_VAR_REF(msg).msg.sms_list.mem = mem;
    LWCELL_MSG_VAR_REF(msg).msg.sms_list.status = stat;
    LWCELL_MSG_VAR_REF(msg).msg.sms_list.entries = entries;
    LWCELL_MSG_VAR_REF(msg).msg.sms_list.etr = etr;
    LWCELL_MSG_VAR_REF(msg).msg.sms_list.er = er;
    LWCELL_MSG_VAR_REF(msg).msg.sms_list.update = update;
    LWCELL_MSG_VAR_REF(msg).msg.sms_list.format = 1; /* Send as plain text */

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 60000);
}

/**
 * \brief           Set preferred storage for SMS
 * \param[in]       mem1: Preferred memory for read/delete SMS operations. Use \ref LWCELL_MEM_CURRENT to keep it as is
 * \param[in]       mem2: Preferred memory for sent/write SMS operations. Use \ref LWCELL_MEM_CURRENT to keep it as is
 * \param[in]       mem3: Preferred memory for received SMS entries. Use \ref LWCELL_MEM_CURRENT to keep it as is
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_sms_set_preferred_storage(lwcell_mem_t mem1, lwcell_mem_t mem2, lwcell_mem_t mem3,
                                 const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    CHECK_ENABLED(); /* Check if enabled */
    CHECK_READY();   /* Check if ready */
    LWCELL_ASSERT(check_sms_mem(mem1, 1) == lwcellOK);
    LWCELL_ASSERT(check_sms_mem(mem2, 1) == lwcellOK);
    LWCELL_ASSERT(check_sms_mem(mem3, 1) == lwcellOK);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CPMS_SET;

    /* In case any of memories is set to current, read current status first from device */
    if (mem1 == LWCELL_MEM_CURRENT || mem2 == LWCELL_MEM_CURRENT || mem3 == LWCELL_MEM_CURRENT) {
        LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CPMS_GET;
    }
    LWCELL_MSG_VAR_REF(msg).msg.sms_memory.mem[0] = mem1;
    LWCELL_MSG_VAR_REF(msg).msg.sms_memory.mem[1] = mem2;
    LWCELL_MSG_VAR_REF(msg).msg.sms_memory.mem[2] = mem3;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 60000);
}

#endif /* LWCELL_CFG_SMS || __DOXYGEN__ */
