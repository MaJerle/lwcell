/**
 * \file            lwgsm_ip_app.c
 * \brief           IP Application functions
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

#if LWGSM_CFG_IP_APP || __DOXYGEN__

/**
 * \brief           IP Application control
 * \param[in]       param: Type of request
 * \param[in]       value: Value for request
 * \param[out]      status: if type of request is to, pointer to return current status. Set to `NULL` when not used
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_ip_app_sapbr(uint8_t param, uint8_t value, void* status, const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
  LWGSM_MSG_VAR_DEFINE(msg);

  LWGSM_MSG_VAR_ALLOC(msg, blocking);
  LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
  LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_IP_APP_SAPBR;
  LWGSM_MSG_VAR_REF(msg).msg.ip_app.sapbr.param = param;
  LWGSM_MSG_VAR_REF(msg).msg.ip_app.sapbr.value = value;
  LWGSM_MSG_VAR_REF(msg).msg.ip_app.status = (ip_app_t*) status;

  return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 85000);
}

#endif /* LWGSM_CFG_IP_APP || __DOXYGEN__ */