/**
 * \file            lwcell_ll_win32.c
 * \brief           Low-level communication with GSM device for WIN32
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
 * This file is part of LwCELL - Lightweight GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.1
 */
#include "lwcell/lwcell_input.h"
#include "lwcell/lwcell_mem.h"
#include "lwcell/lwcell_types.h"
#include "lwcell/lwcell_utils.h"
#include "system/lwcell_ll.h"
#include "system/lwcell_sys.h"
#include "windows.h"

#if !__DOXYGEN__

static uint8_t initialized = 0;
static HANDLE thread_handle;
static volatile HANDLE com_port;    /*!< COM port handle */
static uint8_t data_buffer[0x1000]; /*!< Received data array */

static void uart_thread(void* param);

/**
 * \brief           Send data to GSM device, function called from GSM stack when we have data to send
 * \param[in]       data: Pointer to data to send
 * \param[in]       len: Number of bytes to send
 * \return          Number of bytes sent
 */
static size_t
send_data(const void* data, size_t len) {
    DWORD written;
    if (com_port != NULL) {
#if !LWCELL_CFG_AT_ECHO
        const uint8_t* d = data;
        HANDLE hConsole;

        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
        for (DWORD i = 0; i < len; ++i) {
            printf("%c", d[i]);
        }
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif /* !LWCELL_CFG_AT_ECHO */

        /* Write data to AT port */
        WriteFile(com_port, data, len, &written, NULL);
        FlushFileBuffers(com_port);
        return written;
    }
    return 0;
}

/**
 * \brief           Configure UART (USB to UART)
 */
static uint8_t
configure_uart(uint32_t baudrate) {
    DCB dcb = {0};
    dcb.DCBlength = sizeof(dcb);

    /*
     * On first call,
     * create virtual file on selected COM port and open it
     * as generic read and write
     */
    if (!initialized) {
        static const char* com_ports[] = {"\\\\.\\COM23", "\\\\.\\COM12", "\\\\.\\COM9", "\\\\.\\COM8", "\\\\.\\COM4"};
        for (size_t i = 0; i < sizeof(com_ports) / sizeof(com_ports[0]); ++i) {
            com_port = CreateFileA(com_ports[i], GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, NULL);
            if (GetCommState(com_port, &dcb)) {
                printf("COM PORT %s opened!\r\n", (const char*)com_ports[i]);
                break;
            }
        }
    }

    /* Configure COM port parameters */
    if (GetCommState(com_port, &dcb)) {
        COMMTIMEOUTS timeouts;

        dcb.BaudRate = baudrate;
        dcb.ByteSize = 8;
        dcb.Parity = NOPARITY;
        dcb.StopBits = ONESTOPBIT;

        if (!SetCommState(com_port, &dcb)) {
            printf("Cannot set COM PORT info\r\n");
            return 0;
        }
        if (GetCommTimeouts(com_port, &timeouts)) {
            /* Set timeout to return immediately from ReadFile function */
            timeouts.ReadIntervalTimeout = MAXDWORD;
            timeouts.ReadTotalTimeoutConstant = 0;
            timeouts.ReadTotalTimeoutMultiplier = 0;
            if (!SetCommTimeouts(com_port, &timeouts)) {
                printf("Cannot set COM PORT timeouts\r\n");
            }
            GetCommTimeouts(com_port, &timeouts);
        } else {
            printf("Cannot get COM PORT timeouts\r\n");
            return 0;
        }
    } else {
        printf("Cannot get COM PORT info\r\n");
        return 0;
    }

    /* On first function call, create a thread to read data from COM port */
    if (!initialized) {
        lwcell_sys_thread_create(&thread_handle, "lwcell_ll_thread", uart_thread, NULL, 0, 0);
    }
    return 1;
}

/**
 * \brief            UART thread
 */
static void
uart_thread(void* param) {
    DWORD bytes_read;
    lwcell_sys_sem_t sem;
    FILE* file = NULL;

    LWCELL_UNUSED(param);

    lwcell_sys_sem_create(&sem, 0); /* Create semaphore for delay functions */

    while (com_port == NULL) {
        lwcell_sys_sem_wait(&sem, 1); /* Add some delay with yield */
    }

    fopen_s(&file, "log_file.txt", "w+"); /* Open debug file in write mode */
    while (1) {
        /*
         * Try to read data from COM port
         * and send it to upper layer for processing
         */
        do {
            ReadFile(com_port, data_buffer, sizeof(data_buffer), &bytes_read, NULL);
            if (bytes_read > 0) {
                HANDLE hConsole;
                hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
                SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
                for (DWORD i = 0; i < bytes_read; ++i) {
                    printf("%c", data_buffer[i]);
                }
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

                /* Send received data to input processing module */
#if LWCELL_CFG_INPUT_USE_PROCESS
                lwcell_input_process(data_buffer, (size_t)bytes_read);
#else  /* LWCELL_CFG_INPUT_USE_PROCESS */
                lwcell_input(data_buffer, (size_t)bytes_read);
#endif /* !LWCELL_CFG_INPUT_USE_PROCESS */

                /* Write received data to output debug file */
                if (file != NULL) {
                    fwrite(data_buffer, 1, bytes_read, file);
                    fflush(file);
                }
            }
        } while (bytes_read == (DWORD)sizeof(data_buffer));

        /* Implement delay to allow other tasks processing */
        lwcell_sys_sem_wait(&sem, 1);
    }
}

/**
 * \brief           Callback function called from initialization process
 *
 * \note            This function may be called multiple times if AT baudrate is changed from application.
 *                  It is important that every configuration except AT baudrate is configured only once!
 *
 * \note            This function may be called from different threads in GSM stack when using OS.
 *                  When \ref LWCELL_CFG_INPUT_USE_PROCESS is set to 1, this function may be called from user UART thread.
 *
 * \param[in,out]   ll: Pointer to \ref lwcell_ll_t structure to fill data for communication functions
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_ll_init(lwcell_ll_t* ll) {
#if !LWCELL_CFG_MEM_CUSTOM
    /* Step 1: Configure memory for dynamic allocations */
    static uint8_t memory[0x10000]; /* Create memory for dynamic allocations with specific size */

    /*
     * Create memory region(s) of memory.
     * If device has internal/external memory available,
     * multiple memories may be used
     */
    lwcell_mem_region_t mem_regions[] = {{memory, sizeof(memory)}};
    if (!initialized) {
        lwcell_mem_assignmemory(mem_regions,
                               LWCELL_ARRAYSIZE(mem_regions)); /* Assign memory for allocations to GSM library */
    }
#endif /* !LWCELL_CFG_MEM_CUSTOM */

    /* Step 2: Set AT port send function to use when we have data to transmit */
    if (!initialized) {
        ll->send_fn = send_data; /* Set callback function to send data */
    }

    /* Step 3: Configure AT port to be able to send/receive data to/from GSM device */
    if (!configure_uart(ll->uart.baudrate)) { /* Initialize UART for communication */
        return lwcellERR;
    }
    initialized = 1;
    return lwcellOK;
}

#endif /* !__DOXYGEN__ */
