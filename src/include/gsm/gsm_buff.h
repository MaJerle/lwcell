/**	
 * \file            gsm_buff.h
 * \brief           Buffer manager
 */

/*
 * Copyright (c) 2018 Tilen Majerle
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
 */
#ifndef __GSM_BUFF_H
#define __GSM_BUFF_H

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif

#include "gsm/gsm.h"

/**
 * \ingroup         GSM
 * \defgroup        GSM_BUFF Ring buffer
 * \brief           Generic ring buffer
 * \{
 */

uint8_t     gsm_buff_init(gsm_buff_t* buff, size_t len);
void        gsm_buff_free(gsm_buff_t* buff);
size_t      gsm_buff_write(gsm_buff_t* buff, const void* data, size_t count);
size_t      gsm_buff_read(gsm_buff_t* buff, void* data, size_t count);
size_t      gsm_buff_get_free(gsm_buff_t* buff);
size_t      gsm_buff_get_full(gsm_buff_t* buff);
void        gsm_buff_reset(gsm_buff_t* buff);
size_t      gsm_buff_peek(gsm_buff_t* buff, size_t skip_count, void* data, size_t count);
void *      gsm_buff_get_linear_block_address(gsm_buff_t* buff);
size_t      gsm_buff_get_linear_block_length(gsm_buff_t* buff);
size_t      gsm_buff_skip(gsm_buff_t* buff, size_t len);

/**
 * \}
 */

/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif /* __GSM_BUFF_H */
