/**
 * \file            main.c
 * \brief           Main file
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
 * Before you start using WIN32 implementation with USB and VCP,
 * check lwcell_ll_win32.c implementation and choose your COM port!
 */
#include "lwcell/lwcell.h"
#include "sim_manager.h"
#include "network_utils.h"
#include "device_info.h"
#include "examples_common_lwcell_callback_func.h"

/**
 * \brief           Program entry point
 */
int
main(void) {
    printf("Starting GSM application!\r\n");

    /* Initialize GSM with default callback function */
    if (lwcell_init(examples_common_lwcell_callback_func, 1) != lwcellOK) {
        printf("Cannot initialize LwCELL\r\n");
    }

    /* Read device info */
    read_device_info();

    /*
     * Do not stop program here.
     * New threads were created for GSM processing
     */
    while (1) {
        lwcell_delay(1000);
    }

    return 0;
}
