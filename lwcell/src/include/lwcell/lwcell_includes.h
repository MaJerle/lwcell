/**
 * \file            lwcell_includes.h
 * \brief           All main includes
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
 * Version:         v0.1.1
 */
#ifndef LWCELL_INCLUDES_HDR_H
#define LWCELL_INCLUDES_HDR_H

#include "lwcell/lwcell_buff.h"
#include "lwcell/lwcell_debug.h"
#include "lwcell/lwcell_device_info.h"
#include "lwcell/lwcell_evt.h"
#include "lwcell/lwcell_input.h"
#include "lwcell/lwcell_mem.h"
#include "lwcell/lwcell_network.h"
#include "lwcell/lwcell_operator.h"
#include "lwcell/lwcell_opt.h"
#include "lwcell/lwcell_pbuf.h"
#include "lwcell/lwcell_sim.h"
#include "lwcell/lwcell_types.h"
#include "lwcell/lwcell_utils.h"
#include "system/lwcell_sys.h"

#if LWCELL_CFG_SMS || __DOXYGEN__
#include "lwcell/lwcell_sms.h"
#endif /* LWCELL_CFG_SMS || __DOXYGEN__ */
#if LWCELL_CFG_CALL || __DOXYGEN__
#include "lwcell/lwcell_call.h"
#endif /* LWCELL_CFG_CALL || __DOXYGEN__ */
#if LWCELL_CFG_PHONEBOOK || __DOXYGEN__
#include "lwcell/lwcell_phonebook.h"
#endif /* LWCELL_CFG_PHONEBOOK || __DOXYGEN__ */
#if LWCELL_CFG_CONN || __DOXYGEN__
#include "lwcell/lwcell_conn.h"
#endif /* LWCELL_CFG_CONN || __DOXYGEN__ */
#if LWCELL_CFG_NETCONN || __DOXYGEN__
#include "lwcell/lwcell_netconn.h"
#endif /* LWCELL_CFG_NETCONN || __DOXYGEN__ */
#if LWCELL_CFG_USSD || __DOXYGEN__
#include "lwcell/lwcell_ussd.h"
#endif /* LWCELL_CFG_USSD || __DOXYGEN__ */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWCELL_INCLUDES_HDR_H */
