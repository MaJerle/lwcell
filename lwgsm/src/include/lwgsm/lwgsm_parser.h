/**
 * \file            lwgsm_parser.h
 * \brief           Parser of AT responses
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
#ifndef LWGSM_HDR_PARSER_H
#define LWGSM_HDR_PARSER_H

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

#include "lwgsm/lwgsm.h"

int32_t     lwgsmi_parse_number(const char** str);
uint8_t     lwgsmi_parse_string(const char** src, char* dst, size_t dst_len, uint8_t trim);
uint8_t     lwgsmi_parse_ip(const char** src, lwgsm_ip_t* ip);
uint8_t     lwgsmi_parse_mac(const char** src, lwgsm_mac_t* mac);

uint8_t     lwgsmi_parse_cpin(const char* str, uint8_t send_evt);
uint8_t     lwgsmi_parse_creg(const char* str, uint8_t skip_first);
uint8_t     lwgsmi_parse_csq(const char* str);

uint8_t     lwgsmi_parse_cmgs(const char* str, size_t* num);
uint8_t     lwgsmi_parse_cmti(const char* str, uint8_t send_evt);
uint8_t     lwgsmi_parse_cmgr(const char* str);
uint8_t     lwgsmi_parse_cmgl(const char* str);

uint8_t     lwgsmi_parse_at_sdk_version(const char* str, uint32_t* version_out);

uint8_t     lwgsmi_parse_cops_scan(uint8_t ch, uint8_t reset);
uint8_t     lwgsmi_parse_cops(const char* str);
uint8_t     lwgsmi_parse_clcc(const char* str, uint8_t send_evt);

uint8_t     lwgsmi_parse_cpbs(const char* str, uint8_t opt);
uint8_t     lwgsmi_parse_cpms(const char* str, uint8_t opt);
uint8_t     lwgsmi_parse_cpbr(const char* str);
uint8_t     lwgsmi_parse_cpbf(const char* str);

uint8_t     lwgsmi_parse_cipstatus_conn(const char* str, uint8_t is_conn_line, uint8_t* continueScan);

uint8_t     lwgsmi_parse_ipd(const char* str);

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif /* LWGSM_HDR_PARSER_H */
