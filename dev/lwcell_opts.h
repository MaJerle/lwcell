/**
 * \file            lwcell_opts.h
 * \brief           GSM application options
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
 * This file is part of LwCELL.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.0
 */
#ifndef LWCELL_HDR_OPTS_H
#define LWCELL_HDR_OPTS_H

/* Rename this file to "lwcell_opts.h" for your application */

/*
 * Open "include/lwcell/lwcell_opt.h" and
 * copy & replace here settings you want to change values
 */
#if !__DOXYGEN__
#define LWCELL_CFG_DBG               LWCELL_DBG_ON
#define LWCELL_CFG_DBG_TYPES_ON      LWCELL_DBG_TYPE_TRACE | LWCELL_DBG_TYPE_STATE
#define LWCELL_CFG_DBG_MQTT          LWCELL_DBG_OFF

#define LWCELL_CFG_CONN_MAX_DATA_LEN 1460
#define LWCELL_CFG_INPUT_USE_PROCESS 1
#define LWCELL_CFG_AT_ECHO           0

#define LWCELL_CFG_NETWORK           1

#define LWCELL_CFG_CONN              1
#define LWCELL_CFG_SMS               1
#define LWCELL_CFG_CALL              1
#define LWCELL_CFG_PHONEBOOK         1
#define LWCELL_CFG_USSD              1

#define LWCELL_CFG_USE_API_FUNC_EVT  1

#define LWCELL_CFG_NETCONN           1

#define LWCELL_CFG_MEM_CUSTOM        1

#define LWCELL_CFG_MQTT              1

#if defined(WIN32)
#define LWCELL_CFG_SYS_PORT LWCELL_SYS_PORT_WIN32
#endif

#endif /* !__DOXYGEN__ */

#endif /* LWCELL_HDR_OPTS_H */
