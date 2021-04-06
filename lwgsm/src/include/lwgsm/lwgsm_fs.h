/**
 * \file            lwgsm_fs.h
 * \brief           File System API
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

#ifndef LWGSM_HDR_FS_H
#define LWGSM_HDR_FS_H

lwgsmr_t      lwgsm_fs_create(const char* path, lwgsm_api_cmd_evt_fn evt_fn, void* evt_arg, uint32_t blocking);
lwgsmr_t      lwgsm_fs_delete(const char* path, lwgsm_api_cmd_evt_fn evt_fn, void* evt_arg, uint32_t blocking);
lwgsmr_t      lwgsm_fs_write(const char* path, uint8_t mode, uint16_t size, const char* content, uint8_t input_time,
                              lwgsm_api_cmd_evt_fn evt_fn, void* evt_arg, uint32_t blocking);
#endif /* LWGSM_HDR_FS_H */
