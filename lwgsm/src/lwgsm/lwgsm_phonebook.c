/**
 * \file            lwgsm_phonebook.c
 * \brief           Phonebook API
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
#include "lwgsm/lwgsm_phonebook.h"
#include "lwgsm/lwgsm_mem.h"

#if LWGSM_CFG_PHONEBOOK || __DOXYGEN__

#if !__DOXYGEN__
#define CHECK_ENABLED()                 if (!(check_enabled() == lwgsmOK)) { return lwgsmERRNOTENABLED; }
#endif /* !__DOXYGEN__ */

/**
 * \brief           Check if phonebook is enabled
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
static lwgsmr_t
check_enabled(void) {
    lwgsmr_t res;
    lwgsm_core_lock();
    res = lwgsm.m.pb.enabled ? lwgsmOK : lwgsmERR;
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
check_mem(lwgsm_mem_t mem, uint8_t can_curr) {
    lwgsmr_t res = lwgsmERRMEM;
    lwgsm_core_lock();
    if ((mem < LWGSM_MEM_END && lwgsm.m.pb.mem.mem_available & (1 << (uint32_t)mem)) ||
        (can_curr && mem == LWGSM_MEM_CURRENT)) {
        res = lwgsmOK;
    }
    lwgsm_core_unlock();
    return res;
}

/**
 * \brief           Enable phonebook functionality
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_pb_enable(const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_PHONEBOOK_ENABLE;
    LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CPBS_GET_OPT;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

/**
 * \brief           Disable phonebook functionality
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_pb_disable(const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    lwgsm_core_lock();
    lwgsm.m.pb.enabled = 0;
    if (evt_fn != NULL) {
        evt_fn(lwgsmOK, evt_arg);
    }
    lwgsm_core_unlock();
    return lwgsmOK;
}

/**
 * \brief           Add new phonebook entry to desired memory
 * \param[in]       mem: Memory to use to save entry. Use \ref LWGSM_MEM_CURRENT to use current memory
 * \param[in]       name: Entry name
 * \param[in]       num: Entry phone number
 * \param[in]       type: Entry phone number type
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_pb_add(lwgsm_mem_t mem, const char* name, const char* num, lwgsm_number_type_t type,
           const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_ASSERT("name != NULL", name != NULL);
    LWGSM_ASSERT("num != NULL", num != NULL);
    CHECK_ENABLED();                            /* Check if enabled */
    LWGSM_ASSERT("check_mem() == lwgsmOK", check_mem(mem, 1) == lwgsmOK);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_CPBW_SET;
    if (mem == LWGSM_MEM_CURRENT) {             /* Should be always false */
        LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CPBS_GET;/* First get memory */
    } else {
        LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CPBS_SET;/* First set memory */
    }

    LWGSM_MSG_VAR_REF(msg).msg.pb_write.pos = 0;
    LWGSM_MSG_VAR_REF(msg).msg.pb_write.mem = mem;
    LWGSM_MSG_VAR_REF(msg).msg.pb_write.name = name;
    LWGSM_MSG_VAR_REF(msg).msg.pb_write.num = num;
    LWGSM_MSG_VAR_REF(msg).msg.pb_write.type = type;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

/**
 * \brief           Read single phonebook entry
 * \param[in]       mem: Memory to use to save entry. Use \ref LWGSM_MEM_CURRENT to use current memory
 * \param[in]       pos: Entry position in memory to read
 * \param[out]      entry: Pointer to entry variable to save data
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_pb_read(lwgsm_mem_t mem, size_t pos, lwgsm_pb_entry_t* entry,
            const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    return lwgsm_pb_list(mem, pos, entry, 1, NULL, evt_fn, evt_arg, blocking);
}

/**
 * \brief           Edit or overwrite phonebook entry at desired memory and position
 * \param[in]       mem: Memory to use to save entry. Use \ref LWGSM_MEM_CURRENT to use current memory
 * \param[in]       pos: Entry position in memory to edit
 * \param[in]       name: New entry name
 * \param[in]       num: New entry phone number
 * \param[in]       type: New entry phone number type
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_pb_edit(lwgsm_mem_t mem, size_t pos, const char* name, const char* num, lwgsm_number_type_t type,
            const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_ASSERT("name != NULL", name != NULL);
    LWGSM_ASSERT("num != NULL", num != NULL);
    CHECK_ENABLED();                            /* Check if enabled */
    LWGSM_ASSERT("check_mem() == mem", check_mem(mem, 1) == lwgsmOK);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_CPBW_SET;
    if (mem == LWGSM_MEM_CURRENT) {             /* Should be always false */
        LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CPBS_GET;/* First get memory */
    } else {
        LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CPBS_SET;/* First set memory */
    }

    LWGSM_MSG_VAR_REF(msg).msg.pb_write.pos = pos;
    LWGSM_MSG_VAR_REF(msg).msg.pb_write.mem = mem;
    LWGSM_MSG_VAR_REF(msg).msg.pb_write.name = name;
    LWGSM_MSG_VAR_REF(msg).msg.pb_write.num = num;
    LWGSM_MSG_VAR_REF(msg).msg.pb_write.type = type;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

/**
 * \brief           Delete phonebook entry at desired memory and position
 * \param[in]       mem: Memory to use to save entry. Use \ref LWGSM_MEM_CURRENT to use current memory
 * \param[in]       pos: Entry position in memory to delete
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_pb_delete(lwgsm_mem_t mem, size_t pos,
              const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_ASSERT("pos > 0", pos > 0);
    CHECK_ENABLED();                            /* Check if enabled */
    LWGSM_ASSERT("check_mem() == lwgsmOK", check_mem(mem, 1) == lwgsmOK);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_CPBW_SET;
    if (mem == LWGSM_MEM_CURRENT) {             /* Should be always false */
        LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CPBS_GET;/* First get memory */
    } else {
        LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CPBS_SET;/* First set memory */
    }

    LWGSM_MSG_VAR_REF(msg).msg.pb_write.mem = mem;
    LWGSM_MSG_VAR_REF(msg).msg.pb_write.pos = pos;
    LWGSM_MSG_VAR_REF(msg).msg.pb_write.del = 1;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

/**
 * \brief           List entires from specific memory
 * \param[in]       mem: Memory to use to save entry. Use \ref LWGSM_MEM_CURRENT to use current memory
 * \param[in]       start_index: Start position in memory to list
 * \param[out]      entries: Pointer to array to save entries
 * \param[in]       etr: Number of entries to read
 * \param[out]      er: Pointer to output variable to save entries listed
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_pb_list(lwgsm_mem_t mem, size_t start_index, lwgsm_pb_entry_t* entries, size_t etr, size_t* er,
            const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_ASSERT("start_index", start_index);
    LWGSM_ASSERT("entries != NULL", entries != NULL);
    LWGSM_ASSERT("etr > 0", etr > 0);
    CHECK_ENABLED();
    LWGSM_ASSERT("check_mem() == lwgsmOK", check_mem(mem, 1) == lwgsmOK);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);

    if (er != NULL) {
        *er = 0;
    }
    LWGSM_MEMSET(entries, 0x00, sizeof(*entries) * etr);/* Reset data structure */
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_CPBR;
    if (mem == LWGSM_MEM_CURRENT) {             /* Should be always false */
        LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CPBS_GET;/* First get memory */
    } else {
        LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CPBS_SET;/* First set memory */
    }

    LWGSM_MSG_VAR_REF(msg).msg.pb_list.mem = mem;
    LWGSM_MSG_VAR_REF(msg).msg.pb_list.start_index = start_index;
    LWGSM_MSG_VAR_REF(msg).msg.pb_list.entries = entries;
    LWGSM_MSG_VAR_REF(msg).msg.pb_list.etr = etr;
    LWGSM_MSG_VAR_REF(msg).msg.pb_list.er = er;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

/**
 * \brief           Search for entires with specific name from specific memory
 * \note            Search works by entry name only. Phone number search is not available
 * \param[in]       mem: Memory to use to save entry. Use \ref LWGSM_MEM_CURRENT to use current memory
 * \param[in]       search: String to search for
 * \param[out]      entries: Pointer to array to save entries
 * \param[in]       etr: Number of entries to read
 * \param[out]      er: Pointer to output variable to save entries found
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_pb_search(lwgsm_mem_t mem, const char* search, lwgsm_pb_entry_t* entries, size_t etr, size_t* er,
              const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_ASSERT("search != NULL", search != NULL);
    LWGSM_ASSERT("entries != NULL", entries != NULL);
    LWGSM_ASSERT("etr > 0", etr > 0);
    CHECK_ENABLED();                            /* Check if enabled */
    LWGSM_ASSERT("check_mem() == mem", check_mem(mem, 1) == lwgsmOK);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);

    if (er != NULL) {
        *er = 0;
    }
    LWGSM_MEMSET(entries, 0x00, sizeof(*entries) * etr);/* Reset data structure */
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_CPBF;
    if (mem == LWGSM_MEM_CURRENT) {             /* Should be always false */
        LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CPBS_GET;/* First get memory */
    } else {
        LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CPBS_SET;/* First set memory */
    }

    LWGSM_MSG_VAR_REF(msg).msg.pb_search.mem = mem;
    LWGSM_MSG_VAR_REF(msg).msg.pb_search.search = search;
    LWGSM_MSG_VAR_REF(msg).msg.pb_search.entries = entries;
    LWGSM_MSG_VAR_REF(msg).msg.pb_search.etr = etr;
    LWGSM_MSG_VAR_REF(msg).msg.pb_search.er = er;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

#endif /* LWGSM_CFG_PHONEBOOK || __DOXYGEN__ */
