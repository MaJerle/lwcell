/**
 * \file            lwcell.h
 * \brief           Lightweight GSM-AT library
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
 * This file is part of LwCELL - Lightweight GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.1
 */
#ifndef LWCELL_HDR_H
#define LWCELL_HDR_H

/* Get most important include files */
#include "lwcell/lwcell_includes.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \defgroup        LWCELL Lightweight GSM-AT parser library
 * \brief           Lightweight GSM-AT parser library
 * \{
 */

lwcellr_t lwcell_init(lwcell_evt_fn evt_func, const uint32_t blocking);
lwcellr_t lwcell_reset(const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
lwcellr_t lwcell_reset_with_delay(uint32_t delay, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                                const uint32_t blocking);

lwcellr_t lwcell_set_func_mode(uint8_t mode, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                             const uint32_t blocking);

lwcellr_t lwcell_core_lock(void);
lwcellr_t lwcell_core_unlock(void);

lwcellr_t lwcell_device_set_present(uint8_t present, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                                  const uint32_t blocking);
uint8_t lwcell_device_is_present(void);

uint8_t lwcell_delay(uint32_t ms);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWCELL_HDR_H */
