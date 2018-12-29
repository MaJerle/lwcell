/**
 * \file            gsm_includes.h
 * \brief           All main includes
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
#ifndef __GSM_INCLUDES_H
#define __GSM_INCLUDES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "gsm_config.h"
#include "gsm/gsm_typedefs.h"
#include "gsm/gsm_buff.h"
#include "gsm/gsm_input.h"
#include "gsm/gsm_debug.h"
#include "gsm/gsm_utilities.h"
#include "gsm/gsm_pbuf.h"
#include "gsm/gsm_sim.h"
#include "gsm/gsm_operator.h"
#include "gsm/gsm_evt.h"
#include "gsm/gsm_network.h"

#include "system/gsm_sys.h"

#if GSM_CFG_SMS || __DOXYGEN__
#include "gsm/gsm_sms.h"
#endif /* GSM_CFG_SMS */
#if GSM_CFG_CALL || __DOXYGEN__
#include "gsm/gsm_call.h"
#endif /* GSM_CFG_CALL */
#if GSM_CFG_PHONEBOOK || __DOXYGEN__
#include "gsm/gsm_phonebook.h"
#endif /* GSM_CFG_PHONEBOOK */
#if GSM_CFG_CONN || __DOXYGEN__
#include "gsm/gsm_conn.h"
#endif /* GSM_CFG_CONN */
#if GSM_CFG_NETCONN || __DOXYGEN__
#include "gsm/gsm_netconn.h"
#endif /* GSM_CFG_NETCONN */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GSM_INCLUDES_H */
