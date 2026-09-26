/**
 * \file            lwcell_gnss.h
 * \brief           GNSS/GPS API
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
 *                  Harald LESCHNER <picohari@googlemail.com>
 * Version:         v0.1.2
 */
#ifndef LWCELL_GNSS_HDR_H
#define LWCELL_GNSS_HDR_H

#include "lwcell/lwcell_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \ingroup         LWCELL
 * \defgroup        LWCELL_GNSS GPS API
 * \brief           GNSS manager
 * \{
 */

/* Base commands */
lwcellr_t lwcell_gnss_enable(const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
lwcellr_t lwcell_gnss_disable(const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);

/* GNSS commands */
lwcellr_t lwcell_gnss_info(const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
uint8_t lwcell_gnss_get_time(struct tm* dt, uint16_t* ms, uint8_t* fix_status);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWCELL_GNSS_HDR_H */
