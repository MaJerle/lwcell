/**
 * \file            gsm.c
 * \brief           Main GSM core file
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
#include "gsm/gsm.h"
#include "gsm/gsm_mem.h"
#include "gsm/gsm_threads.h"
#include "system/gsm_ll.h"

#if GSM_CFG_OS != 1
#error GSM_CFG_OS must be set to 1!
#endif

static gsmr_t   def_callback(gsm_cb_t* cb);
static gsm_cb_func_t def_cb_link;

gsm_t gsm;

/**
 * \brief           Default callback function for events
 * \param[in]       cb: Pointer to callback data structure
 * \return          Member of \ref gsmr_t enumeration
 */
static gsmr_t
def_callback(gsm_cb_t* cb) {
    return gsmOK;
}

/**
 * \brief           Init and prepare GSM stack
 * \param[in]       cb_func: Event callback function
 * \return          Member of \ref gsmr_t enumeration
 */
gsmr_t
gsm_init(gsm_cb_fn cb_func) {
    gsm.status.f.initialized = 0;               /* Clear possible init flag */
    
    def_cb_link.fn = cb_func ? cb_func : def_callback;
    gsm.cb_func = &def_cb_link;                 /* Set callback function */
    
    gsm_sys_init();                             /* Init low-level system */
    gsm_ll_init(&gsm.ll, GSM_CFG_AT_PORT_BAUDRATE); /* Init low-level communication */
    
    gsm_sys_sem_create(&gsm.sem_sync, 1);       /* Create new semaphore with unlocked state */
    gsm_sys_mbox_create(&gsm.mbox_producer, GSM_CFG_THREAD_PRODUCER_MBOX_SIZE); /* Producer message queue */
    gsm_sys_thread_create(&gsm.thread_producer, "producer", gsm_thread_producer, &gsm, GSM_SYS_THREAD_SS, GSM_SYS_THREAD_PRIO);
    
    gsm_sys_mbox_create(&gsm.mbox_process, GSM_CFG_THREAD_PROCESS_MBOX_SIZE);   /* Consumer message queue */
    gsm_sys_thread_create(&gsm.thread_process,  "process", gsm_thread_process, &gsm, GSM_SYS_THREAD_SS, GSM_SYS_THREAD_PRIO);

#if !GSM_CFG_INPUT_USE_PROCESS
    gsm_buff_init(&gsm.buff, GSM_CFG_RCV_BUFF_SIZE);    /* Init buffer for input data */
#endif /* !GSM_CFG_INPUT_USE_PROCESS */
    gsm.status.f.initialized = 1;               /* We are initialized now */
    
    /*
     * Call reset command and call default
     * AT commands to prepare basic setup for device
     */
    gsm_reset(1);
    gsmi_send_cb(GSM_CB_INIT_FINISH);           /* Call user callback function */
    
    return gsmOK;
}

/**
 * \brief           Execute reset and send default commands
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_reset(uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);                    /* Define variable for message */

    GSM_MSG_VAR_ALLOC(msg);                     /* Allocate memory for variable */
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_RESET;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, blocking, 60000);   /* Send message to producer queue */
}

/**
 * \brief           Lock and protect GSM core from multiple access at a time
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_core_lock(void) {
    GSM_CORE_PROTECT();                         /* Lock GSM core */
    return gsmOK;
}

/**
 * \brief           Unlock and unprotect GSM core from multiple access at a time
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_core_unlock(void) {
    GSM_CORE_UNPROTECT();                       /* Unlock GSM core */
    return gsmOK;
}

/**
 * \brief           Register callback function for global (non-connection based) events
 * \param[in]       cb_fn: Callback function to call on specific event
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_cb_register(gsm_cb_fn cb_fn) {
    gsmr_t res = gsmOK;
    gsm_cb_func_t* func, *newFunc;
    
    GSM_ASSERT("cb_fn != NULL", cb_fn != NULL); /* Assert input parameters */
    
    GSM_CORE_PROTECT();                         /* Lock GSM core */
    
    /* Check if function already exists on list */
    for (func = gsm.cb_func; func != NULL; func = func->next) {
        if (func->fn == cb_fn) {
            res = gsmERR;
            break;
        }
    }
    
    if (res == gsmOK) {
        newFunc = gsm_mem_alloc(sizeof(*newFunc));  /* Get memory for new function */
        if (newFunc != NULL) {
            memset(newFunc, 0x00, sizeof(*newFunc));/* Reset memory */
            newFunc->fn = cb_fn;                /* Set function pointer */
            if (gsm.cb_func == NULL) {
                gsm.cb_func = newFunc;          /* This should never happen! */
            } else {
                for (func = gsm.cb_func; func->next != NULL; func = func->next) {}
                func->next = newFunc;           /* Set new function as next */
                res = gsmOK;
            }
            res = gsmOK;
        } else {
            res = gsmERRMEM;
        }
    }
    GSM_CORE_UNPROTECT();                       /* Unlock GSM core */
    return res;
}

/**
 * \brief           Unregister callback function for global (non-connection based) events
 * \note            Function must be first registered using \ref gsm_cb_register
 * \param[in]       cb_fn: Callback function to call on specific event
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_cb_unregister(gsm_cb_fn cb_fn) {
    gsm_cb_func_t* func, *prev;
    GSM_ASSERT("cb_fn != NULL", cb_fn != NULL); /* Assert input parameters */
    
    GSM_CORE_PROTECT();                         /* Lock GSM core */
    for (prev = gsm.cb_func, func = gsm.cb_func->next; func != NULL; prev = func, func = func->next) {
        if (func->fn == cb_fn) {
            prev->next = func->next;
            gsm_mem_free(func);
            func = NULL;
            break;
        }
    }
    GSM_CORE_UNPROTECT();                       /* Unlock GSM core */
    return gsmOK;
}

/**
 * \brief           Delay for amount of milliseconds
 * \param[in]       ms: Milliseconds to delay
 */
void
gsm_delay(uint32_t ms) {
    gsm_sys_sem_t sem;
    if (ms != 0) {
        gsm_sys_sem_create(&sem, 0);            /* Create semaphore in locked state */
        gsm_sys_sem_wait(&sem, ms);             /* Wait for semaphore, timeout should occur */
        gsm_sys_sem_release(&sem);              /* Release semaphore */
        gsm_sys_sem_delete(&sem);               /* Delete semaphore */
    }
}

/**
 * \brief           Set modem function mode
 * \note            Use this function to set modem to normal or low-power mode
 * \param[in]       mode: Mode status. Set to 1 for full functionality or 0 for low-power mode (no functionality)
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_set_func_mode(uint8_t mode, uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);                    /* Define variable for message */

    GSM_MSG_VAR_ALLOC(msg);                     /* Allocate memory for variable */
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_CFUN_SET;
    GSM_MSG_VAR_REF(msg).msg.cfun.mode = mode;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, blocking, 60000);   /* Send message to producer queue */
}

/**
 * \brief           Set pin code to make card ready
 * \param[in]       pin: Pin code in string format
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_set_pin(const char* pin, uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);                    /* Define variable for message */

    GSM_MSG_VAR_ALLOC(msg);                     /* Allocate memory for variable */
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_CPIN_SET;
    GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CPIN_GET;
    GSM_MSG_VAR_REF(msg).msg.cpin.pin = pin;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, blocking, 10000);   /* Send message to producer queue */
}

/**
* \brief           Sets baudrate of AT port (usually UART)
* \param[in]       baud: Baudrate in units of bits per second
* \param[in]       blocking: Status whether command should be blocking or not
* \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
*/
gsmr_t
gsm_set_at_baudrate(uint32_t baud, uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);                    /* Define variable for message */

    GSM_MSG_VAR_ALLOC(msg);                     /* Allocate memory for variable */
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_UART;
    GSM_MSG_VAR_REF(msg).msg.uart.baudrate = baud;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, blocking, 2000);    /* Send message to producer queue */
}

/**
 * \brief           Get current operator name
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_operator_get(uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);                    /* Define variable for message */

    GSM_MSG_VAR_ALLOC(msg);                     /* Allocate memory for variable */
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_COPS_GET;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, blocking, 2000);    /* Send message to producer queue */
}

/**
 * \brief           Get current operator name
 * \param[in]       ops: Pointer to array to write found operators
 * \param[in]       opsl: Length of input array in units of elements
 * \param[out]      opf: Pointer to ouput variable to save number of operators found
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_operator_scan(gsm_operator_t* ops, size_t opsl, size_t* opf, uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);                    /* Define variable for message */

    GSM_MSG_VAR_ALLOC(msg);                     /* Allocate memory for variable */
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_COPS_GET_OPT;
    GSM_MSG_VAR_REF(msg).msg.cops_scan.ops = ops;
    GSM_MSG_VAR_REF(msg).msg.cops_scan.opsl = opsl;
    GSM_MSG_VAR_REF(msg).msg.cops_scan.opf = opf;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, blocking, 120000);  /* Send message to producer queue */
}
