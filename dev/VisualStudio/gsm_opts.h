/**
 * \file            gsm_opts.h
 * \brief           GSM application options
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
 * This file is part of GSM-AT.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         $_version_$
 */
#ifndef GSM_HDR_OPTS_H
#define GSM_HDR_OPTS_H

/* Rename this file to "gsm_opts.h" for your application */

/*
 * Open "include/gsm/gsm_opt.h" and
 * copy & replace here settings you want to change values
 */
#if !__DOXYGEN__
#define GSM_CFG_DBG                         GSM_DBG_ON
#define GSM_CFG_DBG_TYPES_ON                GSM_DBG_TYPE_TRACE | GSM_DBG_TYPE_STATE
#define GSM_CFG_DBG_MQTT                    GSM_DBG_OFF

#define GSM_CFG_IPD_MAX_BUFF_SIZE           1460
#define GSM_CFG_INPUT_USE_PROCESS           1
#define GSM_CFG_AT_ECHO                     0

#define GSM_CFG_NETWORK                     1

#define GSM_CFG_CONN                        1
#define GSM_CFG_SMS                         1
#define GSM_CFG_CALL                        1
#define GSM_CFG_PHONEBOOK                   1
#define GSM_CFG_USSD                        1

#define GSM_CFG_USE_API_FUNC_EVT            1

#define GSM_CFG_NETCONN                     1

#define GSM_CFG_MEM_CUSTOM                  1

#if defined(WIN32)
#define GSM_CFG_SYS_PORT					GSM_SYS_PORT_WIN32
#endif

#endif /* !__DOXYGEN__ */

#endif /* GSM_HDR_OPTS_H */
