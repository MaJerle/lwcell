/**
 * \file            gsm.h
 * \brief           GSM AT commands parser
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
 * This file is part of GSM-AT.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 */
#ifndef __GSM_H
#define __GSM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \defgroup        GSM GSM AT lib
 * \brief           GSM stack
 * \{
 */
 
/* Setup config file */
#include "gsm_config.h"

#include "gsm/gsm_typedefs.h"
#include "gsm/gsm_buff.h"
#include "gsm/gsm_input.h"
#include "system/gsm_sys.h"
#include "gsm/gsm_debug.h"
#include "gsm/gsm_utilities.h"
#include "gsm/gsm_pbuf.h"
#include "gsm/gsm_conn.h"
#if GSM_CFG_MODE_STATION
#include "gsm/gsm_sta.h"
#endif /* GSM_CFG_MODE_STATION */
#if GSM_CFG_MODE_ACCESS_POINT
#include "gsm/gsm_ap.h"
#endif /* GSM_CFG_MODE_ACCESS_POINT */
#if GSM_CFG_OS
#include "gsm/gsm_netconn.h"
#endif /* GSM_CFG_OS */
#if GSM_CFG_PING
#include "gsm/gsm_ping.h"
#endif /* GSM_CFG_PING */
#if GSM_CFG_WPS
#include "gsm/gsm_wps.h"
#endif /* GSM_CFG_WPS */
#if GSM_CFG_SNTP
#include "gsm/gsm_sntp.h"
#endif /* GSM_CFG_SNTP */
#if GSM_CFG_HOSTNAME
#include "gsm/gsm_hostname.h"
#endif /* GSM_CFG_HOSTNAME */

gsmr_t      gsm_init(gsm_cb_fn cb_func);
gsmr_t      gsm_reset(uint32_t blocking);
gsmr_t      gsm_set_at_baudrate(uint32_t baud, uint32_t blocking);
gsmr_t      gsm_set_wifi_mode(gsm_mode_t mode, uint32_t blocking);
gsmr_t      gsm_set_mux(uint8_t mux, uint32_t blocking);

gsmr_t      gsm_set_server(gsm_port_t port, uint16_t max_conn, uint16_t timeout, gsm_cb_fn cb, uint32_t blocking);

gsmr_t      gsm_dns_getbyhostname(const char* host, gsm_ip_t* ip, uint32_t blocking);

gsmr_t      gsm_core_lock(void);
gsmr_t      gsm_core_unlock(void);

gsmr_t      gsm_cb_register(gsm_cb_fn cb_fn);
gsmr_t      gsm_cb_unregister(gsm_cb_fn cb_fn);

void        gsm_delay(uint32_t ms);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GSM_H */
