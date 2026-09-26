/**
 * \file            lwcell_gnss.c
 * \brief           GNSS/GPS API
 */

/*
 * Copyright (c) 2024 Tilen MAJERLE
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
 *                  Harald LESCHNER <picohari@googlemail.com>
 * Version:         v0.1.2
 */
#include "lwcell/lwcell_gnss.h"
#include "lwcell/lwcell_private.h"

#if LWCELL_CFG_GNSS || __DOXYGEN__

#if !__DOXYGEN__
#define CHECK_ENABLED()                                                                                                \
    if (!(check_enabled() == lwcellOK)) {                                                                              \
        return lwcellERRNOTENABLED;                                                                                    \
    }
#endif /* !__DOXYGEN__ */

/**
 * \brief           Check if gnss feature is enabled
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
static lwcellr_t
check_enabled(void) {
    lwcellr_t res;
    lwcell_core_lock();
    res = lwcell.m.gnss.enabled ? lwcellOK : lwcellERR;
    lwcell_core_unlock();
    return res;
}

/**
 * \brief           Enable GNSS functionality
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_gnss_enable(const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_GNSS_ENABLE;
    LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_GNSS_CGNSPWR;
    LWCELL_MSG_VAR_REF(msg).msg.gnss_power.enable = 1; /* Turn on GNSS power supply  */

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 1000);
}

/**
 * \brief           Disable SMS functionality
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_gnss_disable(const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_GNSS_ENABLE;
    LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_GNSS_CGNSPWR;
    LWCELL_MSG_VAR_REF(msg).msg.gnss_power.enable = 0; /* Turn off GNSS power supply  */

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 1000);
}

/**
 * \brief           Read GNSS/GPS data
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_gnss_info(const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWCELL_MSG_VAR_DEFINE(msg);

    CHECK_ENABLED(); /* No point querying GNSS info if GNSS was never enabled */

    LWCELL_MSG_VAR_ALLOC(msg, blocking);
    LWCELL_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWCELL_MSG_VAR_REF(msg).cmd_def = LWCELL_CMD_GNSS_CGNSINF;
    LWCELL_MSG_VAR_REF(msg).cmd = LWCELL_CMD_GNSS_CGNSINF;

    return lwcelli_send_msg_to_producer_mbox(&LWCELL_MSG_VAR_REF(msg), lwcelli_initiate_cmd, 1000);
}

/**
 * \brief           Get a thread-safe snapshot of the last known GNSS date/time
 * \param[out]      dt: Pointer to \ref struct tm to be filled with UTC date/time
 * \param[out]      ms: Optional pointer to receive milliseconds fraction, can be `NULL`
 * \param[out]      fix_status: Optional pointer to receive current fix status, can be `NULL`
 * \return          `1` if a valid fix was ever received, `0` if no valid time is available yet
 */
uint8_t
lwcell_gnss_get_time(struct tm* dt, uint16_t* ms, uint8_t* fix_status) {
    uint8_t valid;

    LWCELL_ASSERT(dt != NULL);

    lwcell_core_lock();
    /*
     * fix_status != 0 means the module has at least one valid GNSS fix
     * (per SIM7000 +CGNSINF docs: 0 = no fix, 1 = fix available).
     * Without ever having had a fix, dt would still hold the zero-initialized
     * struct from lwcelli_reset_everything() and must not be reported as valid.
     */
    valid = (lwcell.m.gnss.fix_status != 0);
    if (valid) {
        *dt = lwcell.m.gnss.dt;
        if (ms != NULL) {
            *ms = lwcell.m.gnss.ms;
        }
    }
    if (fix_status != NULL) {
        *fix_status = lwcell.m.gnss.fix_status;
    }
    lwcell_core_unlock();

    return valid;
}

#endif /* LWCELL_CFG_GNSS || __DOXYGEN__ */