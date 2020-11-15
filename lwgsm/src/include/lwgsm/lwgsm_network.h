/**
 * \file            lwgsm_network.h
 * \brief           Network API
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
#ifndef LWGSM_HDR_NETWORK_H
#define LWGSM_HDR_NETWORK_H

#include "lwgsm/lwgsm.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \ingroup         LWGSM
 * \defgroup        LWGSM_NETWORK Network API
 * \brief           Network manager
 * \{
 */

/* Basic commands, always available */
lwgsmr_t      lwgsm_network_rssi(int16_t* rssi, const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
lwgsm_network_reg_status_t    lwgsm_network_get_reg_status(void);

/* TCP/IP related commands */
lwgsmr_t      lwgsm_network_attach(const char* apn, const char* user, const char* pass, const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
lwgsmr_t      lwgsm_network_detach(const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
uint8_t     lwgsm_network_is_attached(void);
lwgsmr_t      lwgsm_network_copy_ip(lwgsm_ip_t* ip);
lwgsmr_t      lwgsm_network_check_status(const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWGSM_HDR_NETWORK_H */
