/**
 * \file            lwcell_call.h
 * \brief           Call API
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
 * This file is part of LwCELL - Lightweight cellular modem AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.1
 */
#ifndef LWCELL_CALL_HDR_H
#define LWCELL_CALL_HDR_H

#include "lwcell/lwcell_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \ingroup         LWCELL
 * \defgroup        LWCELL_CALL Call API
 * \brief           Call manager
 * \{
 */

lwcellr_t lwcell_call_enable(const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
lwcellr_t lwcell_call_disable(const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);

lwcellr_t lwcell_call_start(const char* number, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                          const uint32_t blocking);
lwcellr_t lwcell_call_answer(const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
lwcellr_t lwcell_call_hangup(const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWCELL_CALL_HDR_H */
