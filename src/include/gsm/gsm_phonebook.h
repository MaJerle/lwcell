/**	
 * \file            gsm_phonebook.h
 * \brief           Phonebook API
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
#ifndef __GSM_PHONEBOOK_H
#define __GSM_PHONEBOOK_H

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif

#include "gsm/gsm.h"

/**
 * \ingroup         GSM
 * \defgroup        GSM_PHONEBOOK Phonebook API
 * \brief           Phonebook manager
 * \{
 */

gsmr_t      gsm_pb_enable(const uint32_t blocking);
gsmr_t      gsm_pb_disable(const uint32_t blocking);

gsmr_t      gsm_pb_add(gsm_mem_t mem, const char* name, const char* num, gsm_number_type_t type, const uint32_t blocking);
gsmr_t      gsm_pb_edit(gsm_mem_t mem, size_t pos, const char* name, const char* num, gsm_number_type_t type, const uint32_t blocking);
gsmr_t      gsm_pb_delete(gsm_mem_t mem, size_t pos, const uint32_t blocking);
gsmr_t      gsm_pb_read(gsm_mem_t mem, size_t pos, gsm_pb_entry_t* entry, const uint32_t blocking);
gsmr_t      gsm_pb_list(gsm_mem_t mem, size_t start_index, gsm_pb_entry_t* entries, size_t etr, size_t* er, const uint32_t blocking);
gsmr_t      gsm_pb_search(gsm_mem_t mem, const char* search, gsm_pb_entry_t* entries, size_t etr, size_t* er, const uint32_t blocking);

/**
 * \}
 */

/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif /* __GSM_PHONEBOOK_H */
