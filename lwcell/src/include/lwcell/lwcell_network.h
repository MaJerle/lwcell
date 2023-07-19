/**
 * \file            lwcell_network.h
 * \brief           Network API
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
 * This file is part of LwCELL - Lightweight GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.1
 */
#ifndef LWCELL_NETWORK_HDR_H
#define LWCELL_NETWORK_HDR_H

#include "lwcell/lwcell_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \ingroup         LWCELL
 * \defgroup        LWCELL_NETWORK Network API
 * \brief           Network manager
 * \{
 */

/* Basic commands, always available */
lwcellr_t lwcell_network_rssi(int16_t* rssi, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                            const uint32_t blocking);
lwcell_network_reg_status_t lwcell_network_get_reg_status(void);

/* TCP/IP related commands */
lwcellr_t lwcell_network_attach(const char* apn, const char* user, const char* pass, const lwcell_api_cmd_evt_fn evt_fn,
                              void* const evt_arg, const uint32_t blocking);
lwcellr_t lwcell_network_detach(const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
uint8_t lwcell_network_is_attached(void);
lwcellr_t lwcell_network_copy_ip(lwcell_ip_t* ip);
lwcellr_t lwcell_network_check_status(const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWCELL_NETWORK_HDR_H */
