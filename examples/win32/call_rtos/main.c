/**
 * \file            main.c
 * \brief           Main file
 */

/*
 * Copyright (c) 2019 Tilen MAJERLE
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
 * Before you start using WIN32 implementation with USB and VCP,
 * check gsm_ll_win32.c implementation and choose your COM port!
 */
#include "gsm/gsm.h"
#include "sim_manager.h"
#include "network_utils.h"

static gsmr_t gsm_callback_func(gsm_evt_t* evt);

/**
 * \brief           Program entry point
 */
int
main(void) {
    printf("Starting GSM application!\r\n");

    /* Initialize GSM with default callback function */
    if (gsm_init(gsm_callback_func, 1) != gsmOK) {
        printf("Cannot initialize GSM-AT Library\r\n");
    }

    /* Configure device by unlocking SIM card */
    if (configure_sim_card()) {
        printf("SIM card configured. Adding delay to stabilize SIM card.\r\n");
        gsm_delay(10000);
    } else {
        printf("Cannot configure SIM card! Is it inserted, pin valid and not under PUK? Closing down...\r\n");
        while (1) { gsm_delay(1000); }
    }

    /* Enable calls */
    if (gsm_call_enable(NULL, NULL, 1) == gsmOK) {
        printf("Calls enabled. You may take your phone and call modem.\r\n");
    } else {
        printf("Could not enabled call functionality!\r\n");
    }

    /*
     * Do not stop program here.
     * New threads were created for GSM processing
     */
    while (1) {
        gsm_delay(1000);
    }

    return 0;
}

/**
 * \brief           Event callback function for GSM stack
 * \param[in]       evt: Event information with data
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
static gsmr_t
gsm_callback_func(gsm_evt_t* evt) {
    switch (gsm_evt_get_type(evt)) {
        case GSM_EVT_INIT_FINISH: printf("Library initialized!\r\n"); break;
        /* Process and print registration change */
        case GSM_EVT_NETWORK_REG_CHANGED: network_utils_process_reg_change(evt); break;
        /* Process current network operator */
        case GSM_EVT_NETWORK_OPERATOR_CURRENT: network_utils_process_curr_operator(evt); break;
        /* Process signal strength */
        case GSM_EVT_SIGNAL_STRENGTH: network_utils_process_rssi(evt); break;

        /* Other user events here... */
#if GSM_CFG_CALL
        case GSM_EVT_CALL_CHANGED: {
            const gsm_call_t* call = gsm_evt_call_changed_get_call(evt);
            if (call->state == GSM_CALL_STATE_ACTIVE) {
                printf("Call is active!\r\n");

                /* In case of mobile originated direction */
                if (call->dir == GSM_CALL_DIR_MO) {
                    gsm_call_hangup(NULL, NULL, 0); /* Manually hangup call */
                }
            } else if (call->state == GSM_CALL_STATE_INCOMING) {
                printf("We received incoming call! Phone number: %s\r\n", call->number);
                gsm_call_answer(NULL, NULL, 0); /* Answer to a call */
            } else if (call->state == GSM_CALL_STATE_DIALING) {
                printf("Call is dialing!\r\n");
            } else if (call->state == GSM_CALL_STATE_DISCONNECT) {
                printf("Call ended!\r\n");
            }
            break;
        }
#endif /* GSM_CFG_CALL */

        default: break;
    }
    return gsmOK;
}
