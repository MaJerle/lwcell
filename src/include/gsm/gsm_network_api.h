/**
 * \file            gsm_network_api.h
 * \brief           API functions for multi-thread network functions
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
#ifndef GSM_HDR_NETWORK_API_H
#define GSM_HDR_NETWORK_API_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "gsm/gsm.h"

/**
 * \ingroup         GSM_API
 * \defgroup        GSM_NETWORK_API Network API
 * \brief           Network API functions for multi-thread operations
 * \{
 * 
 * Its purpose is to control network attach status when multiple threads needs
 * to enable/disable network connection.
 *
 * Each thread asks to attach network connection and manager
 * controls when it is necessary to connect or disconnect.
 *
 */

gsmr_t  gsm_network_set_credentials(const char* apn, const char* user, const char* pass);
gsmr_t  gsm_network_request_attach(void);
gsmr_t  gsm_network_request_detach(void);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GSM_HDR_NETWORK_API_H */
