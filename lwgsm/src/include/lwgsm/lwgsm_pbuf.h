/**
 * \file            lwgsm_pbuf.h
 * \brief           Packet buffer manager
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
#ifndef GSM_HDR_PBUF_H
#define GSM_HDR_PBUF_H

#include "lwgsm/lwgsm.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \ingroup         GSM
 * \defgroup        GSM_PBUF Packet buffer
 * \brief           Packet buffer manager
 * \{
 */

gsm_pbuf_p      gsm_pbuf_new(size_t len);
size_t          gsm_pbuf_free(gsm_pbuf_p pbuf);
void*           gsm_pbuf_data(const gsm_pbuf_p pbuf);
size_t          gsm_pbuf_length(const gsm_pbuf_p pbuf, uint8_t tot);
lwgsmr_t          gsm_pbuf_take(gsm_pbuf_p pbuf, const void* data, size_t len, size_t offset);
size_t          gsm_pbuf_copy(gsm_pbuf_p pbuf, void* data, size_t len, size_t offset);

lwgsmr_t          gsm_pbuf_cat(gsm_pbuf_p head, const gsm_pbuf_p tail);
lwgsmr_t          gsm_pbuf_chain(gsm_pbuf_p head, gsm_pbuf_p tail);
lwgsmr_t          gsm_pbuf_ref(gsm_pbuf_p pbuf);

uint8_t         gsm_pbuf_get_at(const gsm_pbuf_p pbuf, size_t pos, uint8_t* el);
size_t          gsm_pbuf_memcmp(const gsm_pbuf_p pbuf, const void* data, size_t len, size_t offset);
size_t          gsm_pbuf_strcmp(const gsm_pbuf_p pbuf, const char* str, size_t offset);
size_t          gsm_pbuf_memfind(const gsm_pbuf_p pbuf, const void* data, size_t len, size_t off);
size_t          gsm_pbuf_strfind(const gsm_pbuf_p pbuf, const char* str, size_t off);

uint8_t         gsm_pbuf_advance(gsm_pbuf_p pbuf, int len);
gsm_pbuf_p      gsm_pbuf_skip(gsm_pbuf_p pbuf, size_t offset, size_t* new_offset);

void*           gsm_pbuf_get_linear_addr(const gsm_pbuf_p pbuf, size_t offset, size_t* new_len);

void            gsm_pbuf_set_ip(gsm_pbuf_p pbuf, const gsm_ip_t* ip, gsm_port_t port);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GSM_HDR_PBUF_H */
