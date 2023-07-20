/**
 * \file            lwcell_sim.c
 * \brief           SIM API
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
#include "lwcell/lwcell_sim.h"
#include "lwcell/lwcell_private.h"

/**
 * \brief           Get current cached SIM state from stack
 * \note            Information is always valid, starting after successful device reset using \ref lwcell_reset function call
 * \return          Member of \ref lwcell_sim_state_t enumeration
 */
lwcell_sim_state_t
lwcell_sim_get_current_state(void) {
    lwcell_sim_state_t state;
    lwcell_core_lock();
    state = lwcell.m.sim.state;
    lwcell_core_unlock();
    return state;
}

/**
 * \brief           Enter pin code to unlock SIM
 * \param[in]       pin: Pin code in string format
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_sim_pin_enter(const char* pin, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                     const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_ASSERT(pin != NULL);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CPIN_SET;
    LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_CPIN_GET;
    LWCELL_MSG_VAR_REF(msg).msg.cpin_enter.pin = pin;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 30000);
}

/**
 * \brief           Add pin number to open SIM card
 * \note            Use this function only if your SIM card doesn't have PIN code.
 *                  If you wish to change current pin, use \ref lwcell_sim_pin_change instead
 * \param[in]       pin: Current SIM pin code
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_sim_pin_add(const char* pin, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_ASSERT(pin != NULL);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CPIN_ADD;
    LWCELL_MSG_VAR_REF(msg).msg.cpin_add.pin = pin;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 10000);
}

/**
 * \brief           Change current pin code
 * \param[in]       pin: Current pin code
 * \param[in]       new_pin: New pin code
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_sim_pin_change(const char* pin, const char* new_pin, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                      const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_ASSERT(pin != NULL);
    LWCELL_ASSERT(new_pin != NULL);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CPIN_CHANGE;
    LWCELL_MSG_VAR_REF(msg).msg.cpin_change.current_pin = pin;
    LWCELL_MSG_VAR_REF(msg).msg.cpin_change.new_pin = new_pin;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 10000);
}

/**
 * \brief           Remove pin code from SIM
 * \param[in]       pin: Current pin code
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_sim_pin_remove(const char* pin, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                      const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_ASSERT(pin != NULL);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CPIN_REMOVE;
    LWCELL_MSG_VAR_REF(msg).msg.cpin_remove.pin = pin;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 10000);
}

/**
 * \brief           Enter PUK code and new PIN to unlock SIM card
 * \param[in]       puk: PUK code associated with SIM card
 * \param[in]       new_pin: New PIN code to use
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_sim_puk_enter(const char* puk, const char* new_pin, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                     const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_ASSERT(puk != NULL);
    LWCELL_ASSERT(new_pin != NULL);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_CPUK_SET;
    LWCELL_MSG_VAR_REF(msg).msg.cpuk_enter.puk = puk;
    LWCELL_MSG_VAR_REF(msg).msg.cpuk_enter.pin = new_pin;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 10000);
}
