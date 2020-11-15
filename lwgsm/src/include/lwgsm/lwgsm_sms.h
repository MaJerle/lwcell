/**
 * \file            lwgsm_sms.h
 * \brief           SMS API
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
#ifndef LWGSM_HDR_SMS_H
#define LWGSM_HDR_SMS_H

#include "lwgsm/lwgsm.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \ingroup         LWGSM
 * \defgroup        LWGSM_SMS SMS API
 * \brief           SMS manager
 * \{
 */

lwgsmr_t      lwgsm_sms_enable(const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
lwgsmr_t      lwgsm_sms_disable(const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);

lwgsmr_t      lwgsm_sms_send(const char* num, const char* text, const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
lwgsmr_t      lwgsm_sms_read(lwgsm_mem_t mem, size_t pos, lwgsm_sms_entry_t* entry, uint8_t update, const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
lwgsmr_t      lwgsm_sms_delete(lwgsm_mem_t mem, size_t pos, const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
lwgsmr_t      lwgsm_sms_delete_all(lwgsm_sms_status_t status, const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
lwgsmr_t      lwgsm_sms_list(lwgsm_mem_t mem, lwgsm_sms_status_t stat, lwgsm_sms_entry_t* entries, size_t etr, size_t* er, uint8_t update, const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg,
                         const uint32_t blocking);
lwgsmr_t      lwgsm_sms_set_preferred_storage(lwgsm_mem_t mem1, lwgsm_mem_t mem2, lwgsm_mem_t mem3, const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWGSM_HDR_SMS_H */
