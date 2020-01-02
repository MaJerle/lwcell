/**
 * \file            gsm_sim.c
 * \brief           SIM API
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
#include "gsm/gsm_private.h"
#include "gsm/gsm_sim.h"
#include "gsm/gsm_mem.h"

/**
 * \brief           Get current cached SIM state from stack
 * \note            Information is always valid, starting after successful device reset using \ref gsm_reset function call
 * \return          Member of \ref gsm_sim_state_t enumeration
 */
gsm_sim_state_t
gsm_sim_get_current_state(void) {
    gsm_sim_state_t state;
    gsm_core_lock();
    state = gsm.m.sim.state;
    gsm_core_unlock();
    return state;
}

/**
 * \brief           Enter pin code to unlock SIM
 * \param[in]       pin: Pin code in string format
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_sim_pin_enter(const char* pin,
                    const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);

    GSM_ASSERT("pin != NULL", pin != NULL);

    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_CPIN_SET;
    GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CPIN_GET;
    GSM_MSG_VAR_REF(msg).msg.cpin_enter.pin = pin;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 30000);
}

/**
 * \brief           Add pin number to open SIM card
 * \note            Use this function only if your SIM card doesn't have PIN code.
 *                  If you wish to change current pin, use \ref gsm_sim_pin_change instead
 * \param[in]       pin: Current SIM pin code
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_sim_pin_add(const char* pin,
                    const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);

    GSM_ASSERT("pin != NULL", pin != NULL);

    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_CPIN_ADD;
    GSM_MSG_VAR_REF(msg).msg.cpin_add.pin = pin;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 10000);
}

/**
 * \brief           Change current pin code
 * \param[in]       pin: Current pin code
 * \param[in]       new_pin: New pin code
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_sim_pin_change(const char* pin, const char* new_pin, 
                    const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);

    GSM_ASSERT("pin != NULL", pin != NULL);
    GSM_ASSERT("new_pin != NULL", new_pin != NULL);

    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_CPIN_CHANGE;
    GSM_MSG_VAR_REF(msg).msg.cpin_change.current_pin = pin;
    GSM_MSG_VAR_REF(msg).msg.cpin_change.new_pin = new_pin;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 10000);
}

/**
 * \brief           Remove pin code from SIM
 * \param[in]       pin: Current pin code
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_sim_pin_remove(const char* pin,
                    const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);

    GSM_ASSERT("pin != NULL", pin != NULL);

    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_CPIN_REMOVE;
    GSM_MSG_VAR_REF(msg).msg.cpin_remove.pin = pin;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 10000);
}

/**
 * \brief           Enter PUK code and new PIN to unlock SIM card
 * \param[in]       puk: PUK code associated with SIM card
 * \param[in]       new_pin: New PIN code to use
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_sim_puk_enter(const char* puk, const char* new_pin,
                    const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);

    GSM_ASSERT("puk != NULL", puk != NULL);
    GSM_ASSERT("new_pin != NULL", new_pin != NULL);

    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GMM_CMD_CPUK_SET;
    GSM_MSG_VAR_REF(msg).msg.cpuk_enter.puk = puk;
    GSM_MSG_VAR_REF(msg).msg.cpuk_enter.pin = new_pin;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 10000);
}
