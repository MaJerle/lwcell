/**
 * \file            lwcell_phonebook.c
 * \brief           Phonebook API
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
 * This file is part of LwCELL - Lightweight GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.1
 */
#include "lwcell/lwcell_phonebook.h"
#include "lwcell/lwcell_private.h"

#if LWCELL_CFG_PHONEBOOK || __DOXYGEN__

#if !__DOXYGEN__
#define CHECK_ENABLED()                                                                                                \
    if (!(check_enabled() == lwcellOK)) {                                                                               \
        return lwcellERRNOTENABLED;                                                                                     \
    }
#endif /* !__DOXYGEN__ */

/**
 * \brief           Check if phonebook is enabled
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
static lwcellr_t
check_enabled(void) {
    lwcellr_t res;
    lwcell_core_lock();
    res = lwcell.m.pb.enabled ? lwcellOK : lwcellERR;
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
check_mem(lwcell_mem_t mem, uint8_t can_curr) {
    lwcellr_t res = lwcellERRMEM;
    lwcell_core_lock();
    if ((mem < LWCELL_MEM_END && lwcell.m.pb.mem.mem_available & (1 << (uint32_t)mem))
        || (can_curr && mem == LWCELL_MEM_CURRENT)) {
        res = lwcellOK;
    }
    lwcell_core_unlock();
    return res;
}

/**
 * \brief           Enable phonebook functionality
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_pb_enable(const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_PHONEBOOK_ENABLE;
    LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CPBS_GET_OPT;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 60000);
}

/**
 * \brief           Disable phonebook functionality
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_pb_disable(const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    lwcell_core_lock();
    lwcell.m.pb.enabled = 0;
    if (evt_fn != NULL) {
        evt_fn(lwcellOK, evt_arg);
    }
    lwcell_core_unlock();
    LWCELL_UNUSED(blocking);
    return lwcellOK;
}

/**
 * \brief           Add new phonebook entry to desired memory
 * \param[in]       mem: Memory to use to save entry. Use \ref LWCELL_MEM_CURRENT to use current memory
 * \param[in]       name: Entry name
 * \param[in]       num: Entry phone number
 * \param[in]       type: Entry phone number type
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_pb_add(lwcell_mem_t mem, const char* name, const char* num, lwcell_number_type_t type,
             const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_ASSERT(name != NULL);
    LWCELL_ASSERT(num != NULL);
    CHECK_ENABLED(); /* Check if enabled */
    LWCELL_ASSERT(check_mem(mem, 1) == lwcellOK);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CPBW_SET;
    if (mem == LWCELL_MEM_CURRENT) {                      /* Should be always false */
        LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CPBS_GET; /* First get memory */
    } else {
        LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CPBS_SET; /* First set memory */
    }

    LWCELL_MSG_VAR_REF(msg).msg.pb_write.pos = 0;
    LWCELL_MSG_VAR_REF(msg).msg.pb_write.mem = mem;
    LWCELL_MSG_VAR_REF(msg).msg.pb_write.name = name;
    LWCELL_MSG_VAR_REF(msg).msg.pb_write.num = num;
    LWCELL_MSG_VAR_REF(msg).msg.pb_write.type = type;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 60000);
}

/**
 * \brief           Read single phonebook entry
 * \param[in]       mem: Memory to use to save entry. Use \ref LWCELL_MEM_CURRENT to use current memory
 * \param[in]       pos: Entry position in memory to read
 * \param[out]      entry: Pointer to entry variable to save data
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_pb_read(lwcell_mem_t mem, size_t pos, lwcell_pb_entry_t* entry, const lwcell_api_cmd_evt_fn evt_fn,
              void* const evt_arg, const uint32_t blocking) {
    return lwcell_pb_list(mem, pos, entry, 1, NULL, evt_fn, evt_arg, blocking);
}

/**
 * \brief           Edit or overwrite phonebook entry at desired memory and position
 * \param[in]       mem: Memory to use to save entry. Use \ref LWCELL_MEM_CURRENT to use current memory
 * \param[in]       pos: Entry position in memory to edit
 * \param[in]       name: New entry name
 * \param[in]       num: New entry phone number
 * \param[in]       type: New entry phone number type
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_pb_edit(lwcell_mem_t mem, size_t pos, const char* name, const char* num, lwcell_number_type_t type,
              const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_ASSERT(name != NULL);
    LWCELL_ASSERT(num != NULL);
    CHECK_ENABLED(); /* Check if enabled */
    LWCELL_ASSERT(check_mem(mem, 1) == lwcellOK);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CPBW_SET;
    if (mem == LWCELL_MEM_CURRENT) {                      /* Should be always false */
        LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CPBS_GET; /* First get memory */
    } else {
        LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CPBS_SET; /* First set memory */
    }

    LWCELL_MSG_VAR_REF(msg).msg.pb_write.pos = pos;
    LWCELL_MSG_VAR_REF(msg).msg.pb_write.mem = mem;
    LWCELL_MSG_VAR_REF(msg).msg.pb_write.name = name;
    LWCELL_MSG_VAR_REF(msg).msg.pb_write.num = num;
    LWCELL_MSG_VAR_REF(msg).msg.pb_write.type = type;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 60000);
}

/**
 * \brief           Delete phonebook entry at desired memory and position
 * \param[in]       mem: Memory to use to save entry. Use \ref LWCELL_MEM_CURRENT to use current memory
 * \param[in]       pos: Entry position in memory to delete
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_pb_delete(lwcell_mem_t mem, size_t pos, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_ASSERT(pos > 0);
    CHECK_ENABLED(); /* Check if enabled */
    LWCELL_ASSERT(check_mem(mem, 1) == lwcellOK);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CPBW_SET;
    if (mem == LWCELL_MEM_CURRENT) {                      /* Should be always false */
        LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CPBS_GET; /* First get memory */
    } else {
        LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CPBS_SET; /* First set memory */
    }

    LWCELL_MSG_VAR_REF(msg).msg.pb_write.mem = mem;
    LWCELL_MSG_VAR_REF(msg).msg.pb_write.pos = pos;
    LWCELL_MSG_VAR_REF(msg).msg.pb_write.del = 1;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 60000);
}

/**
 * \brief           List entires from specific memory
 * \param[in]       mem: Memory to use to save entry. Use \ref LWCELL_MEM_CURRENT to use current memory
 * \param[in]       start_index: Start position in memory to list
 * \param[out]      entries: Pointer to array to save entries
 * \param[in]       etr: Number of entries to read
 * \param[out]      er: Pointer to output variable to save entries listed
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_pb_list(lwcell_mem_t mem, size_t start_index, lwcell_pb_entry_t* entries, size_t etr, size_t* er,
              const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_ASSERT(start_index);
    LWCELL_ASSERT(entries != NULL);
    LWCELL_ASSERT(etr > 0);
    CHECK_ENABLED();
    LWCELL_ASSERT(check_mem(mem, 1) == lwcellOK);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);

    if (er != NULL) {
        *er = 0;
    }
    LWCELL_MEMSET(entries, 0x00, sizeof(*entries) * etr); /* Reset data structure */
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CPBR;
    if (mem == LWCELL_MEM_CURRENT) {                      /* Should be always false */
        LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CPBS_GET; /* First get memory */
    } else {
        LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CPBS_SET; /* First set memory */
    }

    LWCELL_MSG_VAR_REF(msg).msg.pb_list.mem = mem;
    LWCELL_MSG_VAR_REF(msg).msg.pb_list.start_index = start_index;
    LWCELL_MSG_VAR_REF(msg).msg.pb_list.entries = entries;
    LWCELL_MSG_VAR_REF(msg).msg.pb_list.etr = etr;
    LWCELL_MSG_VAR_REF(msg).msg.pb_list.er = er;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 60000);
}

/**
 * \brief           Search for entires with specific name from specific memory
 * \note            Search works by entry name only. Phone number search is not available
 * \param[in]       mem: Memory to use to save entry. Use \ref LWCELL_MEM_CURRENT to use current memory
 * \param[in]       search: String to search for
 * \param[out]      entries: Pointer to array to save entries
 * \param[in]       etr: Number of entries to read
 * \param[out]      er: Pointer to output variable to save entries found
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_pb_search(lwcell_mem_t mem, const char* search, lwcell_pb_entry_t* entries, size_t etr, size_t* er,
                const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_ASSERT(search != NULL);
    LWCELL_ASSERT(entries != NULL);
    LWCELL_ASSERT(etr > 0);
    CHECK_ENABLED(); /* Check if enabled */
    LWCELL_ASSERT(check_mem(mem, 1) == lwcellOK);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);

    if (er != NULL) {
        *er = 0;
    }
    LWCELL_MEMSET(entries, 0x00, sizeof(*entries) * etr); /* Reset data structure */
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CPBF;
    if (mem == LWCELL_MEM_CURRENT) {                      /* Should be always false */
        LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CPBS_GET; /* First get memory */
    } else {
        LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CPBS_SET; /* First set memory */
    }

    LWCELL_MSG_VAR_REF(msg).msg.pb_search.mem = mem;
    LWCELL_MSG_VAR_REF(msg).msg.pb_search.search = search;
    LWCELL_MSG_VAR_REF(msg).msg.pb_search.entries = entries;
    LWCELL_MSG_VAR_REF(msg).msg.pb_search.etr = etr;
    LWCELL_MSG_VAR_REF(msg).msg.pb_search.er = er;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 60000);
}

#endif /* LWCELL_CFG_PHONEBOOK || __DOXYGEN__ */
