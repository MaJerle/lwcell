/**
 * \file            lwcell_memories.h
 * \brief           Supported GSM device memories
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

LWCELL_DEV_MEMORY_ENTRY(SM_P, "SM_P")
LWCELL_DEV_MEMORY_ENTRY(ME_P, "ME_P")
LWCELL_DEV_MEMORY_ENTRY(SM, "SM")
LWCELL_DEV_MEMORY_ENTRY(ME, "ME")
LWCELL_DEV_MEMORY_ENTRY(MT, "MT")
LWCELL_DEV_MEMORY_ENTRY(BM, "BM")
LWCELL_DEV_MEMORY_ENTRY(SR, "SR")
LWCELL_DEV_MEMORY_ENTRY(ON, "ON")
LWCELL_DEV_MEMORY_ENTRY(FD, "FD")

#undef LWCELL_DEV_MEMORY_ENTRY