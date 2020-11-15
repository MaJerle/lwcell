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
 * This file is part of LwGSM - Lightweight GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.0
 */
#include "lwgsm/lwgsm_private.h"
#include "lwgsm/lwgsm_mem.h"
#include "lwgsm/lwgsm_threads.h"
#include "system/lwgsm_ll.h"

#if LWGSM_CFG_OS != 1
#error LWGSM_CFG_OS must be set to 1!
#endif

static lwgsmr_t   def_callback(lwgsm_evt_t* cb);
static lwgsm_evt_func_t def_evt_link;

lwgsm_t lwgsm;

/**
 * \brief           Default callback function for events
 * \param[in]       evt: Pointer to callback data structure
 * \return          Member of \ref lwgsmr_t enumeration
 */
static lwgsmr_t
def_callback(lwgsm_evt_t* evt) {
    LWGSM_UNUSED(evt);
    return lwgsmOK;
}

/**
 * \brief           Init and prepare GSM stack for device operation
 * \note            Function must be called from operating system thread context.
 *                  It creates necessary threads and waits them to start, thus running operating system is important.
 *                  - When \ref LWGSM_CFG_RESET_ON_INIT is enabled, reset sequence will be sent to device
 *                      otherwise manual call to \ref lwgsm_reset is required to setup device
 *
 * \param[in]       evt_func: Global event callback function for all major events
 * \param[in]       blocking: Status whether command should be blocking or not.
 *                      Used when \ref LWGSM_CFG_RESET_ON_INIT is enabled.
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_init(lwgsm_evt_fn evt_func, const uint32_t blocking) {
    lwgsmr_t res = lwgsmOK;

    lwgsm.status.f.initialized = 0;             /* Clear possible init flag */

    def_evt_link.fn = evt_func != NULL ? evt_func : def_callback;
    lwgsm.evt_func = &def_evt_link;             /* Set callback function */

    if (!lwgsm_sys_init()) {                    /* Init low-level system */
        goto cleanup;
    }

    if (!lwgsm_sys_sem_create(&lwgsm.sem_sync, 1)) {/* Create sync semaphore between threads */
        LWGSM_DEBUGF(LWGSM_CFG_DBG_INIT | LWGSM_DBG_LVL_SEVERE | LWGSM_DBG_TYPE_TRACE,
                   "[CORE] Cannot allocate sync semaphore!\r\n");
        goto cleanup;
    }

    /* Create message queues */
    if (!lwgsm_sys_mbox_create(&lwgsm.mbox_producer, LWGSM_CFG_THREAD_PRODUCER_MBOX_SIZE)) {
        LWGSM_DEBUGF(LWGSM_CFG_DBG_INIT | LWGSM_DBG_LVL_SEVERE | LWGSM_DBG_TYPE_TRACE,
                   "[CORE] Cannot allocate producer mbox queue!\r\n");
        goto cleanup;
    }
    if (!lwgsm_sys_mbox_create(&lwgsm.mbox_process, LWGSM_CFG_THREAD_PROCESS_MBOX_SIZE)) {
        LWGSM_DEBUGF(LWGSM_CFG_DBG_INIT | LWGSM_DBG_LVL_SEVERE | LWGSM_DBG_TYPE_TRACE,
                   "[CORE] Cannot allocate process mbox queue!\r\n");
        goto cleanup;
    }

    /* Create threads */
    lwgsm_sys_sem_wait(&lwgsm.sem_sync, 0);
    if (!lwgsm_sys_thread_create(&lwgsm.thread_produce, "lwgsm_produce", lwgsm_thread_produce, &lwgsm.sem_sync, LWGSM_SYS_THREAD_SS, LWGSM_SYS_THREAD_PRIO)) {
        LWGSM_DEBUGF(LWGSM_CFG_DBG_INIT | LWGSM_DBG_LVL_SEVERE | LWGSM_DBG_TYPE_TRACE,
                   "[CORE] Cannot create producing thread!\r\n");
        lwgsm_sys_sem_release(&lwgsm.sem_sync); /* Release semaphore and return */
        goto cleanup;
    }
    lwgsm_sys_sem_wait(&lwgsm.sem_sync, 0);     /* Wait semaphore, should be unlocked in produce thread */
    if (!lwgsm_sys_thread_create(&lwgsm.thread_process, "lwgsm_process", lwgsm_thread_process, &lwgsm.sem_sync, LWGSM_SYS_THREAD_SS, LWGSM_SYS_THREAD_PRIO)) {
        LWGSM_DEBUGF(LWGSM_CFG_DBG_INIT | LWGSM_DBG_LVL_SEVERE | LWGSM_DBG_TYPE_TRACE,
                   "[CORE] Cannot create processing thread!\r\n");
        lwgsm_sys_thread_terminate(&lwgsm.thread_produce);  /* Delete produce thread */
        lwgsm_sys_sem_release(&lwgsm.sem_sync); /* Release semaphore and return */
        goto cleanup;
    }
    lwgsm_sys_sem_wait(&lwgsm.sem_sync, 0);     /* Wait semaphore, should be unlocked in produce thread */
    lwgsm_sys_sem_release(&lwgsm.sem_sync);     /* Release semaphore manually */

    lwgsm_core_lock();
    lwgsm.ll.uart.baudrate = LWGSM_CFG_AT_PORT_BAUDRATE;
    lwgsm_ll_init(&lwgsm.ll);                   /* Init low-level communication */

#if !LWGSM_CFG_INPUT_USE_PROCESS
    lwgsm_buff_init(&lwgsm.buff, LWGSM_CFG_RCV_BUFF_SIZE);  /* Init buffer for input data */
#endif /* !LWGSM_CFG_INPUT_USE_PROCESS */

    lwgsm.status.f.initialized = 1;             /* We are initialized now */
    lwgsm.status.f.dev_present = 1;             /* We assume device is present at this point */

    lwgsmi_send_cb(LWGSM_EVT_INIT_FINISH);      /* Call user callback function */

    /*
     * Call reset command and call default
     * AT commands to prepare basic setup for device
     */
#if LWGSM_CFG_RESET_ON_INIT
    if (lwgsm.status.f.dev_present) {
        lwgsm_core_unlock();
        res = lwgsm_reset_with_delay(LWGSM_CFG_RESET_DELAY_DEFAULT, NULL, NULL, blocking);  /* Send reset sequence with delay */
        lwgsm_core_lock();
    }
#else /* LWGSM_CFG_RESET_ON_INIT */
    LWGSM_UNUSED(blocking);
#endif /* !LWGSM_CFG_RESET_ON_INIT */
    lwgsm_core_unlock();

    return res;

cleanup:
    if (lwgsm_sys_mbox_isvalid(&lwgsm.mbox_producer)) {
        lwgsm_sys_mbox_delete(&lwgsm.mbox_producer);
        lwgsm_sys_mbox_invalid(&lwgsm.mbox_producer);
    }
    if (lwgsm_sys_mbox_isvalid(&lwgsm.mbox_process)) {
        lwgsm_sys_mbox_delete(&lwgsm.mbox_process);
        lwgsm_sys_mbox_invalid(&lwgsm.mbox_process);
    }
    if (lwgsm_sys_sem_isvalid(&lwgsm.sem_sync)) {
        lwgsm_sys_sem_delete(&lwgsm.sem_sync);
        lwgsm_sys_sem_invalid(&lwgsm.sem_sync);
    }
    return lwgsmERRMEM;
}

/**
 * \brief           Execute reset and send default commands
 * \param[in]       evt_fn: Callback function called when command is finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_reset(const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    return lwgsm_reset_with_delay(0, evt_fn, evt_arg, blocking);
}

/**
 * \brief           Execute reset and send default commands with delay
 * \param[in]       delay: Number of milliseconds to wait before initiating first command to device
 * \param[in]       evt_fn: Callback function called when command is finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_reset_with_delay(uint32_t delay,
                     const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_RESET;
    LWGSM_MSG_VAR_REF(msg).msg.reset.delay = delay;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

/**
 * \brief           Lock stack from multi-thread access, enable atomic access to core
 *
 * If lock was `0` prior function call, lock is enabled and increased
 *
 * \note            Function may be called multiple times to increase locks.
 *                  Application must take care to call \ref lwgsm_core_unlock
 *                  the same amount of time to make sure lock gets back to `0`
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_core_lock(void) {
    lwgsm_sys_protect();
    ++lwgsm.locked_cnt;
    return lwgsmOK;
}

/**
 * \brief           Unlock stack for multi-thread access
 *
 * Used in conjunction with \ref lwgsm_core_lock function
 *
 * If lock was non-zero before function call, lock is decreased.
 * When `lock == 0`, protection is disabled and other threads may access to core
 *
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_core_unlock(void) {
    --lwgsm.locked_cnt;
    lwgsm_sys_unprotect();
    return lwgsmOK;
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
lwgsm_delay(uint32_t ms) {
    lwgsm_sys_sem_t sem;
    if (ms == 0) {
        return 1;
    }
    if (lwgsm_sys_sem_create(&sem, 0)) {
        lwgsm_sys_sem_wait(&sem, ms);
        lwgsm_sys_sem_release(&sem);
        lwgsm_sys_sem_delete(&sem);
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
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_set_func_mode(uint8_t mode,
                  const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_CFUN_SET;
    LWGSM_MSG_VAR_REF(msg).msg.cfun.mode = mode;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
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
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_device_set_present(uint8_t present,
                       const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    lwgsmr_t res = lwgsmOK;
    lwgsm_core_lock();
    present = present ? 1 : 0;
    if (present != lwgsm.status.f.dev_present) {
        lwgsm.status.f.dev_present = present;

        if (!lwgsm.status.f.dev_present) {
            /* Manually reset stack to default device state */
            lwgsmi_reset_everything(1);
        } else {
#if LWGSM_CFG_RESET_ON_DEVICE_PRESENT
            lwgsm_core_unlock();
            res = lwgsm_reset_with_delay(LWGSM_CFG_RESET_DELAY_DEFAULT, evt_fn, evt_arg, blocking); /* Reset with delay */
            lwgsm_core_lock();
#endif /* LWGSM_CFG_RESET_ON_DEVICE_PRESENT */
        }
        lwgsmi_send_cb(LWGSM_EVT_DEVICE_PRESENT);   /* Send present event */
    }
    lwgsm_core_unlock();

    LWGSM_UNUSED(evt_fn);
    LWGSM_UNUSED(evt_arg);
    LWGSM_UNUSED(blocking);

    return res;
}

/**
 * \brief           Check if device is present
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_device_is_present(void) {
    uint8_t res;
    lwgsm_core_lock();
    res = lwgsm.status.f.dev_present;
    lwgsm_core_unlock();
    return res;
}
