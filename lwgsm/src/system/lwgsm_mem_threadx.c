/**
 * \file            lwcell_mem_lwmem.c
 * \brief           Dynamic memory manager implemented with LwMEM
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
 * This file is part of LwCELL - Lightweight GSM-AT parser library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.1
 */
#include "lwcell/lwcell_types.h"
#include "tx_api.h"

#if !__DOXYGEN__

/* Byte pool is defined externally, in the lwcell_sys_threadx.c file */
extern TX_BYTE_POOL* lwcell_threadx_byte_pool;

void*
lwcell_mem_malloc(size_t size) {
    void* pointer;
    return tx_byte_allocate(lwcell_threadx_byte_pool, &pointer, size, TX_NO_WAIT) == TX_SUCCESS ? pointer : NULL;
}

void*
lwcell_mem_realloc(void* ptr, size_t size) {
    /* No need to support it */
    return NULL;
}

void*
lwcell_mem_calloc(size_t num, size_t size) {
    size_t total = num * size;
    void* pointer = lwcell_mem_malloc(total);
    TX_MEMSET(pointer, 0, total);
    return pointer;
}

void
lwcell_mem_free(void* ptr) {
    (VOID) tx_byte_release(ptr);
}

#endif /* !__DOXYGEN__ */
