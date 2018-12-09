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
 * This file is part of GSM-AT library.
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

static gsmr_t   def_callback(gsm_evt_t* cb);
static gsm_evt_func_t def_evt_link;

gsm_t gsm;

/**
 * \brief           Default callback function for events
 * \param[in]       cb: Pointer to callback data structure
 * \return          Member of \ref gsmr_t enumeration
 */
static gsmr_t
def_callback(gsm_evt_t* cb) {
    return gsmOK;
}

/**
 * \brief           Init and prepare GSM stack
 * \note            When \ref GSM_CFG_RESET_ON_INIT is enabled, reset sequence will be sent to device.
 *                  In this case, `blocking` parameter indicates if we shall wait or not for response
 * \param[in]       evt_func: Event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          Member of \ref gsmr_t enumeration
 */
gsmr_t
gsm_init(gsm_evt_fn evt_func, uint32_t blocking) {
    gsm.status.f.initialized = 0;               /* Clear possible init flag */
    
    def_evt_link.fn = evt_func != NULL ? evt_func : def_callback;
    gsm.evt_func = &def_evt_link;               /* Set callback function */
    
    gsm_sys_init();                             /* Init low-level system */
    gsm.ll.uart.baudrate = GSM_CFG_AT_PORT_BAUDRATE;
    gsm_ll_init(&gsm.ll);                       /* Init low-level communication */
    
    gsm_sys_sem_create(&gsm.sem_sync, 1);       /* Create new semaphore with unlocked state */
    gsm_sys_mbox_create(&gsm.mbox_producer, GSM_CFG_THREAD_PRODUCER_MBOX_SIZE); /* Producer message queue */
    gsm_sys_thread_create(&gsm.thread_producer, "gsm_producer", (gsm_sys_thread_fn)gsm_thread_producer, &gsm, GSM_SYS_THREAD_SS, GSM_SYS_THREAD_PRIO);

    gsm_sys_mbox_create(&gsm.mbox_process, GSM_CFG_THREAD_PROCESS_MBOX_SIZE);   /* Consumer message queue */
    gsm_sys_thread_create(&gsm.thread_process,  "gsm_process", (gsm_sys_thread_fn)gsm_thread_process, &gsm, GSM_SYS_THREAD_SS, GSM_SYS_THREAD_PRIO);

#if !GSM_CFG_INPUT_USE_PROCESS
    gsm_buff_init(&gsm.buff, GSM_CFG_RCV_BUFF_SIZE);    /* Init buffer for input data */
#endif /* !GSM_CFG_INPUT_USE_PROCESS */
    gsm.status.f.initialized = 1;               /* We are initialized now */
    gsm.status.f.dev_present = 1;               /* We assume device is present at this point */
    
    /*
     * Call reset command and call default
     * AT commands to prepare basic setup for device
     */
#if GSM_CFG_RESET_ON_INIT
    if (gsm.status.f.dev_present) {             /* In case device exists */
        gsm_reset_with_delay(GSM_CFG_RESET_DELAY_DEFAULT, blocking);    /* Send reset sequence with delay */
    }
#else
    GSM_UNUSED(blocking);                       /* Unused variable */
#endif /* GSM_CFG_RESET_ON_INIT */
    gsmi_send_cb(GSM_EVT_INIT_FINISH);          /* Call user callback function */
    
    return gsmOK;
}

/**
 * \brief           Execute reset and send default commands
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_reset(uint32_t blocking) {
    return gsm_reset_with_delay(0, blocking);
}

/**
 * \brief           Execute reset and send default commands with delay
 * \param[in]       delay: Number of milliseconds to wait before initiating first command to device
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_reset_with_delay(uint32_t delay, uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);                    /* Define variable for message */

    GSM_MSG_VAR_ALLOC(msg);                     /* Allocate memory for variable */
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_RESET;
    GSM_MSG_VAR_REF(msg).msg.reset.delay = delay;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, blocking, 60000);   /* Send message to producer queue */
}

/**
 * \brief           Increase protection counter
 *
 *                  If lock was `0` before func call, lock is enabled and increased
 * \note            Function may be called multiple times to increase locks. 
 *                  User must take care of calling \ref gsm_core_unlock function
 *                  for the same amount to decrease lock back to `0`
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_core_lock(void) {
    GSM_CORE_PROTECT();                         /* Lock GSM core */
    return gsmOK;
}

/**
 * \brief           Decrease protection counter
 *
 *                  If lock was non-zero before func call, it is decreased.
 *                  When `lock == 0`, protection is disabled
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_core_unlock(void) {
    GSM_CORE_UNPROTECT();                       /* Unlock GSM core */
    return gsmOK;
}

/**
 * \brief           Register callback function for global (non-connection based) events
 * \param[in]       fn: Callback function to call on specific event
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_evt_register(gsm_evt_fn fn) {
    gsmr_t res = gsmOK;
    gsm_evt_func_t* func, *newFunc;
    
    GSM_ASSERT("cb_fn != NULL", fn != NULL);    /* Assert input parameters */
    
    GSM_CORE_PROTECT();                         /* Lock GSM core */
    
    /* Check if function already exists on list */
    for (func = gsm.evt_func; func != NULL; func = func->next) {
        if (func->fn == fn) {
            res = gsmERR;
            break;
        }
    }
    
    if (res == gsmOK) {
        newFunc = gsm_mem_alloc(sizeof(*newFunc));  /* Get memory for new function */
        if (newFunc != NULL) {
            memset(newFunc, 0x00, sizeof(*newFunc));/* Reset memory */
            newFunc->fn = fn;                   /* Set function pointer */
            if (gsm.evt_func == NULL) {
                gsm.evt_func = newFunc;         /* This should never happen! */
            } else {
                for (func = gsm.evt_func; func->next != NULL; func = func->next) {}
                func->next = newFunc;           /* Set new function as next */
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
 * \note            Function must be first registered using \ref gsm_evt_register
 * \param[in]       fn: Callback function to call on specific event
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_evt_unregister(gsm_evt_fn fn) {
    gsm_evt_func_t* func, *prev;
    GSM_ASSERT("cb_fn != NULL", fn != NULL);    /* Assert input parameters */
    
    GSM_CORE_PROTECT();                         /* Lock GSM core */
    for (prev = gsm.evt_func, func = gsm.evt_func->next; func != NULL; prev = func, func = func->next) {
        if (func->fn == fn) {
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
 * \param[in]       mode: Mode status. Set to `1` for full functionality or `0` for low-power mode (no functionality)
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
 * \brief           Notify stack if device is present or not
 * 
 *                  Use this function to notify stack that device is not connected and not ready to communicate with host device
 * \param[in]       present: Flag indicating device is present
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_device_set_present(uint8_t present, uint32_t blocking) {
    gsmr_t res = gsmOK;
    GSM_CORE_PROTECT();                         /* Protect core */
    gsm.status.f.dev_present = GSM_U8(!!present);   /* Device is present */
    
    /**
     * \todo: Set stack to default values
     */

#if GSM_CFG_RESET_ON_INIT
    if (gsm.status.f.dev_present) {             /* Is new device present? */
        GSM_CORE_UNPROTECT();                   /* Unprotect core */
        res = gsm_reset_with_delay(GSM_CFG_RESET_DELAY_DEFAULT, blocking); /* Reset with delay */
        GSM_CORE_PROTECT();                     /* Protect core */
    }
#else
    GSM_UNUSED(blocking);                       /* Unused variable */
#endif /* GSM_CFG_RESET_ON_INIT */
    
    gsmi_send_cb(GSM_EVT_DEVICE_PRESENT);       /* Send present event */
    
    GSM_CORE_UNPROTECT();                       /* Unprotect core */
    return res;
}

/**
 * \brief           Check if device is present
 * \return          `1` on success, `0` otherwise
 */
uint8_t
gsm_device_is_present(void) {
    uint8_t res;
    GSM_CORE_PROTECT();                         /* Protect core */
    res = gsm.status.f.dev_present;
    GSM_CORE_UNPROTECT();                       /* Unprotect core */
    return res;
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
