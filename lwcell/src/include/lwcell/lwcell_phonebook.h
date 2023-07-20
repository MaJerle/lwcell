/**
 * \file            lwcell_phonebook.h
 * \brief           Phonebook API
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
 * This file is part of LwCELL - Lightweight cellular modem AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.1
 */
#ifndef LWCELL_PHONEBOOK_HDR_H
#define LWCELL_PHONEBOOK_HDR_H

#include "lwcell/lwcell_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \ingroup         LWCELL
 * \defgroup        LWCELL_PHONEBOOK Phonebook API
 * \brief           Phonebook manager
 * \{
 */

lwcellr_t lwcell_pb_enable(const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
lwcellr_t lwcell_pb_disable(const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);

lwcellr_t lwcell_pb_add(lwcell_mem_t mem, const char* name, const char* num, lwcell_number_type_t type,
                      const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
lwcellr_t lwcell_pb_edit(lwcell_mem_t mem, size_t pos, const char* name, const char* num, lwcell_number_type_t type,
                       const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
lwcellr_t lwcell_pb_delete(lwcell_mem_t mem, size_t pos, const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg,
                         const uint32_t blocking);
lwcellr_t lwcell_pb_read(lwcell_mem_t mem, size_t pos, lwcell_pb_entry_t* entry, const lwcell_api_cmd_evt_fn evt_fn,
                       void* const evt_arg, const uint32_t blocking);
lwcellr_t lwcell_pb_list(lwcell_mem_t mem, size_t start_index, lwcell_pb_entry_t* entries, size_t etr, size_t* er,
                       const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
lwcellr_t lwcell_pb_search(lwcell_mem_t mem, const char* search, lwcell_pb_entry_t* entries, size_t etr, size_t* er,
                         const lwcell_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWCELL_PHONEBOOK_HDR_H */
