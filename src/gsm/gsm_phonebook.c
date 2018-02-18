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
 * This file is part of GSM-AT.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 */
#include "gsm/gsm_private.h"
#include "gsm/gsm_phonebook.h"
#include "gsm/gsm_mem.h"

#if GSM_CFG_PHONEBOOK || __DOXYGEN__

/**
 * \brief           Check if input memory is available in modem
 * \param[in]       mem: Memory to test
 * \param[in]       can_curr: Flag indicates if \ref GSM_MEM_CURRENT option can be used
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
static gsmr_t
check_mem(gsm_mem_t mem, uint8_t can_curr) {
    gsmr_t res = gsmERRMEM;
    GSM_CORE_PROTECT();                     /* Protect core */
    if ((mem < GSM_MEM_END && gsm.pb.mem.mem_available & (1 << (uint32_t)mem)) ||
        (can_curr && mem == GSM_MEM_CURRENT)) {
        res = gsmOK;
    }
    GSM_CORE_UNPROTECT();                   /* Unprotect core */
    return res;
}

gsmr_t
gsm_pb_add(gsm_mem_t mem, const char* name, const char* num, gsm_number_type_t type, uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);                    /* Define variable for message */

    GSM_ASSERT("name != NULL", name != NULL);   /* Assert input parameters */
    GSM_ASSERT("num != NULL", num != NULL);     /* Assert input parameters */
    GSM_ASSERT("mem", check_mem(mem, 1) == gsmOK);  /* Assert input parameters */

    GSM_MSG_VAR_ALLOC(msg);                     /* Allocate memory for variable */
    
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_CPBW_SET;
    if (mem == GSM_MEM_CURRENT) {               /* Should be always false */
        GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CPBS_GET;    /* First get memory */
    } else {
        GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CPBS_SET;    /* First set memory */
    }

    GSM_MSG_VAR_REF(msg).msg.pb_write.pos = 0;
    GSM_MSG_VAR_REF(msg).msg.pb_write.mem = mem;
    GSM_MSG_VAR_REF(msg).msg.pb_write.name = name;
    GSM_MSG_VAR_REF(msg).msg.pb_write.num = num;
    GSM_MSG_VAR_REF(msg).msg.pb_write.type = type;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, blocking, 60000);   /* Send message to producer queue */
}

gsmr_t
gsm_pb_edit(gsm_mem_t mem, size_t pos, const char* name, const char* num, gsm_number_type_t type, uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);                    /* Define variable for message */

    GSM_ASSERT("name != NULL", name != NULL);   /* Assert input parameters */
    GSM_ASSERT("num != NULL", num != NULL);     /* Assert input parameters */
    GSM_ASSERT("mem", check_mem(mem, 1) == gsmOK);  /* Assert input parameters */

    GSM_MSG_VAR_ALLOC(msg);                     /* Allocate memory for variable */

    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_CPBW_SET;
    if (mem == GSM_MEM_CURRENT) {               /* Should be always false */
        GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CPBS_GET;    /* First get memory */
    }
    else {
        GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CPBS_SET;    /* First set memory */
    }

    GSM_MSG_VAR_REF(msg).msg.pb_write.pos = pos;
    GSM_MSG_VAR_REF(msg).msg.pb_write.mem = mem;
    GSM_MSG_VAR_REF(msg).msg.pb_write.name = name;
    GSM_MSG_VAR_REF(msg).msg.pb_write.num = num;
    GSM_MSG_VAR_REF(msg).msg.pb_write.type = type;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, blocking, 60000);   /* Send message to producer queue */
}

gsmr_t
gsm_pb_delete(gsm_mem_t mem, size_t pos, uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);                    /* Define variable for message */

    GSM_ASSERT("mem", check_mem(mem, 1) == gsmOK);  /* Assert input parameters */
    GSM_ASSERT("pos > 0", pos > 0);             /* Assert input parameters */

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

gsmr_t
gsm_pb_list(gsm_mem_t mem, size_t start_index, gsm_pb_entry_t* entries, size_t etr, size_t* er, uint32_t blocking);

gsmr_t
gsm_pb_search(gsm_mem_t mem, const char* search, gsm_pb_entry_t* entries, size_t etr, size_t* er, uint32_t blocking);

#endif /* GSM_CFG_PHONEBOOK || __DOXYGEN__ */
