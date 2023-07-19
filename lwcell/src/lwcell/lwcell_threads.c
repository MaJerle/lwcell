/**
 * \file            lwcell_threads.c
 * \brief           OS threads implementations
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
#include "lwcell/lwcell_threads.h"
#include "lwcell/lwcell_int.h"
#include "lwcell/lwcell_parser.h"
#include "lwcell/lwcell_private.h"
#include "lwcell/lwcell_timeout.h"
#include "system/lwcell_sys.h"

/**
 * \brief           User thread to process input packets from API functions
 * \param[in]       arg: User argument. Semaphore to release when thread starts
 */
void
lwcell_thread_produce(void* const arg) {
    lwcell_sys_sem_t* sem = arg;
    lwcell_t* e = &lwcell;
    lwcell_msg_t* msg;
    lwcellr_t res;
    uint32_t time;

    /* Thread is running, unlock semaphore */
    if (lwcell_sys_sem_isvalid(sem)) {
        lwcell_sys_sem_release(sem); /* Release semaphore */
    }

    lwcell_core_lock();
    while (1) {
        lwcell_core_unlock();
        do {
            time = lwcell_sys_mbox_get(&e->mbox_producer, (void**)&msg, 0); /* Get message from queue */
        } while (time == LWCELL_SYS_TIMEOUT || msg == NULL);
        LWCELL_THREAD_PRODUCER_HOOK(); /* Execute producer thread hook */
        lwcell_core_lock();

        res = lwcellOK; /* Start with OK */
        e->msg = msg;   /* Set message handle */

        /*
         * This check is performed when adding command to queue
         * Do it again here to prevent long timeouts,
         * if device present flag changes
         */
        if (!e->status.f.dev_present) {
            res = lwcellERRNODEVICE;
        }

        /* For reset message, we can have delay! */
        if (res == lwcellOK && msg->cmd_def == LWCELL_CMD_RESET) {
            if (msg->msg.reset.delay > 0) {
                lwcell_delay(msg->msg.reset.delay);
            }
            lwcelli_reset_everything(1); /* Reset stack before trying to reset */
        }

        /*
         * Try to call function to process this message
         * Usually it should be function to transmit data to AT port
         */
        if (res == lwcellOK && msg->fn != NULL) { /* Check for callback processing function */
            /*
             * Obtain semaphore
             * This code should not block at any point.
             * If it blocks, severe problems occurred and program should
             * immediate terminate
             */
            lwcell_core_unlock();
            lwcell_sys_sem_wait(&e->sem_sync, 0); /* First call */
            lwcell_core_lock();
            res = msg->fn(msg);         /* Process this message, check if command started at least */
            time = ~LWCELL_SYS_TIMEOUT; /* Reset time */
            if (res == lwcellOK) {      /* We have valid data and data were sent */
                lwcell_core_unlock();
                time = lwcell_sys_sem_wait(
                    &e->sem_sync,
                    msg->block_time); /* Second call; Wait for synchronization semaphore from processing thread or timeout */
                lwcell_core_lock();
                if (time == LWCELL_SYS_TIMEOUT) { /* Sync timeout occurred? */
                    res = lwcellTIMEOUT;          /* Timeout on command */
                }
            }

            /* Notify application on command timeout */
            if (res == lwcellTIMEOUT) {
                lwcelli_send_cb(LWCELL_EVT_CMD_TIMEOUT);
            }

            LWCELL_DEBUGW(
                LWCELL_CFG_DBG_THREAD | LWCELL_DBG_TYPE_TRACE | LWCELL_DBG_LVL_SEVERE, res == lwcellTIMEOUT,
                "[LWCELL THREAD] Timeout in produce thread waiting for command to finish in process thread\r\n");
            LWCELL_DEBUGW(LWCELL_CFG_DBG_THREAD | LWCELL_DBG_TYPE_TRACE | LWCELL_DBG_LVL_SEVERE,
                          res != lwcellOK && res != lwcellTIMEOUT,
                          "[LWCELL THREAD] Could not start execution for command %d\r\n", (int)msg->cmd);

            /*
             * Manually release semaphore in all cases:
             *
             * Case 1: msg->fn function fails, command did not start,
             *           application needs to release previously acquired semaphore
             * Case 2: If time == TIMEOUT, acquiring on second call was not successful,
             *           application has to manually release semaphore, taken on first call
             * Case 3: If time != TIMEOUT, acquiring on second call was successful,
             *           which effectively means that another thread successfully released semaphore,
             *           application has to release semaphore, now taken on second call
             *
             * If application would not manually release semaphore,
             * and if command would return with timeout (or fail),
             * it would not be possible to start a new command after,
             * because semaphore would be still locked
             */
            lwcell_sys_sem_release(&e->sem_sync);
        } else {
            if (res == lwcellOK) {
                res = lwcellERR; /* Simply set error message */
            }
        }
        if (res != lwcellOK) {
            /* Process global callbacks */
            lwcelli_process_events_for_timeout_or_error(msg, res);

            msg->res = res; /* Save response */
        }

#if LWCELL_CFG_USE_API_FUNC_EVT
        /* Send event function to user */
        if (msg->evt_fn != NULL) {
            msg->evt_fn(msg->res, msg->evt_arg); /* Send event with user argument */
        }
#endif /* LWCELL_CFG_USE_API_FUNC_EVT */

        /*
         * In case message is blocking,
         * release semaphore and notify finished with processing
         * otherwise directly free memory of message structure
         */
        if (msg->is_blocking) {
            lwcell_sys_sem_release(&msg->sem);
        } else {
            LWCELL_MSG_VAR_FREE(msg);
        }
        e->msg = NULL;
    }
}

/**
 * \brief           Thread for processing received data from device
 *
 *                  This thread is also used to handle timeout events
 *                  in correct time order as it is never blocked by user command
 *
 * \param[in]       arg: User argument. Semaphore to release when thread starts
 * \sa              LWCELL_CFG_INPUT_USE_PROCESS
 */
void
lwcell_thread_process(void* const arg) {
    lwcell_sys_sem_t* sem = arg;
    lwcell_t* e = &lwcell;
    lwcell_msg_t* msg;
    uint32_t time;

    /* Thread is running, unlock semaphore */
    if (lwcell_sys_sem_isvalid(sem)) {
        lwcell_sys_sem_release(sem); /* Release semaphore */
    }

#if !LWCELL_CFG_INPUT_USE_PROCESS
    lwcell_core_lock();
    while (1) {
        lwcell_core_unlock();
        time = lwcelli_get_from_mbox_with_timeout_checks(&e->mbox_process, (void**)&msg, 10);
        LWCELL_THREAD_PROCESS_HOOK(); /* Execute process thread hook */
        lwcell_core_lock();

        if (time == LWCELL_SYS_TIMEOUT || msg == NULL) {
            LWCELL_UNUSED(time); /* Unused variable */
        }
        lwcelli_process_buffer(); /* Process input data */
#else                             /* LWCELL_CFG_INPUT_USE_PROCESS */
    while (1) {
        /*
         * Check for next timeout event only here
         *
         * If there are no timeouts to process, we can wait unlimited time.
         * In case new timeout occurs, thread will wake up by writing new element to mbox process queue
         */
        time = lwcelli_get_from_mbox_with_timeout_checks(&e->mbox_process, (void**)&msg, 0);
        LWCELL_THREAD_PROCESS_HOOK(); /* Execute process thread hook */
        LWCELL_UNUSED(time);
#endif                            /* !LWCELL_CFG_INPUT_USE_PROCESS */
    }
}
