/**
 * \file            lwcell_ll_stm32_hal.c
 * \brief           Low-level communication with GSM device for STM32
 */

/*
 * Copyright (c) 2024 Tilen MAJERLE and Ilya Kargapolov
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
 * Authors:          Tilen MAJERLE <tilen@majerle.eu>,
 *                   Harald LESCHNER <picohari@googlemail.com>
 * Version:          v0.1.2
 */

#include <string.h>

#include "config.h"
#include "log.h"
#include "usart.h"

#include "cmsis_os2.h"
#include "lwcell_ll_stm32_hal.h"

#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_usart.h"

#include "lwcell/lwcell.h"
#include "lwcell/lwcell_input.h"
#include "lwcell/lwcell_mem.h"
#include "lwcell/lwcell_private.h"
#include "lwcell/lwcell_types.h"
#include "lwcell/lwcell_utils.h"
#include "system/lwcell_ll.h"
#include "system/lwcell_sys.h"

#define ARRAY_LEN(x)        (sizeof(x) / sizeof((x)[0]))

/* Print USART traffic on logging console */
#define LWCELL_DBG_LOG_UART 1

#ifndef LWCELL_CFG_INPUT_USE_PROCESS
#error "LWCELL_CFG_INPUT_USE_PROCESS must be enabled"
#endif

/* State variables */
static uint8_t initialized;
static size_t old_pos = 0;

/* Reception buffer for USART-DMA transfer */
static uint8_t usart_rx_dma_buffer[LWCELL_USART_DMA_RX_BUFF_SIZE];

/* Message queue ID */
osMessageQueueId_t usart_rx_dma_queue_id;
osEventFlagsId_t lwcell_ready_evt_id; /* To be shared with other tasks: lwcell_ready_evt_id = osEventFlagsNew(NULL); */

/* Tasks ID */
osThreadId_t uartTask_id;
osThreadId_t gnssTask_id;

/* Forward declarations for internal use */
static void usart_rx_check(void);
static void usart_rx_dma_thread(void* arg);
static size_t usart_send(const void* data, size_t len);
static void gnss_poll_thread(void* arg);
static lwcellr_t lwcell_callback_func(lwcell_evt_t* evt);

/* Testing stuff */
// static void usart_send_test_string(const char* str);

/**
 * \brief           Check for new data in USART RX DMA buffer
 */
static void
usart_rx_check(void) {
    size_t pos;

    /* Calculate current position in buffer and check for new data available */
    pos = ARRAY_LEN(usart_rx_dma_buffer) - LL_DMA_GetDataLength(DMA2, LL_DMA_STREAM_2);

    if (pos != old_pos) { /* Check change in received data */

        if (pos > old_pos) { /* Current position is over previous one */
/*
 * Processing is done in "linear" mode.
 *
 * Application processing is fast with single data block,
 * length is simply calculated by subtracting pointers
 *
 * [   0   ]
 * [   1   ] <- old_pos |------------------------------------|
 * [   2   ]            |                                    |
 * [   3   ]            | Single block (len = pos - old_pos) |
 * [   4   ]            |                                    |
 * [   5   ]            |------------------------------------|
 * [   6   ] <- pos
 * [   7   ]
 * [ N - 1 ]
 */
#if LWCELL_DBG_LOG_UART
            LOG_DEBUG("RX(%u): %.*s", (unsigned)(pos - old_pos), (int)(pos - old_pos), &usart_rx_dma_buffer[old_pos]);
#endif
            lwcell_input_process(&usart_rx_dma_buffer[old_pos], pos - old_pos);

        } else {
/*
 * Processing is done in "overflow" mode..
 *
 * Application must process data twice,
 * since there are 2 linear memory blocks to handle
 *
 * [   0   ]            |---------------------------------|
 * [   1   ]            | Second block (len = pos)        |
 * [   2   ]            |---------------------------------|
 * [   3   ] <- pos
 * [   4   ] <- old_pos |---------------------------------|
 * [   5   ]            |                                 |
 * [   6   ]            | First block (len = N - old_pos) |
 * [   7   ]            |                                 |
 * [ N - 1 ]            |---------------------------------|
 */
#if LWCELL_DBG_LOG_UART
            LOG_DEBUG("RX(%u): %.*s", (unsigned)(ARRAY_LEN(usart_rx_dma_buffer) - old_pos),
                      (int)(ARRAY_LEN(usart_rx_dma_buffer) - old_pos), &usart_rx_dma_buffer[old_pos]);
#endif
            lwcell_input_process(&usart_rx_dma_buffer[old_pos], ARRAY_LEN(usart_rx_dma_buffer) - old_pos);

            if (pos > 0) {
#if LWCELL_DBG_LOG_UART
                LOG_DEBUG("RX(%u): %.*s", (unsigned)pos, (int)pos, &usart_rx_dma_buffer[0]);
#endif
                lwcell_input_process(&usart_rx_dma_buffer[0], pos);
            }
        }
        old_pos = pos; /* Save current position as old for next transfers */
    }
}

/**
 * \brief           USART RX DMA thread
 * \param[in]       arg: Thread argument (not used)
 */
static void
usart_rx_dma_thread(void* arg) {
    (void)arg; /* Unused */

    void* d; /* Void pointer for queue */

    /* Send test data */
    // usart_send_test_string("Test 1\r\n");
    // usart_send_test_string("Test 2\r\n");

    while (1) {
        /* Block thread and wait for event to process USART data */
        osMessageQueueGet(usart_rx_dma_queue_id, &d, NULL, osWaitForever);

        /* Simply call processing function */
        usart_rx_check();

        (void)d; /* Empty */
    }
}

/**
 * \brief           Poll GNSS/GPS data thread
 * \param[in]       arg: Thread argument (not used)
 */
static void
gnss_poll_thread(void* arg) {
    (void)arg; /* Unused */

    /* Enable GNSS/GPS reception */
    lwcell_gnss_enable(NULL, NULL, 1);
    osDelay(1000);

    while (1) {
        /* Read gnss data */
        lwcell_gnss_info(NULL, NULL, 1);
        osDelay(2000);
    }
}

/**
 * \brief           Initialization thread for SIM7000 module
 * \param[in]       arg: Thread argument (not used)
 */
void
LWCELL_thread(void* arg) {
    (void)arg; /* Unused */

    /* Create message queue before initializing UART */
    /* This is to make sure message queue is ready before UART interrupts are enabled */
    usart_rx_dma_queue_id = osMessageQueueNew(10, sizeof(void*), NULL);

    osDelay(2000);

    LOG_DEBUG("\r\n[INFO] Starting SIM7000 initialization thread");

    /* Start reception - uart has been initialized in main.c */
    HAL_UART_Receive_DMA(&huart1, usart_rx_dma_buffer, sizeof(usart_rx_dma_buffer));
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);

    /*
     * After cold power-up, the SIM7000E module needs a variable amount of time
     * before its UART/autobaud detector is ready. Keep sending "AT\r" until the
     * module actually responds, instead of blindly waiting a fixed time.
     */
    if (!lwcell_ll_wait_module_ready(20000, 2000)) {
        LOG_DEBUG("[ERROR] Module did not come up after power-on, continuing anyway...");
    }

    /* Definitions for lwcell_uart task */
    const osThreadAttr_t uartTask_attr = {
        .name = "lwcell_uart",
        .stack_size = configMINIMAL_STACK_SIZE * 10,
        .priority = (osPriority_t)osPriorityNormal,
    };

    /* Start DMA reception task */
    uartTask_id = osThreadNew(usart_rx_dma_thread, NULL, &uartTask_attr);

    if (uartTask_id == NULL) {
        LOG_DEBUG("[ERROR] Cannot create USART RX DMA thread");
    }

    /* Initialize lwcell subsystem with default callback function - is blocking! */
    if (lwcell_init(lwcell_callback_func, 1) != lwcellOK) {
        LOG_DEBUG("[ERROR] Cannot initialize LwCELL");
    } else {
        LOG_DEBUG("[INFO] LwCELL initialized!");

        osDelay(5000); /* Puh! We did it, module is initialized - time to have a break! */

        /* Signal other tasks that lwcell API is now safe to use */
        osEventFlagsSet(lwcell_ready_evt_id, LWCELL_READY_FLAG);

        /* Now it's safe to start some other tasks using the lwcell API .. */
        const osThreadAttr_t gnssTask_attr = {
            .name = "lwcell_gnss",
            .stack_size = configMINIMAL_STACK_SIZE * 8,
            .priority = (osPriority_t)osPriorityLow,
        };
        gnssTask_id = osThreadNew(gnss_poll_thread, NULL, &gnssTask_attr);
        if (gnssTask_id == NULL) {
            LOG_DEBUG("[ERROR] Cannot create GNSS poll thread");
        }

        /* Do more things here ... */
    }

    /* Terminate this thread */
    osThreadExit();
}

/**
 * \brief           Process received data over UART
 * \note            Either process them directly or copy to other bigger buffer
 * \param[in]       data: Data to process
 * \param[in]       len: Length in units of bytes
 */
static size_t
usart_send(const void* data, size_t len) {
    const uint8_t* d = data;
    size_t total = len;

#if LWCELL_DBG_LOG_UART
    LOG_DEBUG("TX(%u): %.*s", (unsigned)len, (int)len, (const char*)data);
#endif

    /*
     * This function is called on DMA TC or HT events, and on UART IDLE (if enabled) event.
     *
     * For the sake of this example, function does a loop-back data over UART in polling mode.
     * Check ringbuff RX-based example for implementation with TX & RX DMA transfer.
     */

    for (; len > 0; --len, ++d) {
        LL_USART_TransmitData8(USART1, *d);
        while (!LL_USART_IsActiveFlag_TXE(USART1)) {}
    }
    while (!LL_USART_IsActiveFlag_TC(USART1)) {}

    return total;
}

#if 0
/**
 * \brief           Send string to USART
 * \param[in]       str: String to send
 */
static void usart_send_test_string(const char* str)
{
    usart_send_data(str, strlen(str));
}
#endif

#if defined(LWCELL_RESET_PIN)
/**
 * \brief           Hardware reset callback
 */
static uint8_t
reset_device(uint8_t state) {
    if (state) { /* Activate reset line */
        LL_GPIO_ResetOutputPin(LWCELL_RESET_PORT, LWCELL_RESET_PIN);
    } else {
        LL_GPIO_SetOutputPin(LWCELL_RESET_PORT, LWCELL_RESET_PIN);
    }
    return 1;
}
#endif /* defined(LWCELL_RESET_PIN) */

/**
 * \brief           Initialize low-level functions for LwCELL
 * \param[in]       ll: Pointer to low-level structure
 * \return          lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
lwcell_ll_init(lwcell_ll_t* ll) {
#if !LWCELL_CFG_MEM_CUSTOM
    static uint8_t memory[LWCELL_USART_DMA_RX_BUFF_SIZE];
    lwcell_mem_region_t mem_regions[] = {{memory, sizeof(memory)}};

    if (!initialized) {
        lwcell_mem_assignmemory(mem_regions, LWCELL_ARRAYSIZE(mem_regions)); /* Assign memory for allocations */
    }
#endif /* !LWCELL_CFG_MEM_CUSTOM */

    if (!initialized) {
        ll->send_fn = usart_send; /* Set callback function to send data */
#if defined(LWCELL_RESET_PIN)
        ll->reset_fn = reset_device; /* Set callback for hardware reset */
#endif                               /* defined(LWCELL_RESET_PIN) */
    }

    initialized = 1;
    return lwcellOK;
}

/**
 * \brief           Callback function to de-init low-level communication part
 * \param[in,out]   ll: Pointer to \ref lwcell_ll_t structure to fill data for communication functions
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t enumeration otherwise
 */
lwcellr_t
lwcell_ll_deinit(lwcell_ll_t* ll) {
    if (usart_rx_dma_queue_id != NULL) {
        osMessageQueueId_t tmp = usart_rx_dma_queue_id;
        usart_rx_dma_queue_id = NULL;
        osMessageQueueDelete(tmp);
    }
    if (gnssTask_id != NULL) {
        osThreadId_t tmp = gnssTask_id;
        gnssTask_id = NULL;
        osThreadTerminate(tmp);
    }
    if (uartTask_id != NULL) {
        osThreadId_t tmp = uartTask_id;
        uartTask_id = NULL;
        osThreadTerminate(tmp);
    }
    initialized = 0;
    LWCELL_UNUSED(ll);
    return lwcellOK;
}

/**
 * \brief       Wait for module to respond to AT commands (autobaud sync)
 *
 * Sends "AT\r" repeatedly until the module produces its first response.
 * Since SIM7000E has local echo enabled by default (ATE1), every "AT\r" we
 * send will be echoed back by the module itself once it is alive - so we
 * must stop sending pings as soon as we see a response, otherwise our own
 * echo traffic would keep the "quiet period" from ever being reached.
 *
 * After the first response, we stop pinging entirely and just passively
 * wait for quiet_ms of silence, which then reliably indicates that the
 * module has finished its boot banner and is ready for real commands.
 */
uint8_t
lwcell_ll_wait_module_ready(uint32_t max_wait_ms, uint32_t quiet_ms) {
    const uint32_t step_ms = 300;
    uint32_t waited = 0;
    uint32_t quiet_time = 0;
    size_t pos_before;
    size_t pos_now;
    uint8_t got_response = 0;

    char ping[] = "AT\r";

    /*
     * Give the module a brief grace period before starting to ping.
     * This matters especially after AT+CFUN=1,1: the module first still
     * answers with the "OK" for the CFUN command itself, and only shortly
     * after actually begins its internal reboot. Pinging immediately can
     * otherwise get a stale/misleading "early" response.
     */
    osDelay(1000);
    waited += 1000;

    pos_before = ARRAY_LEN(usart_rx_dma_buffer) - LL_DMA_GetDataLength(DMA2, LL_DMA_STREAM_2);

    /* Phase 1: keep sending "AT\r" until we see the first response */
    while (!got_response && waited < max_wait_ms) {

        usart_send(ping, strlen(ping));
        osDelay(step_ms);
        waited += step_ms;

        pos_now = ARRAY_LEN(usart_rx_dma_buffer) - LL_DMA_GetDataLength(DMA2, LL_DMA_STREAM_2);

        if (pos_now != pos_before) {
            got_response = 1;
            LOG_DEBUG("[INFO] Module started responding after %u ms", (unsigned)waited);
            pos_before = pos_now;
        }
    }

    if (!got_response) {
        LOG_DEBUG("[WARN] Module did not respond within %u ms", (unsigned)max_wait_ms);
        lwcell_ll_rx_clear();
        return 0;
    }

    /* Phase 2: stop pinging (avoid our own echo!), just passively wait
     * for quiet_ms of silence to let boot banner / remaining traffic settle */
    quiet_time = 0;
    while (waited < max_wait_ms && quiet_time < quiet_ms) {

        osDelay(step_ms);
        waited += step_ms;

        pos_now = ARRAY_LEN(usart_rx_dma_buffer) - LL_DMA_GetDataLength(DMA2, LL_DMA_STREAM_2);

        if (pos_now != pos_before) {
            quiet_time = 0; /* Still receiving data (e.g. boot banner) */
            pos_before = pos_now;
        } else {
            quiet_time += step_ms;
        }
    }

    LOG_DEBUG("[INFO] Module settled after %u ms total", (unsigned)waited);

    /* Reset RX tracking so lwcell starts clean, discarding sync traffic */
    lwcell_ll_rx_clear();

    return got_response;
}

/**
 * \brief           Discard any pending/unprocessed data in the platform's
 *                  RX buffer and reset RX position tracking to the current
 *                  DMA write position (NOT necessarily 0!).
 */
lwcellr_t
lwcell_ll_rx_clear(void) {
    /* Content itself doesn't matter - we only discard it by advancing
     * old_pos past it. memset() is not strictly necessary, but harmless
     * and helps if you ever want to dump the raw buffer for debugging. */
    memset(usart_rx_dma_buffer, 0, sizeof(usart_rx_dma_buffer));

    /* IMPORTANT: old_pos must track the actual current DMA write position,
     * not 0 - the DMA pointer keeps advancing independently of the memset
     * above and does not reset to the start of the buffer. */
    old_pos = ARRAY_LEN(usart_rx_dma_buffer) - LL_DMA_GetDataLength(DMA2, LL_DMA_STREAM_2);

    return lwcellOK;
}

/**
 * \brief           Event callback function for GSM stack
 * \param[in]       evt: Event information with data
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
static lwcellr_t
lwcell_callback_func(lwcell_evt_t* evt) {
    switch (lwcell_evt_get_type(evt)) {

        case LWCELL_EVT_INIT_FINISH: LOG_DEBUG("[MSG DBG] Booting SIMcom device ..."); break;
        case LWCELL_EVT_RESET: LOG_DEBUG("[INFO] %s module ready!", lwcell.m.model_number); break;
        case LWCELL_EVT_DEVICE_IDENTIFIED:
            LOG_DEBUG("[INFO] Manufacturer: %s", lwcell.m.model_manufacturer);
            LOG_DEBUG("[INFO] Model:        %s", lwcell.m.model_number);
            LOG_DEBUG("[INFO] Revision:     %s", lwcell.m.model_revision);
            LOG_DEBUG("[INFO] -----------------------------");
            LOG_DEBUG("[INFO] IMEI:         %s", lwcell.m.model_serial_number);
            LOG_DEBUG("[INFO] -----------------------------");
            break;
        case LWCELL_EVT_GNSS_ENABLE:
            LOG_DEBUG("[MSG GNSS] Module %s", lwcell.m.gnss.enabled ? "active" : "disabled");
            break;
        case LWCELL_EVT_GNSS_POWER:
            LOG_DEBUG("[MSG GNSS] Power %s", lwcell.m.gnss.enabled ? "enabled" : "disabled");
            break;
        case LWCELL_EVT_GNSS_READY:
            if (evt->evt.gnss_parse.res != lwcellOK) {
                LOG_DEBUG("[MSG GNSS] Parser error: %d", evt->evt.gnss_parse.res); /* Report error */
            } else {
                LOG_DEBUG("[MSG GNSS] Parser status: %s", evt->evt.gnss_parse.gnss->fix_status ? "Ready" : "No data");
            }
            break;

            /* Process and print registration change */
            // case LWCELL_EVT_NETWORK_REG_CHANGED: network_utils_process_reg_change(evt); break;

            /* Process current network operator */
            // case LWCELL_EVT_NETWORK_OPERATOR_CURRENT: network_utils_process_curr_operator(evt); break;

            /* Process signal strength */
            // case LWCELL_EVT_SIGNAL_STRENGTH: network_utils_process_rssi(evt); break;

            /* Other user events here... */

        default: break;
    }
    return lwcellOK;
}

#if INTERRUPT_HANDLING_INFO

/* Have this in stm32f4xx_it.c */

void
DMA2_Stream2_IRQHandler(void) {
    void* d = (void*)1;

    /* Check half-transfer complete interrupt */
    if (LL_DMA_IsEnabledIT_HT(DMA2, LL_DMA_STREAM_2) && LL_DMA_IsActiveFlag_HT1(DMA2)) {
        LL_DMA_ClearFlag_HT1(DMA2);                         /* Clear half-transfer complete flag */
        osMessageQueuePut(usart_rx_dma_queue_id, &d, 0, 0); /* Write data to queue. Do not use wait function! */
    }

    /* Check transfer-complete interrupt */
    if (LL_DMA_IsEnabledIT_TC(DMA2, LL_DMA_STREAM_2) && LL_DMA_IsActiveFlag_TC1(DMA2)) {
        LL_DMA_ClearFlag_TC1(DMA2);                         /* Clear transfer complete flag */
        osMessageQueuePut(usart_rx_dma_queue_id, &d, 0, 0); /* Write data to queue. Do not use wait function! */
    }

    HAL_DMA_IRQHandler(&hdma_usart1_rx);
}

void
USART1_IRQHandler(void) {
    void* d = (void*)1;

    /* Check for IDLE line interrupt */
    if (LL_USART_IsEnabledIT_IDLE(USART1) && LL_USART_IsActiveFlag_IDLE(USART1)) {
        LL_USART_ClearFlag_IDLE(USART1);                    /* Clear IDLE line flag */
        osMessageQueuePut(usart_rx_dma_queue_id, &d, 0, 0); /* Write data to queue. Do not use wait function! */
    }

    HAL_UART_IRQHandler(&huart1);
}

#endif /* INTERRUPT_HANDLING_INFO */