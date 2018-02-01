/**
 * \file            gsm_parser.c
 * \brief           Parse incoming data from AT port
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
#include "gsm/gsm_parser.h"
#include "gsm/gsm_mem.h"

/**
 * \brief           Parse number from string
 * \note            Input string pointer is changed and number is skipped
 * \param[in]       Pointer to pointer to string to parse
 * \return          Parsed number
 */
int32_t
gsmi_parse_number(const char** str) {
    int32_t val = 0;
    uint8_t minus = 0;
    const char* p = *str;                       /*  */
    
    if (*p == '"') {                            /* Skip leading quotes */
        p++;
    }
    if (*p == ',') {                            /* Skip leading comma */
        p++;
    }
    if (*p == '"') {                            /* Skip leading quotes */
        p++;
    }
    if (*p == '-') {                            /* Check negative number */
        minus = 1;
        p++;
    }
    while (GSM_CHARISNUM(*p)) {                 /* Parse until character is valid number */
        val = val * 10 + GSM_CHARTONUM(*p);
        p++;
    }
    if (*p == ',') {                            /* Go to next entry if possible */
        p++;
    }
    *str = p;                                   /* Save new pointer with new offset */
    
    return minus ? -val : val;
}

/**
 * \brief           Parse number from string as hex
 * \note            Input string pointer is changed and number is skipped
 * \param[in]       Pointer to pointer to string to parse
 * \return          Parsed number
 */
uint32_t
gsmi_parse_hexnumber(const char** str) {
    int32_t val = 0;
    const char* p = *str;                       /*  */
    
    if (*p == '"') {                            /* Skip leading quotes */
        p++;
    }
    if (*p == ',') {                            /* Skip leading comma */
        p++;
    }
    if (*p == '"') {                            /* Skip leading quotes */
        p++;
    }
    while (GSM_CHARISHEXNUM(*p)) {              /* Parse until character is valid number */
        val = val * 16 + GSM_CHARHEXTONUM(*p);
        p++;
    }
    if (*p == ',') {                            /* Go to next entry if possible */
        p++;
    }
    *str = p;                                   /* Save new pointer with new offset */
    return val;
}

/**
 * \brief           Parse input string as string part of AT command
 * \param[in,out]   src: Pointer to pointer to string to parse from
 * \param[in]       dst: Destination pointer. Use NULL in case you want only skip string in source
 * \param[in]       dst_len: Length of distance buffer, including memory for NULL termination
 * \param[in]       trim: Set to 1 to process entire string, even if no memory anymore
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_parse_string(const char** src, char* dst, size_t dst_len, uint8_t trim) {
    const char* p = *src;
    size_t i;
    
    if (*p == ',') {
        p++;
    }
    if (*p == '"') {
        p++;
    }
    i = 0;
    if (dst_len) {
        dst_len--;
    }
    while (*p) {
        if (*p == '"' && (p[1] == ',' || p[1] == '\r' || p[1] == '\n')) {
            p++;
            break;
        }
        if (dst) {
            if (i < dst_len) {
                *dst++ = *p;
                i++;
            } else if (!trim) {
                break;
            }
        }
        p++;
    }
    if (dst) {
        *dst = 0;
    }
    *src = p;
    return 1;
}

/**
 * \brief           Parse string as IP address
 * \param[in,out]   src: Pointer to pointer to string to parse from
 * \param[in]       dst: Destination pointer
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_parse_ip(const char** src, gsm_ip_t* ip) {
    const char* p = *src;
    
    if (*p == '"') {
        p++;
    }
    ip->ip[0] = gsmi_parse_number(&p); p++;
    ip->ip[1] = gsmi_parse_number(&p); p++;
    ip->ip[2] = gsmi_parse_number(&p); p++;
    ip->ip[3] = gsmi_parse_number(&p);
    if (*p == '"') {
        p++;
    }
    
    *src = p;                                   /* Set new pointer */
    return 1;
}

/**
 * \brief           Parse string as MAC address
 * \param[in,out]   src: Pointer to pointer to string to parse from
 * \param[in]       dst: Destination pointer
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_parse_mac(const char** src, gsm_mac_t* mac) {
    const char* p = *src;
    
    if (*p == '"') {                            /* Go to next entry if possible */
        p++;
    }
    mac->mac[0] = gsmi_parse_hexnumber(&p); p++;
    mac->mac[1] = gsmi_parse_hexnumber(&p); p++;
    mac->mac[2] = gsmi_parse_hexnumber(&p); p++;
    mac->mac[3] = gsmi_parse_hexnumber(&p); p++;
    mac->mac[4] = gsmi_parse_hexnumber(&p); p++;
    mac->mac[5] = gsmi_parse_hexnumber(&p);
    if (*p == '"') {                            /* Skip quotes if possible */
        p++;
    }
    if (*p == ',') {                            /* Go to next entry if possible */
        p++;
    }
    *src = p;                                   /* Set new pointer */
    return 1;
}
