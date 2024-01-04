/**
 * \file            lwcell_device_info.h
 * \brief           Basic device information
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
#ifndef LWCELL_DEVICE_INFO_HDR_H
#define LWCELL_DEVICE_INFO_HDR_H

#include "lwcell/lwcell_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \ingroup         LWCELL
 * \defgroup        LWCELL_DEVICE_INFO Device information
 * \brief           Basic device information
 * \{
 */

lwcellr_t lwcell_device_get_manufacturer(char* manuf, size_t len, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                                       const uint32_t blocking);
lwcellr_t lwcell_device_get_model(char* model, size_t len, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                                const uint32_t blocking);
lwcellr_t lwcell_device_get_revision(char* rev, size_t len, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                                   const uint32_t blocking);
lwcellr_t lwcell_device_get_serial_number(char* serial, size_t len, const lwcell_api_cmd_evt_fn evt_fn,
                                        void* const evt_arg, const uint32_t blocking);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWCELL_DEVICE_INFO_HDR_H */
