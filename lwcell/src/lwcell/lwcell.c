/**
 * \file            lwcell.c
 * \brief           Main GSM core file
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
#include "lwcell/lwcell.h"
#include "lwcell/lwcell_private.h"
#include "lwcell/lwcell_threads.h"
#include "lwcell/lwcell_timeout.h"
#include "system/lwcell_ll.h"

#if LWCELL_CFG_OS != 1
#error LWCELL_CFG_OS must be set to 1!
#endif

static lwcellr_t prv_def_callback(lwcell_evt_t* cb);
static lwcell_evt_func_t def_evt_link;

lwcell_t lwcell;

/**
 * \brief           Default callback function for events
 * \param[in]       evt: Pointer to callback data structure
 * \return          Member of \ref lwcellr_t enumeration
 */
static lwcellr_t
prv_def_callback(lwcell_evt_t* evt) {
    LWCELL_UNUSED(evt);
    return lwcellOK;
}

#if LWCELL_CFG_KEEP_ALIVE

/**
 * \brief           Keep-alive timeout callback function
 * \param[in]       arg: Custom user argument
 */
static void
prv_keep_alive_timeout_fn(void* arg) {
    /* Dispatch keep-alive events */
    lwcelli_send_cb(LWCELL_EVT_KEEP_ALIVE);

    /* Start new timeout */
    lwcell_timeout_add(LWCELL_CFG_KEEP_ALIVE_TIMEOUT, prv_keep_alive_timeout_fn, arg);
}

#endif /* LWCELL_CFG_KEEP_ALIVE */

/**
 * \brief           Init and prepare GSM stack for device operation
 * \note            Function must be called from operating system thread context.
 *                  It creates necessary threads and waits them to start, thus running operating system is important.
 *                  - When \ref LWCELL_CFG_RESET_ON_INIT is enabled, reset sequence will be sent to device
 *                      otherwise manual call to \ref lwcell_reset is required to setup device
 *
 * \param[in]       evt_func: Global event callback function for all major events
 * \param[in]       blocking: Status whether command should be blocking or not.
 *                      Used when \ref LWCELL_CFG_RESET_ON_INIT is enabled.
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_init(lwcell_evt_fn evt_func, const uint32_t blocking) {
    lwcellr_t res = lwcellOK;

    lwcell.status.f.initialized = 0; /* Clear possible init flag */

    def_evt_link.fn = evt_func != NULL ? evt_func : prv_def_callback;
    lwcell.evt_func = &def_evt_link; /* Set callback function */

    if (!lwcell_sys_init()) { /* Init low-level system */
        goto cleanup;
    }

    if (!lwcell_sys_sem_create(&lwcell.sem_sync, 1)) { /* Create sync semaphore between threads */
        LWCELL_DEBUGF(LWCELL_CFG_DBG_INIT | LWCELL_DBG_LVL_SEVERE | LWCELL_DBG_TYPE_TRACE,
                     "[LWCELL CORE] Cannot allocate sync semaphore!\r\n");
        goto cleanup;
    }

    /* Create message queues */
    if (!lwcell_sys_mbox_create(&lwcell.mbox_producer, LWCELL_CFG_THREAD_PRODUCER_MBOX_SIZE)) {
        LWCELL_DEBUGF(LWCELL_CFG_DBG_INIT | LWCELL_DBG_LVL_SEVERE | LWCELL_DBG_TYPE_TRACE,
                     "[LWCELL CORE] Cannot allocate producer mbox queue!\r\n");
        goto cleanup;
    }
    if (!lwcell_sys_mbox_create(&lwcell.mbox_process, LWCELL_CFG_THREAD_PROCESS_MBOX_SIZE)) {
        LWCELL_DEBUGF(LWCELL_CFG_DBG_INIT | LWCELL_DBG_LVL_SEVERE | LWCELL_DBG_TYPE_TRACE,
                     "[LWCELL CORE] Cannot allocate process mbox queue!\r\n");
        goto cleanup;
    }

    /* Create threads */
    lwcell_sys_sem_wait(&lwcell.sem_sync, 0);
    if (!lwcell_sys_thread_create(&lwcell.thread_produce, "lwcell_produce", lwcell_thread_produce, &lwcell.sem_sync,
                                 LWCELL_SYS_THREAD_SS, LWCELL_SYS_THREAD_PRIO)) {
        LWCELL_DEBUGF(LWCELL_CFG_DBG_INIT | LWCELL_DBG_LVL_SEVERE | LWCELL_DBG_TYPE_TRACE,
                     "[LWCELL CORE] Cannot create producing thread!\r\n");
        lwcell_sys_sem_release(&lwcell.sem_sync); /* Release semaphore and return */
        goto cleanup;
    }
    lwcell_sys_sem_wait(&lwcell.sem_sync, 0); /* Wait semaphore, should be unlocked in produce thread */
    if (!lwcell_sys_thread_create(&lwcell.thread_process, "lwcell_process", lwcell_thread_process, &lwcell.sem_sync,
                                 LWCELL_SYS_THREAD_SS, LWCELL_SYS_THREAD_PRIO)) {
        LWCELL_DEBUGF(LWCELL_CFG_DBG_INIT | LWCELL_DBG_LVL_SEVERE | LWCELL_DBG_TYPE_TRACE,
                     "[LWCELL CORE] Cannot create processing thread!\r\n");
        lwcell_sys_thread_terminate(&lwcell.thread_produce); /* Delete produce thread */
        lwcell_sys_sem_release(&lwcell.sem_sync);            /* Release semaphore and return */
        goto cleanup;
    }
    lwcell_sys_sem_wait(&lwcell.sem_sync, 0); /* Wait semaphore, should be unlocked in produce thread */
    lwcell_sys_sem_release(&lwcell.sem_sync); /* Release semaphore manually */

    lwcell_core_lock();
    lwcell.ll.uart.baudrate = LWCELL_CFG_AT_PORT_BAUDRATE;
    lwcell_ll_init(&lwcell.ll); /* Init low-level communication */

#if !LWCELL_CFG_INPUT_USE_PROCESS
    lwcell_buff_init(&lwcell.buff, LWCELL_CFG_RCV_BUFF_SIZE); /* Init buffer for input data */
#endif                                                     /* !LWCELL_CFG_INPUT_USE_PROCESS */

    lwcell.status.f.initialized = 1; /* We are initialized now */
    lwcell.status.f.dev_present = 1; /* We assume device is present at this point */

    lwcelli_send_cb(LWCELL_EVT_INIT_FINISH); /* Call user callback function */

#if LWCELL_CFG_KEEP_ALIVE
    /* Register keep-alive events */
    lwcell_timeout_add(LWCELL_CFG_KEEP_ALIVE_TIMEOUT, prv_keep_alive_timeout_fn, NULL);
#endif /* LWCELL_CFG_KEEP_ALIVE */

    /*
     * Call reset command and call default
     * AT commands to prepare basic setup for device
     */
#if LWCELL_CFG_RESET_ON_INIT
    if (lwcell.status.f.dev_present) {
        lwcell_core_unlock();
        res = lwcell_reset_with_delay(LWCELL_CFG_RESET_DELAY_DEFAULT, NULL, NULL,
                                     blocking); /* Send reset sequence with delay */
        lwcell_core_lock();
    }
#else  /* LWCELL_CFG_RESET_ON_INIT */
    LWCELL_UNUSED(blocking);
#endif /* !LWCELL_CFG_RESET_ON_INIT */
    lwcell_core_unlock();

    return res;

cleanup:
    if (lwcell_sys_mbox_isvalid(&lwcell.mbox_producer)) {
        lwcell_sys_mbox_delete(&lwcell.mbox_producer);
        lwcell_sys_mbox_invalid(&lwcell.mbox_producer);
    }
    if (lwcell_sys_mbox_isvalid(&lwcell.mbox_process)) {
        lwcell_sys_mbox_delete(&lwcell.mbox_process);
        lwcell_sys_mbox_invalid(&lwcell.mbox_process);
    }
    if (lwcell_sys_sem_isvalid(&lwcell.sem_sync)) {
        lwcell_sys_sem_delete(&lwcell.sem_sync);
        lwcell_sys_sem_invalid(&lwcell.sem_sync);
    }
    return lwcellERRMEM;
}

/**
 * \brief           Execute reset and send default commands
 * \param[in]       evt_fn: Callback function called when command is finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_reset(const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    return lwcell_reset_with_delay(0, evt_fn, evt_arg, blocking);
}

/**
 * \brief           Execute reset and send default commands with delay
 * \param[in]       delay: Number of milliseconds to wait before initiating first command to device
 * \param[in]       evt_fn: Callback function called when command is finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_reset_with_delay(uint32_t delay, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                       const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_RESET;
    LWCELL_MSG_VAR_REF(msg).msg.reset.delay = delay;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 60000);
}

/**
 * \brief           Lock stack from multi-thread access, enable atomic access to core
 *
 * If lock was `0` prior function call, lock is enabled and increased
 *
 * \note            Function may be called multiple times to increase locks.
 *                  Application must take care to call \ref lwcell_core_unlock
 *                  the same amount of time to make sure lock gets back to `0`
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_core_lock(void) {
    lwcell_sys_protect();
    ++lwcell.locked_cnt;
    return lwcellOK;
}

/**
 * \brief           Unlock stack for multi-thread access
 *
 * Used in conjunction with \ref lwcell_core_lock function
 *
 * If lock was non-zero before function call, lock is decreased.
 * When `lock == 0`, protection is disabled and other threads may access to core
 *
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_core_unlock(void) {
    --lwcell.locked_cnt;
    lwcell_sys_unprotect();
    return lwcellOK;
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
lwcell_delay(uint32_t ms) {
    lwcell_sys_sem_t sem;
    if (ms == 0) {
        return 1;
    }
    if (lwcell_sys_sem_create(&sem, 0)) {
        lwcell_sys_sem_wait(&sem, ms);
        lwcell_sys_sem_release(&sem);
        lwcell_sys_sem_delete(&sem);
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
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_set_func_mode(uint8_t mode, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CFUN_SET;
    LWCELL_MSG_VAR_REF(msg).msg.cfun.mode = mode;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 60000);
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
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_device_set_present(uint8_t present, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                         const uint32_t blocking) {
    lwcellr_t res = lwcellOK;
    lwcell_core_lock();
    present = present ? 1 : 0;
    if (present != lwcell.status.f.dev_present) {
        lwcell.status.f.dev_present = present;

        if (!lwcell.status.f.dev_present) {
            /* Manually reset stack to default device state */
            lwcelli_reset_everything(1);
        } else {
#if LWCELL_CFG_RESET_ON_DEVICE_PRESENT
            lwcell_core_unlock();
            res =
                lwcell_reset_with_delay(LWCELL_CFG_RESET_DELAY_DEFAULT, evt_fn, evt_arg, blocking); /* Reset with delay */
            lwcell_core_lock();
#endif /* LWCELL_CFG_RESET_ON_DEVICE_PRESENT */
        }
        lwcelli_send_cb(LWCELL_EVT_DEVICE_PRESENT); /* Send present event */
    }
    lwcell_core_unlock();

    LWCELL_UNUSED(evt_fn);
    LWCELL_UNUSED(evt_arg);
    LWCELL_UNUSED(blocking);

    return res;
}

/**
 * \brief           Check if device is present
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwcell_device_is_present(void) {
    uint8_t res;
    lwcell_core_lock();
    res = lwcell.status.f.dev_present;
    lwcell_core_unlock();
    return res;
}
