/**
 * \file            lwgsm_includes.h
 * \brief           All main includes
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
#ifndef LWGSM_HDR_INCLUDES_H
#define LWGSM_HDR_INCLUDES_H

#include "lwgsm/lwgsm_opt.h"
#include "lwgsm/lwgsm_typedefs.h"
#include "lwgsm/lwgsm_buff.h"
#include "lwgsm/lwgsm_input.h"
#include "lwgsm/lwgsm_debug.h"
#include "lwgsm/lwgsm_utils.h"
#include "lwgsm/lwgsm_pbuf.h"
#include "lwgsm/lwgsm_sim.h"
#include "lwgsm/lwgsm_operator.h"
#include "lwgsm/lwgsm_evt.h"
#include "lwgsm/lwgsm_network.h"
#include "lwgsm/lwgsm_device_info.h"
#include "system/lwgsm_sys.h"

#if LWGSM_CFG_SMS || __DOXYGEN__
#include "lwgsm/lwgsm_sms.h"
#endif /* LWGSM_CFG_SMS || __DOXYGEN__ */
#if LWGSM_CFG_CALL || __DOXYGEN__
#include "lwgsm/lwgsm_call.h"
#endif /* LWGSM_CFG_CALL || __DOXYGEN__ */
#if LWGSM_CFG_PHONEBOOK || __DOXYGEN__
#include "lwgsm/lwgsm_phonebook.h"
#endif /* LWGSM_CFG_PHONEBOOK || __DOXYGEN__ */
#if LWGSM_CFG_CONN || __DOXYGEN__
#include "lwgsm/lwgsm_conn.h"
#endif /* LWGSM_CFG_CONN || __DOXYGEN__ */
#if LWGSM_CFG_NETCONN || __DOXYGEN__
#include "lwgsm/lwgsm_netconn.h"
#endif /* LWGSM_CFG_NETCONN || __DOXYGEN__ */
#if LWGSM_CFG_USSD || __DOXYGEN__
#include "lwgsm/lwgsm_ussd.h"
#endif /* LWGSM_CFG_USSD || __DOXYGEN__ */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWGSM_HDR_INCLUDES_H */
