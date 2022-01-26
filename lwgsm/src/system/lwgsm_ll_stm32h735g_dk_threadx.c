/**
 * \file            lwgsm_ll_stm32h735g_dk_threadx.c
 * \brief           STM32H735-DK driver with ThreadX
 */

/*
 * Copyright (c) 2022 Tilen MAJERLE
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
 * This file is part of LwGSM - Lightweight GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.1
 */

/*
 * This is special driver for STM32H735G-DK utilizing ThreadX operating system
 * and full DMA support for TX and RX operation.
 */
#include "lwgsm/lwgsm.h"
#include "lwgsm/lwgsm_mem.h"
#include "lwgsm/lwgsm_input.h"
#include "system/lwgsm_ll.h"
#include "lwrb/lwrb.h"
#include "tx_api.h"
#include "mcu.h"

#if !__DOXYGEN__

#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_usart.h"
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_dma.h"
#include "stm32h7xx_ll_rcc.h"

#if !LWGSM_CFG_INPUT_USE_PROCESS
#error "LWGSM_CFG_INPUT_USE_PROCESS must be enabled in `lwgsm_opts.h` to use this driver."
#endif /* LWGSM_CFG_INPUT_USE_PROCESS */
#if !LWGSM_CFG_MEM_CUSTOM
#error "LWGSM_CFG_MEM_CUSTOM must be used instead. This driver does not set memory regions for LwESP."
#endif /* !LWGSM_CFG_MEM_CUSTOM */

/* 
 * USART setup
 *
 * PF6 and PF7 are used together with external STMOD+ extension board 
 */
#define LWGSM_USART                                 UART7
#define LWGSM_USART_CLK_EN                          LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART7)
#define LWGSM_USART_IRQ                             UART7_IRQn
#define LWGSM_USART_IRQ_HANDLER                     UART7_IRQHandler

/* TX DMA */
#define LWGSM_USART_DMA_TX                          DMA1
#define LWGSM_USART_DMA_TX_STREAM                   LL_DMA_STREAM_3
#define LWGSM_USART_DMA_TX_REQUEST                  LL_DMAMUX1_REQ_UART7_TX
#define LWGSM_USART_DMA_TX_CLK_EN                   LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1)
#define LWGSM_USART_DMA_TX_IRQ                      DMA1_Stream3_IRQn
#define LWGSM_USART_DMA_TX_IRQ_HANDLER              DMA1_Stream3_IRQHandler
#define LWGSM_USART_DMA_TX_IS_TC                    LL_DMA_IsActiveFlag_TC3(LWGSM_USART_DMA_TX)
#define LWGSM_USART_DMA_TX_CLEAR_TC                 LL_DMA_ClearFlag_TC3(LWGSM_USART_DMA_TX)
#define LWGSM_USART_DMA_TX_CLEAR_HT                 LL_DMA_ClearFlag_HT3(LWGSM_USART_DMA_TX)
#define LWGSM_USART_DMA_TX_CLEAR_TE                 LL_DMA_ClearFlag_TE3(LWGSM_USART_DMA_TX)

/* RX DMA */
#define LWGSM_USART_DMA_RX                          DMA1
#define LWGSM_USART_DMA_RX_STREAM                   LL_DMA_STREAM_2
#define LWGSM_USART_DMA_RX_REQUEST                  LL_DMAMUX1_REQ_UART7_RX
#define LWGSM_USART_DMA_RX_CLK_EN                   LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1)
#define LWGSM_USART_DMA_RX_IRQ                      DMA1_Stream2_IRQn
#define LWGSM_USART_DMA_RX_IRQ_HANDLER              DMA1_Stream2_IRQHandler
#define LWGSM_USART_DMA_RX_IS_TC                    LL_DMA_IsActiveFlag_TC2(LWGSM_USART_DMA_RX)
#define LWGSM_USART_DMA_RX_IS_HT                    LL_DMA_IsActiveFlag_HT2(LWGSM_USART_DMA_RX)
#define LWGSM_USART_DMA_RX_IS_TE                    LL_DMA_IsActiveFlag_TE2(LWGSM_USART_DMA_RX)
#define LWGSM_USART_DMA_RX_CLEAR_TC                 LL_DMA_ClearFlag_TC2(LWGSM_USART_DMA_RX)
#define LWGSM_USART_DMA_RX_CLEAR_HT                 LL_DMA_ClearFlag_HT2(LWGSM_USART_DMA_RX)
#define LWGSM_USART_DMA_RX_CLEAR_TE                 LL_DMA_ClearFlag_TE2(LWGSM_USART_DMA_RX)

/* GPIO configuration */
#define LWGSM_USART_TX_PORT                         GPIOF
#define LWGSM_USART_TX_PIN                          LL_GPIO_PIN_7
#define LWGSM_USART_TX_PORT_CLK_EN                  LL_AHB1_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOF)
#define LWGSM_USART_TX_PIN_AF                       LL_GPIO_AF_7
#define LWGSM_USART_RX_PORT                         GPIOF
#define LWGSM_USART_RX_PIN                          LL_GPIO_PIN_6
#define LWGSM_USART_RX_PORT_CLK_EN                  LL_AHB1_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOF)
#define LWGSM_USART_RX_PIN_AF                       LL_GPIO_AF_7

/* TX data buffers, must be 32-bytes aligned (cache) and in dma buffer section to make sure DMA has access to the memory region */
ALIGN_32BYTES(static uint8_t __attribute__((section(".dma_buffer"))) lwgsm_tx_rb_data[4096]);
static lwrb_t       lwgsm_tx_rb;
volatile size_t     lwgsm_tx_len;

/* USART thread */
static void prv_lwgsm_read_thread_entry(ULONG arg);
static TX_THREAD    lwgsm_read_thread;
static UCHAR        lwgsm_read_thread_stack[4 * LWGSM_SYS_THREAD_SS];
static size_t       lwgsm_read_old_pos = 0;

/* Message queue */
#define LL_QUEUE_NUM_OF_ENTRY               10
static UCHAR        lwgsm_usart_ll_mbox_mem[LL_QUEUE_NUM_OF_ENTRY * sizeof(ULONG)];
static TX_QUEUE     lwgsm_usart_ll_mbox;

/* USART memory */
ALIGN_32BYTES(static uint8_t __attribute__((section(".dma_buffer"))) lwgsm_usart_rx_dma_buffer[0x100]);
static uint8_t      lwgsm_is_running, lwgsm_initialized = 0;

/**
 * \brief           USART data processing thread
 * This is the thread used to enter received data from UART to the LwESP stack for further processing
 * \param[in]       arg: User argument
 */
static void
prv_lwgsm_read_thread_entry(ULONG arg) {
    size_t pos;

    LWGSM_UNUSED(arg);

    while (1) {
        void* d;
        /* Wait for the event message from DMA or USART */
        tx_queue_receive(&lwgsm_usart_ll_mbox, &d, TX_WAIT_FOREVER);

        /* Read data */
        pos = sizeof(lwgsm_usart_rx_dma_buffer) - LL_DMA_GetDataLength(LWGSM_USART_DMA_RX, LWGSM_USART_DMA_RX_STREAM);
        if (pos != lwgsm_read_old_pos && lwgsm_is_running) {
            SCB_InvalidateDCache_by_Addr(lwgsm_usart_rx_dma_buffer, sizeof(lwgsm_usart_rx_dma_buffer));
            if (pos > lwgsm_read_old_pos) {
                lwgsm_input_process(&lwgsm_usart_rx_dma_buffer[lwgsm_read_old_pos], pos - lwgsm_read_old_pos);
            } else {
                lwgsm_input_process(&lwgsm_usart_rx_dma_buffer[lwgsm_read_old_pos], sizeof(lwgsm_usart_rx_dma_buffer) - lwgsm_read_old_pos);
                if (pos > 0) {
                    lwgsm_input_process(&lwgsm_usart_rx_dma_buffer[0], pos);
                }
            }
            lwgsm_read_old_pos = pos;
        }
    }
}

/**
 * \brief           Try to send more data with DMA
 */
static void
prv_start_tx_transfer(void) {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    if (lwgsm_tx_len == 0
        && (lwgsm_tx_len = lwrb_get_linear_block_read_length(&lwgsm_tx_rb)) > 0) {
        const void* d = lwrb_get_linear_block_read_address(&lwgsm_tx_rb);

        /* Limit tx len up to some size to optimize buffer reading process */
        lwgsm_tx_len = LWGSM_MIN(lwgsm_tx_len, 64);

        /* Cleanup cache first to make sure we have latest data in memory */
        SCB_CleanDCache_by_Addr((void *)d, lwgsm_tx_len);

        /* Disable channel if enabled */
        LL_DMA_DisableStream(LWGSM_USART_DMA_TX, LWGSM_USART_DMA_TX_STREAM);

        /* Clean flags */
        LWGSM_USART_DMA_TX_CLEAR_TC;
        LWGSM_USART_DMA_TX_CLEAR_HT;
        LWGSM_USART_DMA_TX_CLEAR_TE;

        /* Configure DMA */
        LL_DMA_SetMemoryAddress(LWGSM_USART_DMA_TX, LWGSM_USART_DMA_TX_STREAM, (uint32_t)d);
        LL_DMA_SetDataLength(LWGSM_USART_DMA_TX, LWGSM_USART_DMA_TX_STREAM, lwgsm_tx_len);

        /* Enable instances */
        LL_DMA_EnableStream(LWGSM_USART_DMA_TX, LWGSM_USART_DMA_TX_STREAM);
    }
    __set_PRIMASK(primask);
}

/**
 * \brief           Configure UART using DMA for receive in double buffer mode and IDLE line detection
 */
static void
prv_configure_uart(uint32_t baudrate) {
    LL_GPIO_InitTypeDef gpio_init = {0};
    LL_USART_InitTypeDef usart_init = {0};

    if (!lwgsm_initialized) {
        /* Enable peripheral clocks */
        LWGSM_USART_CLK_EN;
        LWGSM_USART_DMA_RX_CLK_EN;
        LWGSM_USART_DMA_TX_CLK_EN;
        LWGSM_USART_TX_PORT_CLK_EN;
        LWGSM_USART_RX_PORT_CLK_EN;

        /* Global pin configuration */
        LL_GPIO_StructInit(&gpio_init);
        gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        gpio_init.Pull = LL_GPIO_PULL_UP;
        gpio_init.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
        gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;

        /* TX PIN */
        gpio_init.Pin = LWGSM_USART_TX_PIN;
        gpio_init.Alternate = LWGSM_USART_TX_PIN_AF;
        LL_GPIO_Init(LWGSM_USART_TX_PORT, &gpio_init);

        /* RX PIN */
        gpio_init.Pin = LWGSM_USART_RX_PIN;
        gpio_init.Alternate = LWGSM_USART_RX_PIN_AF;
        LL_GPIO_Init(LWGSM_USART_RX_PORT, &gpio_init);

        /*******************/
        /*** UART RX DMA ***/
        /*******************/

        /* Enable DMA interrupts */
        NVIC_SetPriority(LWGSM_USART_DMA_RX_IRQ, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0x05, 0x00));
        NVIC_EnableIRQ(LWGSM_USART_DMA_RX_IRQ);

        /* Configure DMA */
        LL_DMA_SetPeriphRequest(LWGSM_USART_DMA_RX, LWGSM_USART_DMA_RX_STREAM, LWGSM_USART_DMA_RX_REQUEST);
        LL_DMA_SetDataTransferDirection(LWGSM_USART_DMA_RX, LWGSM_USART_DMA_RX_STREAM, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
        LL_DMA_SetStreamPriorityLevel(LWGSM_USART_DMA_RX, LWGSM_USART_DMA_RX_STREAM, LL_DMA_PRIORITY_MEDIUM);
        LL_DMA_SetMode(LWGSM_USART_DMA_RX, LWGSM_USART_DMA_RX_STREAM, LL_DMA_MODE_CIRCULAR);
        LL_DMA_SetPeriphIncMode(LWGSM_USART_DMA_RX, LWGSM_USART_DMA_RX_STREAM, LL_DMA_PERIPH_NOINCREMENT);
        LL_DMA_SetMemoryIncMode(LWGSM_USART_DMA_RX, LWGSM_USART_DMA_RX_STREAM, LL_DMA_MEMORY_INCREMENT);
        LL_DMA_SetPeriphSize(LWGSM_USART_DMA_RX, LWGSM_USART_DMA_RX_STREAM, LL_DMA_PDATAALIGN_BYTE);
        LL_DMA_SetMemorySize(LWGSM_USART_DMA_RX, LWGSM_USART_DMA_RX_STREAM, LL_DMA_MDATAALIGN_BYTE);
        LL_DMA_DisableFifoMode(LWGSM_USART_DMA_RX, LWGSM_USART_DMA_RX_STREAM);
        LL_DMA_SetPeriphAddress(LWGSM_USART_DMA_RX, LWGSM_USART_DMA_RX_STREAM, LL_USART_DMA_GetRegAddr(LWGSM_USART, LL_USART_DMA_REG_DATA_RECEIVE));
        LL_DMA_SetMemoryAddress(LWGSM_USART_DMA_RX, LWGSM_USART_DMA_RX_STREAM, (uint32_t)lwgsm_usart_rx_dma_buffer);
        LL_DMA_SetDataLength(LWGSM_USART_DMA_RX, LWGSM_USART_DMA_RX_STREAM, sizeof(lwgsm_usart_rx_dma_buffer));

        /* Enable DMA interrupts */
        LL_DMA_EnableIT_HT(LWGSM_USART_DMA_RX, LWGSM_USART_DMA_RX_STREAM);
        LL_DMA_EnableIT_TC(LWGSM_USART_DMA_RX, LWGSM_USART_DMA_RX_STREAM);
        LL_DMA_EnableIT_TE(LWGSM_USART_DMA_RX, LWGSM_USART_DMA_RX_STREAM);
        LL_DMA_EnableIT_FE(LWGSM_USART_DMA_RX, LWGSM_USART_DMA_RX_STREAM);
        LL_DMA_EnableIT_DME(LWGSM_USART_DMA_RX, LWGSM_USART_DMA_RX_STREAM);

        /*******************/
        /*** UART TX DMA ***/
        /*******************/

        /* Enable DMA interrupts */
        NVIC_SetPriority(LWGSM_USART_DMA_TX_IRQ, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0x05, 0x00));
        NVIC_EnableIRQ(LWGSM_USART_DMA_TX_IRQ);

        /* Configure DMA */
        LL_DMA_SetPeriphRequest(LWGSM_USART_DMA_TX, LWGSM_USART_DMA_TX_STREAM, LWGSM_USART_DMA_TX_REQUEST);
        LL_DMA_SetDataTransferDirection(LWGSM_USART_DMA_TX, LWGSM_USART_DMA_TX_STREAM, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
        LL_DMA_SetStreamPriorityLevel(LWGSM_USART_DMA_TX, LWGSM_USART_DMA_TX_STREAM, LL_DMA_PRIORITY_MEDIUM);
        LL_DMA_SetMode(LWGSM_USART_DMA_TX, LWGSM_USART_DMA_TX_STREAM, LL_DMA_MODE_NORMAL);
        LL_DMA_SetPeriphIncMode(LWGSM_USART_DMA_TX, LWGSM_USART_DMA_TX_STREAM, LL_DMA_PERIPH_NOINCREMENT);
        LL_DMA_SetMemoryIncMode(LWGSM_USART_DMA_TX, LWGSM_USART_DMA_TX_STREAM, LL_DMA_MEMORY_INCREMENT);
        LL_DMA_SetPeriphSize(LWGSM_USART_DMA_TX, LWGSM_USART_DMA_TX_STREAM, LL_DMA_PDATAALIGN_BYTE);
        LL_DMA_SetMemorySize(LWGSM_USART_DMA_TX, LWGSM_USART_DMA_TX_STREAM, LL_DMA_MDATAALIGN_BYTE);
        LL_DMA_DisableFifoMode(LWGSM_USART_DMA_TX, LWGSM_USART_DMA_TX_STREAM);
        LL_DMA_SetPeriphAddress(LWGSM_USART_DMA_TX, LWGSM_USART_DMA_TX_STREAM, LL_USART_DMA_GetRegAddr(LWGSM_USART, LL_USART_DMA_REG_DATA_TRANSMIT));

        /* Enable DMA interrupts */
        LL_DMA_EnableIT_TC(LWGSM_USART_DMA_TX, LWGSM_USART_DMA_TX_STREAM);
        LL_DMA_EnableIT_TE(LWGSM_USART_DMA_TX, LWGSM_USART_DMA_TX_STREAM);

        /*******************/
        /***    UART     ***/
        /*******************/

        /* Enable USART interrupts */
        NVIC_SetPriority(LWGSM_USART_IRQ, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0x05, 0x00));
        NVIC_EnableIRQ(LWGSM_USART_IRQ);

        /* Configure UART */
        LL_USART_DeInit(LWGSM_USART);
        LL_USART_StructInit(&usart_init);
        usart_init.BaudRate = baudrate;
        usart_init.DataWidth = LL_USART_DATAWIDTH_8B;
        usart_init.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
        usart_init.OverSampling = LL_USART_OVERSAMPLING_16;
        usart_init.Parity = LL_USART_PARITY_NONE;
        usart_init.StopBits = LL_USART_STOPBITS_1;
        usart_init.TransferDirection = LL_USART_DIRECTION_TX_RX;
        LL_USART_Init(LWGSM_USART, &usart_init);

        /* Enable USART interrupts and DMA request */
        LL_USART_EnableIT_IDLE(LWGSM_USART);
        LL_USART_EnableIT_ERROR(LWGSM_USART);
        LL_USART_EnableDMAReq_RX(LWGSM_USART);
        LL_USART_EnableDMAReq_TX(LWGSM_USART);

        /* Reset DMA position */
        lwgsm_read_old_pos = 0;

        /* Start DMA and USART */
        LL_DMA_EnableStream(LWGSM_USART_DMA_RX, LWGSM_USART_DMA_RX_STREAM);
        LL_USART_Enable(LWGSM_USART);

        lwgsm_is_running = 1;
    } else {
        //tx_thread_sleep(10);
        //LL_USART_Disable(LWGSM_USART);
        //usart_init.BaudRate = baudrate;
        //LL_USART_Init(LWGSM_USART, &usart_init);
        //LL_USART_Enable(LWGSM_USART);
    }

    /* Create mbox and threads */
    if (lwgsm_usart_ll_mbox.tx_queue_id == TX_CLEAR_ID) {
        tx_queue_create(&lwgsm_usart_ll_mbox, "lwgsm_ll_queue", sizeof(void *) / sizeof(ULONG), lwgsm_usart_ll_mbox_mem, sizeof(lwgsm_usart_ll_mbox_mem));
    }
    if (lwgsm_read_thread.tx_thread_id == TX_CLEAR_ID) {
        tx_thread_create(&lwgsm_read_thread, "lwgsm_lwgsm_read_thread", prv_lwgsm_read_thread_entry, 0,
                lwgsm_read_thread_stack, sizeof(lwgsm_read_thread_stack),
                TX_MAX_PRIORITIES / 2 - 1, TX_MAX_PRIORITIES / 2 - 1,
                TX_NO_TIME_SLICE, TX_AUTO_START);
    }
}

#if defined(LWGSM_RST_PIN)

/**
 * \brief           Hardware reset callback
 * \param[in]       state: Set to `1` to enable reset, `0` to release
 */
static uint8_t
prv_reset_device(uint8_t state) {
    if (state) {                                /* Activate reset line */
        //LL_GPIO_ResetOutputPin(LWGSM_RESET_PORT, LWGSM_RESET_PIN);
    } else {
        //LL_GPIO_SetOutputPin(LWGSM_RESET_PORT, LWGSM_RESET_PIN);
    }
    return 1;
}

#endif /* defined(LWGSM_RST_PIN) */

/**
 * \brief           Send data to ESP device over UART
 * \param[in]       data: Pointer to data to send
 * \param[in]       len: Number of bytes to send
 * \return          Number of bytes sent
 */
static size_t
prv_send_data(const void* data, size_t len) {
    const uint8_t* d = data;
    uint8_t use_dma = 1;

    /*
     * When in DMA TX mode, application writes 
     * TX data to ring buffer for which DMA certainly has access to.
     * 
     * As it is a non-blocking TX (we don't wait for finish),
     * writing to buffer is faster than writing over UART hence
     * we need to find a mechanism to be able to still write as much as fast,
     * if such event happens.
     * 
     * Writes to buffer are checked, and when no memory is available to write full data:
     * - Try to force transfer (if not already on-going)
     * - Yield thread and wait for next-time run
     * 
     * In the meantime, DMA will trigger TC complete interrupt
     * and clean-up used memory, ready for next transfers.
     * 
     * To avoid such complications, allocate > 1kB memory for buffer
     */
    if (use_dma) {
        size_t written = 0;
        do {
            written += lwrb_write(&lwgsm_tx_rb, &d[written], len - written);
            if (written < len) {
                prv_start_tx_transfer();
                tx_thread_relinquish();
            }
        } while (written < len);
        prv_start_tx_transfer();
    } else {   
        for (size_t i = 0; i < len; ++i, ++d) {
            LL_USART_TransmitData8(LWGSM_USART, *d);
            while (!LL_USART_IsActiveFlag_TXE(LWGSM_USART)) {}
        }
    }
    return len;
}

/**
 * \brief           Callback function called from initialization process
 */
lwgsmr_t
lwgsm_ll_init(lwgsm_ll_t* ll) {
    if (!lwgsm_initialized) {
        ll->send_fn = prv_send_data;            /* Set callback function to send data */
#if defined(LWGSM_RST_PIN)
        ll->reset_fn = prv_reset_device;        /* Set callback for hardware reset */
#endif /* defined(LWGSM_RST_PIN) */

        /* Initialize buffer for TX */
        lwgsm_tx_len = 0;
        lwrb_init(&lwgsm_tx_rb, lwgsm_tx_rb_data, sizeof(lwgsm_tx_rb_data));
    }
    prv_configure_uart(ll->uart.baudrate);      /* Initialize UART for communication */
    lwgsm_initialized = 1;
    return lwgsmOK;
}

/**
 * \brief           Callback function to de-init low-level communication part
 */
lwgsmr_t
lwgsm_ll_deinit(lwgsm_ll_t* ll) {
    LL_USART_Disable(LWGSM_USART);
    tx_queue_delete(&lwgsm_usart_ll_mbox);
    tx_thread_delete(&lwgsm_read_thread);

    lwgsm_initialized = 0;
    LWGSM_UNUSED(ll);
    return lwgsmOK;
}

/**
 * \brief           UART global interrupt handler
 */
void
LWGSM_USART_IRQ_HANDLER(void) {
    /* Clear all potential flags */
    LL_USART_ClearFlag_IDLE(LWGSM_USART);
    LL_USART_ClearFlag_PE(LWGSM_USART);
    LL_USART_ClearFlag_FE(LWGSM_USART);
    LL_USART_ClearFlag_ORE(LWGSM_USART);
    LL_USART_ClearFlag_NE(LWGSM_USART);

    /* Write message to thread to wake-it up */
    if (lwgsm_usart_ll_mbox.tx_queue_id != TX_CLEAR_ID) {
        void* d = (void*)1;
        tx_queue_send(&lwgsm_usart_ll_mbox, &d, TX_NO_WAIT);
    }
}

/**
 * \brief           UART DMA RX stream handler
 */
void
LWGSM_USART_DMA_RX_IRQ_HANDLER(void) {
    LWGSM_USART_DMA_RX_CLEAR_TC;
    LWGSM_USART_DMA_RX_CLEAR_HT;
    LWGSM_USART_DMA_RX_CLEAR_TE;

    /* Write message to thread to wake-it up */
    if (lwgsm_usart_ll_mbox.tx_queue_id != TX_CLEAR_ID) {
        void* d = (void*)1;
        tx_queue_send(&lwgsm_usart_ll_mbox, &d, TX_NO_WAIT);
    }
}

/**
 * \brief           UART DMA RX stream handler
 */
void
LWGSM_USART_DMA_TX_IRQ_HANDLER(void) {
    /* React on TC event only */
    if (LL_DMA_IsEnabledIT_TC(LWGSM_USART_DMA_TX, LWGSM_USART_DMA_TX_STREAM) && LWGSM_USART_DMA_TX_IS_TC) {
        LWGSM_USART_DMA_TX_CLEAR_TC;

        lwrb_skip(&lwgsm_tx_rb, lwgsm_tx_len);
        lwgsm_tx_len = 0;
        prv_start_tx_transfer();
    }
}

#endif /* !__DOXYGEN__ */
