/**
 * \file            lwcell_utils.h
 * \brief           Utilities
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
#ifndef LWCELL_UTILS_HDR_H
#define LWCELL_UTILS_HDR_H

#include "lwcell/lwcell_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \ingroup         LWCELL
 * \defgroup        LWCELL_UTILS Utilities
 * \brief           Utilities
 * \{
 */

/**
 * \brief           Assert an input parameter if in valid range
 * \note            Since this is a macro, it may only be used on a functions where return status is of type \ref lwcellr_t enumeration
 * \param[in]       c: Condition to test
 */
#define LWCELL_ASSERT(c)                                                                                                \
    do {                                                                                                               \
        if (!(c)) {                                                                                                    \
            LWCELL_DEBUGF(LWCELL_CFG_DBG_ASSERT, "Assert failed in file %s on line %d: %s\r\n", __FILE__, (int)__LINE__, \
                         #c);                                                                                          \
            return lwcellERRPAR;                                                                                        \
        }                                                                                                              \
    } while (0)

/**
 * \brief           Assert an input parameter if in valid range, return 0 from function on failure
 * \note            Since this is a macro, it may only be used on a functions where return status is of type \ref lwcellr_t enumeration
 * \param[in]       c: Condition to test
 */
#define LWCELL_ASSERT0(c)                                                                                               \
    do {                                                                                                               \
        if (!(c)) {                                                                                                    \
            LWCELL_DEBUGF(LWCELL_CFG_DBG_ASSERT, "Assert failed in file %s on line %d: %s\r\n", __FILE__, (int)__LINE__, \
                         #c);                                                                                          \
            return 0;                                                                                                  \
        }                                                                                                              \
    } while (0)

/**
 * \brief           Align `x` value to specific number of bytes, provided by \ref LWCELL_CFG_MEM_ALIGNMENT configuration
 * \param[in]       x: Input value to align
 * \return          Input value aligned to specific number of bytes
 * \hideinitializer
 */
#define LWCELL_MEM_ALIGN(x)                ((x + (LWCELL_CFG_MEM_ALIGNMENT - 1)) & ~(LWCELL_CFG_MEM_ALIGNMENT - 1))

/**
 * \brief           Get minimal value between `x` and `y` inputs
 * \param[in]       x: First input to test
 * \param[in]       y: Second input to test
 * \return          Minimal value between `x` and `y` parameters
 * \hideinitializer
 */
#define LWCELL_MIN(x, y)                   ((x) < (y) ? (x) : (y))

/**
 * \brief           Get maximal value between `x` and `y` inputs
 * \param[in]       x: First input to test
 * \param[in]       y: Second input to test
 * \return          Maximal value between `x` and `y` parameters
 * \hideinitializer
 */
#define LWCELL_MAX(x, y)                   ((x) > (y) ? (x) : (y))

/**
 * \brief           Get size of statically declared array
 * \param[in]       x: Input array
 * \return          Number of array elements
 * \hideinitializer
 */
#define LWCELL_ARRAYSIZE(x)                (sizeof(x) / sizeof((x)[0]))

/**
 * \brief           Unused argument in a function call
 * \note            Use this on all parameters in a function which are not used to prevent
 *                  compiler warnings complaining about "unused variables"
 * \param[in]       x: Variable which is not used
 * \hideinitializer
 */
#define LWCELL_UNUSED(x)                   ((void)(x))

/**
 * \brief           Get input value casted to `unsigned 32-bit` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define LWCELL_U32(x)                      ((uint32_t)(x))

/**
 * \brief           Get input value casted to `unsigned 16-bit` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define LWCELL_U16(x)                      ((uint16_t)(x))

/**
 * \brief           Get input value casted to `unsigned 8-bit` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define LWCELL_U8(x)                       ((uint8_t)(x))

/**
 * \brief           Get input value casted to `signed 32-bit` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define LWCELL_I32(x)                      ((int32_t)(x))

/**
 * \brief           Get input value casted to `signed 16-bit` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define LWCELL_I16(x)                      ((int16_t)(x))

/**
 * \brief           Get input value casted to `signed 8-bit` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define LWCELL_I8(x)                       ((int8_t)(x))

/**
 * \brief           Get input value casted to `size_t` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define LWCELL_SZ(x)                       ((size_t)(x))

/**
 * \brief           Convert `unsigned 32-bit` number to string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwcell_u32_to_str(num, out)        lwcell_u32_to_gen_str(LWCELL_U32(num), (out), 0, 0)

/**
 * \brief           Convert `unsigned 32-bit` number to HEX string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \param[in]       w: Width of output string.
 *                      When number is shorter than width, leading `0` characters will apply.
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwcell_u32_to_hex_str(num, out, w) lwcell_u32_to_gen_str(LWCELL_U32(num), (out), 1, (w))

/**
 * \brief           Convert `signed 32-bit` number to string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwcell_i32_to_str(num, out)        lwcell_i32_to_gen_str(LWCELL_I32(num), (out))

/**
 * \brief           Convert `unsigned 16-bit` number to string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwcell_u16_to_str(num, out)        lwcell_u32_to_gen_str(LWCELL_U32(LWCELL_U16(num)), (out), 0, 0)

/**
 * \brief           Convert `unsigned 16-bit` number to HEX string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \param[in]       w: Width of output string.
 *                      When number is shorter than width, leading `0` characters will apply.
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwcell_u16_to_hex_str(num, out, w) lwcell_u32_to_gen_str(LWCELL_U32(LWCELL_U16(num)), (out), 1, (w))

/**
 * \brief           Convert `signed 16-bit` number to string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwcell_i16_to_str(num, out)        lwcell_i32_to_gen_str(LWCELL_I32(LWCELL_I16(num)), (out))

/**
 * \brief           Convert `unsigned 8-bit` number to string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwcell_u8_to_str(num, out)         lwcell_u32_to_gen_str(LWCELL_U32(LWCELL_U8(num)), (out), 0, 0)

/**
 * \brief           Convert `unsigned 16-bit` number to HEX string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \param[in]       w: Width of output string.
 *                      When number is shorter than width, leading `0` characters will apply.
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwcell_u8_to_hex_str(num, out, w)  lwcell_u32_to_gen_str(LWCELL_U32(LWCELL_U8(num)), (out), 1, (w))

/**
 * \brief           Convert `signed 8-bit` number to string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwcell_i8_to_str(num, out)         lwcell_i32_to_gen_str(LWCELL_I32(LWCELL_I8(num)), (out))

char* lwcell_u32_to_gen_str(uint32_t num, char* out, uint8_t is_hex, uint8_t padding);
char* lwcell_i32_to_gen_str(int32_t num, char* out);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWCELL_HDR_UTILITIES_H */
