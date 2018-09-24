/**	
 * \file            gsm_sim.h
 * \brief           SIM API
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
 * This file is part of GSM-AT.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 */
#ifndef __GSM_SIM_H
#define __GSM_SIM_H

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif

#include "gsm/gsm.h"

/**
 * \ingroup         GSM
 * \defgroup        GSM_SIM SIM API
 * \brief           SIM card manager
 * \{
 */

gsmr_t      gsm_sim_pin_enter(const char* pin, uint32_t blocking);
gsmr_t      gsm_sim_pin_add(const char* pin, uint32_t blocking);
gsmr_t      gsm_sim_pin_remove(const char* pin, uint32_t blocking);
gsmr_t      gsm_sim_pin_change(const char* pin, const char* new_pin, uint32_t blocking);
gsmr_t      gsm_sim_puk_enter(const char* puk, const char* new_pin, uint32_t blocking);

/**
 * \}
 */

/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif /* __GSM_SIM_H */
