/**
 * \file            lwcell_operator.h
 * \brief           Operator API
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
#ifndef LWCELL_OPERATOR_HDR_H
#define LWCELL_OPERATOR_HDR_H

#include "lwcell/lwcell_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \ingroup         LWCELL
 * \defgroup        LWCELL_OPERATOR Network operator API
 * \brief           network operator API
 * \{
 */

lwcellr_t lwcell_operator_get(lwcell_operator_curr_t* curr, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                              const uint32_t blocking);
lwcellr_t lwcell_operator_set(lwcell_operator_mode_t mode, lwcell_operator_format_t format, const char* name,
                              uint32_t num, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                              const uint32_t blocking);

lwcellr_t lwcell_operator_scan(lwcell_operator_t* ops, size_t opsl, size_t* opf, const lwcell_api_cmd_evt_fn evt_fn,
                               void* const evt_arg, const uint32_t blocking);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWCELL_OPERATOR_HDR_H */
