/**
 * \file            gsm_mem.h
 * \brief           Memory manager
 */

/*
 * Copyright (c) 2019 Tilen MAJERLE
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
#ifndef GSM_HDR_MEM_H
#define GSM_HDR_MEM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "gsm/gsm.h"

/**
 * \ingroup         GSM
 * \defgroup        GSM_MEM Memory manager
 * \brief           Dynamic memory manager
 * \{
 */

#if !GSM_CFG_MEM_CUSTOM || __DOXYGEN__

/**
 * \brief           Single memory region descriptor
 */
typedef struct {
    void* start_addr;                           /*!< Start address of region */
    size_t size;                                /*!< Size in units of bytes of region */
} gsm_mem_region_t;

uint8_t gsm_mem_assignmemory(const gsm_mem_region_t* regions, size_t size);

#endif /* !GSM_CFG_MEM_CUSTOM || __DOXYGEN__ */

void*   gsm_mem_malloc(size_t size);
void*   gsm_mem_realloc(void* ptr, size_t size);
void*   gsm_mem_calloc(size_t num, size_t size);
void    gsm_mem_free(void* ptr);
uint8_t gsm_mem_free_s(void** ptr);


/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GSM_HDR_MEM_H */
