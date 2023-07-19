/**
 * \file            lwcell_network_api.c
 * \brief           API functions for multi-thread network functions
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
#include "lwcell/lwcell_network_api.h"
#include "lwcell/lwcell_network.h"
#include "lwcell/lwcell_private.h"

#if LWCELL_CFG_NETWORK || __DOXYGEN__

/* Network credentials used during connect operation */
static const char* network_apn;
static const char* network_user;
static const char* network_pass;
static uint32_t network_counter;

/**
 * \brief           Set system network credentials before asking for attach
 * \param[in]       apn: APN domain. Set to `NULL` if not used
 * \param[in]       user: APN username. Set to `NULL` if not used
 * \param[in]       pass: APN password. Set to `NULL` if not used
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_network_set_credentials(const char* apn, const char* user, const char* pass) {
    network_apn = apn;
    network_user = user;
    network_pass = pass;

    return lwcellOK;
}

/**
 * \brief           Request manager to attach to network
 * \note            This function is blocking and cannot be called from event functions
 * \return          \ref lwcellOK on success (when attached), member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_network_request_attach(void) {
    lwcellr_t res = lwcellOK;
    uint8_t do_conn = 0;

    /* Check if we need to connect */
    lwcell_core_lock();
    if (network_counter == 0) {
        if (!lwcell_network_is_attached()) {
            do_conn = 1;
        }
    }
    if (!do_conn) {
        ++network_counter;
    }
    lwcell_core_unlock();

    /* Connect to network */
    if (do_conn) {
        res = lwcell_network_attach(network_apn, network_user, network_pass, NULL, NULL, 1);
        if (res == lwcellOK) {
            lwcell_core_lock();
            ++network_counter;
            lwcell_core_unlock();
        }
    }
    return res;
}

/**
 * \brief           Request manager to detach from network
 *
 * If other threads use network, manager will not disconnect from network
 * otherwise it will disable network access
 *
 * \note            This function is blocking and cannot be called from event functions
 * \return          \ref lwcellOK on success (when attached), member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_network_request_detach(void) {
    lwcellr_t res = lwcellOK;
    uint8_t do_disconn = 0;

    /* Check if we need to disconnect */
    lwcell_core_lock();
    if (network_counter > 0) {
        if (network_counter == 1) {
            do_disconn = 1;
        } else {
            --network_counter;
        }
    }
    lwcell_core_unlock();

    /* Connect to network */
    if (do_disconn) {
        res = lwcell_network_detach(NULL, NULL, 1);
        if (res == lwcellOK) {
            lwcell_core_lock();
            --network_counter;
            lwcell_core_unlock();
        }
    }
    return res;
}

#endif /* LWCELL_CFG_NETWORK || __DOXYGEN__ */
