/**
 * \file            gsm_int_device.c
 * \brief           Internal functions for device management
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
#include "gsm/gsm_private.h"
#include "gsm/gsm.h"

#if GSM_CFG_SMS || __DOXYGEN__

uint8_t
gsmi_device_set_sms_ready(uint8_t ready) {
    gsm.status.f.sms_ready = !!ready;           /* SMS ready flag */
    gsmi_send_cb(GSM_CB_SMS_READY);             /* Send SMS ready event */
    return 1;
}

#endif /* GSM_CFG_SMS || __DOXYGEN__ */

#if GSM_CFG_CALL || __DOXYGEN__

uint8_t
gsmi_device_set_call_ready(uint8_t ready) {
    gsm.status.f.call_ready = !!ready;          /* Call ready flag */
    gsmi_send_cb(GSM_CB_CALL_READY);            /* Send call ready event */
    return 1;
}

#endif /* GSM_CFG_CALL || __DOXYGEN__ */
