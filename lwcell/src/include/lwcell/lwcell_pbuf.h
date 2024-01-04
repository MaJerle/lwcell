/**
 * \file            lwcell_pbuf.h
 * \brief           Packet buffer manager
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
#ifndef LWCELL_PBUF_HDR_H
#define LWCELL_PBUF_HDR_H

#include "lwcell/lwcell_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \ingroup         LWCELL
 * \defgroup        LWCELL_PBUF Packet buffer
 * \brief           Packet buffer manager
 * \{
 */

lwcell_pbuf_p lwcell_pbuf_new(size_t len);
size_t lwcell_pbuf_free(lwcell_pbuf_p pbuf);
size_t lwcell_pbuf_free_s(lwcell_pbuf_p* pbuf);
void* lwcell_pbuf_data(const lwcell_pbuf_p pbuf);
size_t lwcell_pbuf_length(const lwcell_pbuf_p pbuf, uint8_t tot);
lwcellr_t lwcell_pbuf_take(lwcell_pbuf_p pbuf, const void* data, size_t len, size_t offset);
size_t lwcell_pbuf_copy(lwcell_pbuf_p pbuf, void* data, size_t len, size_t offset);

lwcellr_t lwcell_pbuf_cat(lwcell_pbuf_p head, const lwcell_pbuf_p tail);
lwcellr_t lwcell_pbuf_cat_s(lwcell_pbuf_p head, lwcell_pbuf_p* tail);
lwcellr_t lwcell_pbuf_chain(lwcell_pbuf_p head, lwcell_pbuf_p tail);
lwcellr_t lwcell_pbuf_ref(lwcell_pbuf_p pbuf);

uint8_t lwcell_pbuf_get_at(const lwcell_pbuf_p pbuf, size_t pos, uint8_t* el);
size_t lwcell_pbuf_memcmp(const lwcell_pbuf_p pbuf, const void* data, size_t len, size_t offset);
size_t lwcell_pbuf_strcmp(const lwcell_pbuf_p pbuf, const char* str, size_t offset);
size_t lwcell_pbuf_memfind(const lwcell_pbuf_p pbuf, const void* data, size_t len, size_t off);
size_t lwcell_pbuf_strfind(const lwcell_pbuf_p pbuf, const char* str, size_t off);

uint8_t lwcell_pbuf_advance(lwcell_pbuf_p pbuf, int len);
lwcell_pbuf_p lwcell_pbuf_skip(lwcell_pbuf_p pbuf, size_t offset, size_t* new_offset);

void* lwcell_pbuf_get_linear_addr(const lwcell_pbuf_p pbuf, size_t offset, size_t* new_len);

void lwcell_pbuf_set_ip(lwcell_pbuf_p pbuf, const lwcell_ip_t* ip, lwcell_port_t port);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWCELL_PBUF_HDR_H */
