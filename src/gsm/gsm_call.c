/**	
 * \file            gsm_call.c
 * \brief           Call API functions
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
#include "gsm/gsm_call.h"
#include "gsm/gsm_mem.h"

#if GSM_CFG_CALL || __DOXYGEN__

#if !__DOXYGEN__
#define CHECK_ENABLED()                 if (!(check_enabled() == gsmOK)) { return gsmERRNOTENABLED; }
#endif /* !__DOXYGEN__ */

/**
 * \brief           Check if sms is enabled
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
static gsmr_t
check_enabled(void) {
    gsmr_t res;
    gsm_core_lock();
    res = gsm.m.call.enabled ? gsmOK : gsmERR;
    gsm_core_unlock();
    return res;
}

/**
 * \brief           Check if call is available
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
static gsmr_t
check_ready(void) {
    gsmr_t res;
    gsm_core_lock();
    res = gsm.m.call.ready ? gsmOK : gsmERR;
    gsm_core_unlock();
    return res;
}

/**
 * \brief           Enable call functionality
 * \param[in]       evt_fn: Callback function called when command is finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
gsmr_t
gsm_call_enable(gsm_api_cmd_evt_fn evt_fn, void* evt_arg, const uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);

    GSM_MSG_VAR_ALLOC(msg);
    GSM_MSG_VAR_SET_EVT(msg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_CALL_ENABLE;
    GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_CLCC_SET;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 60000);
}

/**
 * \brief           Disable call functionality
 * \param[in]       evt_fn: Callback function called when command is finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
gsmr_t
gsm_call_disable(gsm_api_cmd_evt_fn evt_fn, void* evt_arg, const uint32_t blocking) {
    gsm_core_lock();
    gsm.m.call.enabled = 0;
    if (evt_fn != NULL) {
        evt_fn(gsmOK, evt_arg);
    }
    gsm_core_unlock();
    return gsmOK;
}

/**
 * \brief           Start a new voice call
 * \param[in]       evt_fn: Callback function called when command is finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       number: Phone number to call, including country code starting with `+` sign
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_call_start(const char* number,
                gsm_api_cmd_evt_fn evt_fn, void* evt_arg, const uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);

    GSM_ASSERT("number != NULL", number != NULL);   /* Assert input parameters */
    CHECK_ENABLED();                            /* Check if enabled */
    GSM_ASSERT("call_ready", check_ready() == gsmOK);   /* Assert input parameters */

    GSM_MSG_VAR_ALLOC(msg);
    GSM_MSG_VAR_SET_EVT(msg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_ATD;
    GSM_MSG_VAR_REF(msg).msg.call_start.number = number;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 10000);
}

/**
 * \brief           Answer to an incoming call
 * \param[in]       evt_fn: Callback function called when command is finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_call_answer(gsm_api_cmd_evt_fn evt_fn, void* evt_arg, const uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);

    CHECK_ENABLED();

    GSM_MSG_VAR_ALLOC(msg);
    GSM_MSG_VAR_SET_EVT(msg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_ATA;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 10000);
}

/**
 * \brief           Hang-up incoming or active call
 * \param[in]       evt_fn: Callback function called when command is finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
gsm_call_hangup(gsm_api_cmd_evt_fn evt_fn, void* evt_arg, const uint32_t blocking) {
    GSM_MSG_VAR_DEFINE(msg);

    CHECK_ENABLED();

    GSM_MSG_VAR_ALLOC(msg);
    GSM_MSG_VAR_SET_EVT(msg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_ATH;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 10000);
}

#endif /* GSM_CFG_CALL || __DOXYGEN__ */
