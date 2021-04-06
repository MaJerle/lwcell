/**
 * \file            lwgsm_fs.c
 * \brief           File System functions
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
 * Authors:          Tilen MAJERLE <tilen@majerle.eu>,
 *                   Ilya Kargapolov <d3vil.st@gmail.com>
 * Version:         v0.1.0
 */

#include "lwgsm/lwgsm_private.h"
#include "lwgsm/lwgsm_mem.h"

#if LWGSM_CFG_SSL || __DOXYGEN__

/**
 * \brief           Set SSL options
 * \param[in]       param: Option to set (0,1)
 * \param[in]       value: Value to set (0,1)
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_ssl_opt(uint8_t param, uint8_t value, const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
  LWGSM_MSG_VAR_DEFINE(msg);

  LWGSM_MSG_VAR_ALLOC(msg, blocking);
  LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
  LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_SSL_OPT;
  LWGSM_MSG_VAR_REF(msg).msg.ssl.opt.param = param;
  LWGSM_MSG_VAR_REF(msg).msg.ssl.opt.value = value;

  return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 10000);
}

/**
 * \brief           Import Root CA to a modem
 * \param[in]       path: path on a modem file system to Root CA (Binary format)
 * \param[in]       length: length of file to import
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_ssl_setrootca(char* path, uint16_t length, const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
  LWGSM_MSG_VAR_DEFINE(msg);

  LWGSM_MSG_VAR_ALLOC(msg, blocking);
  LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
  LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_SSL_SETROOT;
  LWGSM_MSG_VAR_REF(msg).msg.ssl.ca_path = path;
  LWGSM_MSG_VAR_REF(msg).msg.ssl.ca_length = length;

  return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 10000);
}

#endif /* LWGSM_CFG_SSL || __DOXYGEN__ */