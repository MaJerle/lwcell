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
 * This file is part of LwGSM - Lightweight GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.0
 */
#ifndef LWGSM_HDR_PBUF_H
#define LWGSM_HDR_PBUF_H

#include "lwgsm/lwgsm.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \ingroup         LWGSM
 * \defgroup        LWGSM_PBUF Packet buffer
 * \brief           Packet buffer manager
 * \{
 */

lwgsm_pbuf_p      lwgsm_pbuf_new(size_t len);
size_t          lwgsm_pbuf_free(lwgsm_pbuf_p pbuf);
void*           lwgsm_pbuf_data(const lwgsm_pbuf_p pbuf);
size_t          lwgsm_pbuf_length(const lwgsm_pbuf_p pbuf, uint8_t tot);
lwgsmr_t          lwgsm_pbuf_take(lwgsm_pbuf_p pbuf, const void* data, size_t len, size_t offset);
size_t          lwgsm_pbuf_copy(lwgsm_pbuf_p pbuf, void* data, size_t len, size_t offset);

lwgsmr_t          lwgsm_pbuf_cat(lwgsm_pbuf_p head, const lwgsm_pbuf_p tail);
lwgsmr_t          lwgsm_pbuf_chain(lwgsm_pbuf_p head, lwgsm_pbuf_p tail);
lwgsmr_t          lwgsm_pbuf_ref(lwgsm_pbuf_p pbuf);

uint8_t         lwgsm_pbuf_get_at(const lwgsm_pbuf_p pbuf, size_t pos, uint8_t* el);
size_t          lwgsm_pbuf_memcmp(const lwgsm_pbuf_p pbuf, const void* data, size_t len, size_t offset);
size_t          lwgsm_pbuf_strcmp(const lwgsm_pbuf_p pbuf, const char* str, size_t offset);
size_t          lwgsm_pbuf_memfind(const lwgsm_pbuf_p pbuf, const void* data, size_t len, size_t off);
size_t          lwgsm_pbuf_strfind(const lwgsm_pbuf_p pbuf, const char* str, size_t off);

uint8_t         lwgsm_pbuf_advance(lwgsm_pbuf_p pbuf, int len);
lwgsm_pbuf_p      lwgsm_pbuf_skip(lwgsm_pbuf_p pbuf, size_t offset, size_t* new_offset);

void*           lwgsm_pbuf_get_linear_addr(const lwgsm_pbuf_p pbuf, size_t offset, size_t* new_len);

void            lwgsm_pbuf_set_ip(lwgsm_pbuf_p pbuf, const lwgsm_ip_t* ip, lwgsm_port_t port);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWGSM_HDR_PBUF_H */
