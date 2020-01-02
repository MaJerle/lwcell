/**
 * \file            gsm_operator.h
 * \brief           Operator API
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
 * This file is part of GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         $_version_$
 */
#ifndef GSM_HDR_OPERATOR_H
#define GSM_HDR_OPERATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gsm/gsm.h"

/**
 * \ingroup         GSM
 * \defgroup        GSM_OPERATOR Network operator API
 * \brief           network operator API
 * \{
 */

gsmr_t      gsm_operator_get(gsm_operator_curr_t* curr, const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
gsmr_t      gsm_operator_set(gsm_operator_mode_t mode, gsm_operator_format_t format, const char* name, uint32_t num, const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);

gsmr_t      gsm_operator_scan(gsm_operator_t* ops, size_t opsl, size_t* opf, const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif

#endif /* GSM_HDR_OPERATOR_H */
