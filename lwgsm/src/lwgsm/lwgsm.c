/**
 * \file            lwgsm.c
 * \brief           Main GSM core file
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
 * This file is part of GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         $_version_$
 */
#include "lwgsm/lwgsm_private.h"
#include "lwgsm/lwgsm_mem.h"
#include "lwgsm/lwgsm_threads.h"
#include "system/lwgsm_ll.h"

#if GSM_CFG_OS != 1
#error GSM_CFG_OS must be set to 1!
#endif

static gsmr_t   def_callback(gsm_evt_t* cb);
static gsm_evt_func_t def_evt_link;

gsm_t gsm;

/**
 * \brief           Default callback function for events
 * \param[in]       evt: Pointer to callback data structure
 * \return          Member of \ref gsmr_t enumeration
 */
static gsmr_t
def_callback(gsm_evt_t* evt) {
    GSM_UNUSED(evt);
    return gsmOK;
}

/**
 * \brief           Init and prepare GSM stack for device operation
 * \note            Function must be called from operating system thread context.
 *                  It creates necessary threads and waits them to start, thus running operating system is important.
 *                  - When \ref GSM_CFG_RESET_ON_INIT is enabled, reset sequence will be sent to device
 *                      otherwise manual call to \ref gsm_reset is required to setup device
 *
 * \param[in]       evt_func: Global event callback function for all major events
 * \param[in]       blocking: Status whether command should be blocking or not.
 *                      Used when \ref GSM_CFG_RESET_ON_INIT is enabled.
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_init(gsm_evt_fn evt_func, const uint32_t blocking) {
    gsmr_t res = gsmOK;

    gsm.status.f.initialized = 0;               /* Clear possible init flag */

    def_evt_link.fn = evt_func != NULL ? evt_func : def_callback;
    gsm.evt_func = &def_evt_link;               /* Set callback function */

    if (!gsm_sys_init()) {                      /* Init low-level system */
        goto cleanup;
    }

    if (!gsm_sys_sem_create(&gsm.sem_sync, 1)) {/* Create sync semaphore between threads */
        GSM_DEBUGF(GSM_CFG_DBG_INIT | GSM_DBG_LVL_SEVERE | GSM_DBG_TYPE_TRACE,
                   "[CORE] Cannot allocate sync semaphore!\r\n");
        goto cleanup;
    }

    /* Create message queues */
    if (!gsm_sys_mbox_create(&gsm.mbox_producer, GSM_CFG_THREAD_PRODUCER_MBOX_SIZE)) {  /* Producer */
        GSM_DEBUGF(GSM_CFG_DBG_INIT | GSM_DBG_LVL_SEVERE | GSM_DBG_TYPE_TRACE,
                   "[CORE] Cannot allocate producer mbox queue!\r\n");
        goto cleanup;
    }
    if (!gsm_sys_mbox_create(&gsm.mbox_process, GSM_CFG_THREAD_PROCESS_MBOX_SIZE)) {    /* Process */
        GSM_DEBUGF(GSM_CFG_DBG_INIT | GSM_DBG_LVL_SEVERE | GSM_DBG_TYPE_TRACE,
                   "[CORE] Cannot allocate process mbox queue!\r\n");
        goto cleanup;
    }

    /* Create threads */
    gsm_sys_sem_wait(&gsm.sem_sync, 0);         /* Lock semaphore */
    if (!gsm_sys_thread_create(&gsm.thread_produce, "gsm_produce", gsm_thread_produce, &gsm.sem_sync, GSM_SYS_THREAD_SS, GSM_SYS_THREAD_PRIO)) {
        GSM_DEBUGF(GSM_CFG_DBG_INIT | GSM_DBG_LVL_SEVERE | GSM_DBG_TYPE_TRACE,
                   "[CORE] Cannot create producing thread!\r\n");
        gsm_sys_sem_release(&gsm.sem_sync);     /* Release semaphore and return */
        goto cleanup;
    }
    gsm_sys_sem_wait(&gsm.sem_sync, 0);         /* Wait semaphore, should be unlocked in produce thread */
    if (!gsm_sys_thread_create(&gsm.thread_process, "gsm_process", gsm_thread_process, &gsm.sem_sync, GSM_SYS_THREAD_SS, GSM_SYS_THREAD_PRIO)) {
        GSM_DEBUGF(GSM_CFG_DBG_INIT | GSM_DBG_LVL_SEVERE | GSM_DBG_TYPE_TRACE,
                   "[CORE] Cannot create processing thread!\r\n");
        gsm_sys_thread_terminate(&gsm.thread_produce);  /* Delete produce thread */
        gsm_sys_sem_release(&gsm.sem_sync);     /* Release semaphore and return */
        goto cleanup;
    }
    gsm_sys_sem_wait(&gsm.sem_sync, 0);         /* Wait semaphore, should be unlocked in produce thread */
    gsm_sys_sem_release(&gsm.sem_sync);         /* Release semaphore manually */

    gsm_core_lock();
    gsm.ll.uart.baudrate = GSM_CFG_AT_PORT_BAUDRATE;
    gsm_ll_init(&gsm.ll);                       /* Init low-level communication */

#if !GSM_CFG_INPUT_USE_PROCESS
    gsm_buff_init(&gsm.buff, GSM_CFG_RCV_BUFF_SIZE);    /* Init buffer for input data */
#endif /* !GSM_CFG_INPUT_USE_PROCESS */

    gsm.status.f.initialized = 1;               /* We are initialized now */
    gsm.status.f.dev_present = 1;               /* We assume device is present at this point */

    gsmi_send_cb(GSM_EVT_INIT_FINISH);          /* Call user callback function */

    /*
     * Call reset command and call default
     * AT commands to prepare basic setup for device
     */
#if GSM_CFG_RESET_ON_INIT
    if (gsm.status.f.dev_present) {
        gsm_core_unlock();
        res = gsm_reset_with_delay(GSM_CFG_RESET_DELAY_DEFAULT, NULL, NULL, blocking);  /* Send reset sequence with delay */
        gsm_core_lock();
    }
#else /* GSM_CFG_RESET_ON_INIT */
    GSM_UNUSED(blocking);                       /* Unused variable */
#endif /* !GSM_CFG_RESET_ON_INIT */
    gsm_core_unlock();

    return res;

cleanup:
    if (gsm_sys_mbox_isvalid(&gsm.mbox_producer)) {
        gsm_sys_mbox_delete(&gsm.mbox_producer);
        gsm_sys_mbox_invalid(&gsm.mbox_producer);
    }
    if (gsm_sys_mbox_isvalid(&gsm.mbox_process)) {
        gsm_sys_mbox_delete(&gsm.mbox_process);
        gsm_sys_mbox_invalid(&gsm.mbox_process);
    }
    if (gsm_sys_sem_isvalid(&gsm.sem_sync)) {
        gsm_sys_sem_delete(&gsm.sem_sync);
        gsm_sys_sem_invalid(&gsm.sem_sync);
    }
    return gsmERRMEM;
}

/**
 * \brief           Execute reset and send default commands
 * \param[in]       evt_fn: Callback function called when command is finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_reset(const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    return gsm_reset_with_delay(0, evt_fn, evt_arg, blocking);
}

/**
 * \brief           Execute reset and send default commands with delay
 * \param[in]       delay: Number of milliseconds to wait before initiating first command to device
 * \param[in]       evt_fn: Callback function called when command is finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_reset_with_delay(uint32_t delay,
                     const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);

    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_RESET;
    GSM_MSG_VAR_REF(msg).msg.reset.delay = delay;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 60000);
}

/**
 * \brief           Lock stack from multi-thread access, enable atomic access to core
 *
 * If lock was `0` prior funcion call, lock is enabled and increased
 *
 * \note            Function may be called multiple times to increase locks.
 *                  Application must take care to call \ref gsm_core_unlock
 *                  the same amount of time to make sure lock gets back to `0`
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_core_lock(void) {
    gsm_sys_protect();
    ++gsm.locked_cnt;
    return gsmOK;
}

/**
 * \brief           Unlock stack for multi-thread access
 *
 * Used in conjunction with \ref gsm_core_lock function
 *
 * If lock was non-zero before function call, lock is decreased.
 * When `lock == 0`, protection is disabled and other threads may access to core
 *
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_core_unlock(void) {
    --gsm.locked_cnt;
    gsm_sys_unprotect();
    return gsmOK;
}

/**
 * \brief           Delay for amount of milliseconds
 *
 * Delay is based on operating system semaphores.
 * It locks semaphore and waits for timeout in `ms` time.
 * Based on operating system, thread may be put to \e blocked list during delay and may improve execution speed
 *
 * \param[in]       ms: Milliseconds to delay
 * \return          `1` on success, `0` otherwise
 */
uint8_t
gsm_delay(uint32_t ms) {
    gsm_sys_sem_t sem;
    if (ms == 0) {
        return 1;
    }
    if (gsm_sys_sem_create(&sem, 0)) {
        gsm_sys_sem_wait(&sem, ms);
        gsm_sys_sem_release(&sem);
        gsm_sys_sem_delete(&sem);
        return 1;
    }
    return 0;
}

/**
 * \brief           Set modem function mode
 * \note            Use this function to set modem to normal or low-power mode
 * \param[in]       mode: Mode status. Set to `1` for full functionality or `0` for low-power mode (no functionality)
 * \param[in]       evt_fn: Callback function called when command is finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_set_func_mode(uint8_t mode,
                  const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);

    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_CFUN_SET;
    GSM_MSG_VAR_REF(msg).msg.cfun.mode = mode;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 60000);
}

/**
 * \brief           Notify stack if device is present or not
 *
 * Use this function to notify stack that device is not physically connected
 * and not ready to communicate with host device
 *
 * \param[in]       present: Flag indicating device is present
 * \param[in]       evt_fn: Callback function called when command is finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_device_set_present(uint8_t present,
                       const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    gsmr_t res = gsmOK;
    gsm_core_lock();
    present = present ? 1 : 0;
    if (present != gsm.status.f.dev_present) {
        gsm.status.f.dev_present = present;

        if (!gsm.status.f.dev_present) {
            /* Manually reset stack to default device state */
            gsmi_reset_everything(1);
        } else {
#if GSM_CFG_RESET_ON_DEVICE_PRESENT
            gsm_core_unlock();
            res = gsm_reset_with_delay(GSM_CFG_RESET_DELAY_DEFAULT, evt_fn, evt_arg, blocking); /* Reset with delay */
            gsm_core_lock();
#endif /* GSM_CFG_RESET_ON_DEVICE_PRESENT */
        }
        gsmi_send_cb(GSM_EVT_DEVICE_PRESENT);   /* Send present event */
    }
    gsm_core_unlock();

    GSM_UNUSED(evt_fn);
    GSM_UNUSED(evt_arg);
    GSM_UNUSED(blocking);

    return res;
}

/**
 * \brief           Check if device is present
 * \return          `1` on success, `0` otherwise
 */
uint8_t
gsm_device_is_present(void) {
    uint8_t res;
    gsm_core_lock();
    res = gsm.status.f.dev_present;
    gsm_core_unlock();
    return res;
}
