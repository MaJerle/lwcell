/**	
 * \file            gsm_phonebook.c
 * \brief           Phonebook API
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
 * This file is part of GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 */
#include "gsm/gsm_private.h"
#include "gsm/gsm_phonebook.h"
#include "gsm/gsm_mem.h"

#if GSM_CFG_PHONEBOOK || __DOXYGEN__

#if !__DOXYGEN__
#define CHECK_ENABLED()                 if (!(check_enabled() == gsmOK)) { return gsmERRNOTENABLED; }
#endif /* !__DOXYGEN__ */

/**
 * \brief           Check if phonebook is enabled
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
static gsmr_t
check_enabled(void) {
    gsmr_t res;
    GSM_CORE_PROTECT();                         /* Protect core */
    res = gsm.pb.enabled ? gsmOK : gsmERR;
    GSM_CORE_UNPROTECT();                       /* Unprotect core */
    return res;
}

/**
 * \brief           Check if input memory is available in modem
 * \param[in]       mem: Memory to test
 * \param[in]       can_curr: Flag indicates if \ref GSM_MEM_CURRENT option can be used
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
static gsmr_t
check_mem(gsm_mem_t mem, uint8_t can_curr) {
    gsmr_t res = gsmERRMEM;
    GSM_CORE_PROTECT();                         /* Protect core */
    if ((mem < GSM_MEM_END && gsm.pb.mem.mem_available & (1 << (uint32_t)mem)) ||
        (can_curr && mem == GSM_MEM_CURRENT)) {
        res = gsmOK;
    }
    GSM_CORE_UNPROTECT();                       /* Unprotect core */
    return res;
}

/**
 * \brief           Enable phonebook functionality
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
gsmr_t
gsm_pb_enable(uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);                    /* Define variable for message */

    GSM_MSG_VAR_ALLOC(msg);                     /* Allocate memory for variable */
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_PHONEBOOK_ENABLE;
    GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CPBS_GET_OPT;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, blocking, 60000);   /* Send message to producer queue */
}

/**
 * \brief           Disable phonebook functionality
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
gsmr_t
gsm_pb_disable(uint32_t blocking) {
    GSM_CORE_PROTECT();                         /* Protect core */
    gsm.pb.enabled = 0;                         /* Clear enabled status */
    GSM_CORE_UNPROTECT();                       /* Unprotect core */
    return gsmOK;
}

/**
 * \brief           Add new phonebook entry to desired memory
 * \param[in]       mem: Memory to use to save entry. Use \ref GSM_MEM_CURRENT to use current memory
 * \param[in]       name: Entry name
 * \param[in]       num: Entry phone number
 * \param[in]       type: Entry phone number type
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
gsmr_t
gsm_pb_add(gsm_mem_t mem, const char* name, const char* num, gsm_number_type_t type, uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);                    /* Define variable for message */

    GSM_ASSERT("name != NULL", name != NULL);   /* Assert input parameters */
    GSM_ASSERT("num != NULL", num != NULL);     /* Assert input parameters */
    CHECK_ENABLED();                            /* Check if enabled */
    GSM_ASSERT("mem", check_mem(mem, 1) == gsmOK);  /* Assert input parameters */

    GSM_MSG_VAR_ALLOC(msg);                     /* Allocate memory for variable */
    
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_CPBW_SET;
    if (mem == GSM_MEM_CURRENT) {               /* Should be always false */
        GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CPBS_GET;/* First get memory */
    } else {
        GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CPBS_SET;/* First set memory */
    }

    GSM_MSG_VAR_REF(msg).msg.pb_write.pos = 0;
    GSM_MSG_VAR_REF(msg).msg.pb_write.mem = mem;
    GSM_MSG_VAR_REF(msg).msg.pb_write.name = name;
    GSM_MSG_VAR_REF(msg).msg.pb_write.num = num;
    GSM_MSG_VAR_REF(msg).msg.pb_write.type = type;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, blocking, 60000);   /* Send message to producer queue */
}

/**
 * \brief           Read single phonebook entry
 * \param[in]       mem: Memory to use to save entry. Use \ref GSM_MEM_CURRENT to use current memory
 * \param[in]       pos: Entry position in memory to read
 * \param[out]      entry: Pointer to entry variable to save data
 * \param[in]       blocking: Status whether command should be blocking or not
 * \param[in]       blocking: Status whether command should be blocking or not
 */
gsmr_t
gsm_pb_read(gsm_mem_t mem, size_t pos, gsm_pb_entry_t* entry, uint32_t blocking) {
    return gsm_pb_list(mem, pos, entry, 1, NULL, blocking);
}

/**
 * \brief           Edit or overwrite phonebook entry at desired memory and position
 * \param[in]       mem: Memory to use to save entry. Use \ref GSM_MEM_CURRENT to use current memory
 * \param[in]       pos: Entry position in memory to edit
 * \param[in]       name: New entry name
 * \param[in]       num: New entry phone number
 * \param[in]       type: New entry phone number type
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
gsmr_t
gsm_pb_edit(gsm_mem_t mem, size_t pos, const char* name, const char* num, gsm_number_type_t type, uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);                    /* Define variable for message */

    GSM_ASSERT("name != NULL", name != NULL);   /* Assert input parameters */
    GSM_ASSERT("num != NULL", num != NULL);     /* Assert input parameters */
    CHECK_ENABLED();                            /* Check if enabled */
    GSM_ASSERT("mem", check_mem(mem, 1) == gsmOK);  /* Assert input parameters */

    GSM_MSG_VAR_ALLOC(msg);                     /* Allocate memory for variable */

    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_CPBW_SET;
    if (mem == GSM_MEM_CURRENT) {               /* Should be always false */
        GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CPBS_GET;/* First get memory */
    } else {
        GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CPBS_SET;/* First set memory */
    }

    GSM_MSG_VAR_REF(msg).msg.pb_write.pos = pos;
    GSM_MSG_VAR_REF(msg).msg.pb_write.mem = mem;
    GSM_MSG_VAR_REF(msg).msg.pb_write.name = name;
    GSM_MSG_VAR_REF(msg).msg.pb_write.num = num;
    GSM_MSG_VAR_REF(msg).msg.pb_write.type = type;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, blocking, 60000);   /* Send message to producer queue */
}

/**
 * \brief           Delete phonebook entry at desired memory and position
 * \param[in]       mem: Memory to use to save entry. Use \ref GSM_MEM_CURRENT to use current memory
 * \param[in]       pos: Entry position in memory to delete
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
gsmr_t
gsm_pb_delete(gsm_mem_t mem, size_t pos, uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);                    /* Define variable for message */

    GSM_ASSERT("pos > 0", pos > 0);             /* Assert input parameters */
    CHECK_ENABLED();                            /* Check if enabled */
    GSM_ASSERT("mem", check_mem(mem, 1) == gsmOK);  /* Assert input parameters */

    GSM_MSG_VAR_ALLOC(msg);                     /* Allocate memory for variable */

    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_CPBW_SET;
    if (mem == GSM_MEM_CURRENT) {               /* Should be always false */
        GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CPBS_GET;    /* First get memory */
    } else {
        GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CPBS_SET;    /* First set memory */
    }

    GSM_MSG_VAR_REF(msg).msg.pb_write.mem = mem;
    GSM_MSG_VAR_REF(msg).msg.pb_write.pos = pos;
    GSM_MSG_VAR_REF(msg).msg.pb_write.del = 1;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, blocking, 60000);   /* Send message to producer queue */
}

/**
 * \brief           List entires from specific memory
 * \param[in]       mem: Memory to use to save entry. Use \ref GSM_MEM_CURRENT to use current memory
 * \param[in]       start_index: Start position in memory to list
 * \param[out]      entries: Pointer to array to save entries
 * \param[in]       etr: Number of entries to read
 * \param[out]      er: Pointer to output variable to save entries listed
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
gsmr_t
gsm_pb_list(gsm_mem_t mem, size_t start_index, gsm_pb_entry_t* entries, size_t etr, size_t* er, uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);                    /* Define variable for message */

    GSM_ASSERT("start_index > 0", start_index > 0); /* Assert input parameters */
    GSM_ASSERT("entries != NULL", entries != NULL); /* Assert input parameters */
    GSM_ASSERT("etr > 0", etr > 0);             /* Assert input parameters */
    CHECK_ENABLED();                            /* Check if enabled */
    GSM_ASSERT("mem", check_mem(mem, 1) == gsmOK);  /* Assert input parameters */

    GSM_MSG_VAR_ALLOC(msg);                     /* Allocate memory for variable */

    if (er != NULL) {
        *er = 0;
    }
    memset(entries, 0x00, sizeof(*entries) * etr);  /* Reset data structure */
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_CPBR;
    if (mem == GSM_MEM_CURRENT) {               /* Should be always false */
        GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CPBS_GET;    /* First get memory */
    } else {
        GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CPBS_SET;    /* First set memory */
    }

    GSM_MSG_VAR_REF(msg).msg.pb_list.mem = mem;
    GSM_MSG_VAR_REF(msg).msg.pb_list.start_index = start_index;
    GSM_MSG_VAR_REF(msg).msg.pb_list.entries = entries;
    GSM_MSG_VAR_REF(msg).msg.pb_list.etr = etr;
    GSM_MSG_VAR_REF(msg).msg.pb_list.er = er;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, blocking, 60000);   /* Send message to producer queue */
}

/**
 * \brief           Search for entires with specific name from specific memory
 * \note            Search works by entry name only. Phone number search is not available
 * \param[in]       mem: Memory to use to save entry. Use \ref GSM_MEM_CURRENT to use current memory
 * \param[in]       search: String to search for
 * \param[out]      entries: Pointer to array to save entries
 * \param[in]       etr: Number of entries to read
 * \param[out]      er: Pointer to output variable to save entries found
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
gsmr_t
gsm_pb_search(gsm_mem_t mem, const char* search, gsm_pb_entry_t* entries, size_t etr, size_t* er, uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);                    /* Define variable for message */

    GSM_ASSERT("search != NULL", search != NULL);   /* Assert input parameters */
    GSM_ASSERT("entries != NULL", entries != NULL); /* Assert input parameters */
    GSM_ASSERT("etr > 0", etr > 0);             /* Assert input parameters */
    CHECK_ENABLED();                            /* Check if enabled */
    GSM_ASSERT("mem", check_mem(mem, 1) == gsmOK);  /* Assert input parameters */

    GSM_MSG_VAR_ALLOC(msg);                     /* Allocate memory for variable */

    if (er != NULL) {
        *er = 0;
    }
    memset(entries, 0x00, sizeof(*entries) * etr);  /* Reset data structure */
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_CPBF;
    if (mem == GSM_MEM_CURRENT) {               /* Should be always false */
        GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CPBS_GET;    /* First get memory */
    } else {
        GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CPBS_SET;    /* First set memory */
    }

    GSM_MSG_VAR_REF(msg).msg.pb_search.mem = mem;
    GSM_MSG_VAR_REF(msg).msg.pb_search.search = search;
    GSM_MSG_VAR_REF(msg).msg.pb_search.entries = entries;
    GSM_MSG_VAR_REF(msg).msg.pb_search.etr = etr;
    GSM_MSG_VAR_REF(msg).msg.pb_search.er = er;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, blocking, 60000);   /* Send message to producer queue */
}

#endif /* GSM_CFG_PHONEBOOK || __DOXYGEN__ */
