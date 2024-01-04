/**
 * \file            lwcell_ll_stm32h735g_dk_threadx.c
 * \brief           STM32H735-DK driver with ThreadX
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
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.1
 */

/*
 * This is special driver for STM32H735G-DK utilizing ThreadX operating system
 * and full DMA support for TX and RX operation.
 */
#include "lwcell/lwcell_input.h"
#include "lwcell/lwcell_mem.h"
#include "lwcell/lwcell_types.h"
#include "lwcell/lwcell_utils.h"
#include "lwrb/lwrb.h"
#include "mcu.h"
#include "system/lwcell_ll.h"
#include "system/lwcell_sys.h"
#include "tx_api.h"

#if !__DOXYGEN__

#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_dma.h"
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_rcc.h"
#include "stm32h7xx_ll_usart.h"

#if !LWCELL_CFG_INPUT_USE_PROCESS
#error "LWCELL_CFG_INPUT_USE_PROCESS must be enabled in `lwcell_opts.h` to use this driver."
#endif /* LWCELL_CFG_INPUT_USE_PROCESS */
#if !LWCELL_CFG_MEM_CUSTOM
#error "LWCELL_CFG_MEM_CUSTOM must be used instead. This driver does not set memory regions for LwESP."
#endif /* !LWCELL_CFG_MEM_CUSTOM */

/*
 * USART setup
 *
 * PF6 and PF7 are used together with external STMOD+ extension board
 */
#define LWCELL_USART                    UART7
#define LWCELL_USART_CLK_EN             LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART7)
#define LWCELL_USART_IRQ                UART7_IRQn
#define LWCELL_USART_IRQ_HANDLER        UART7_IRQHandler

/* TX DMA */
#define LWCELL_USART_DMA_TX             DMA1
#define LWCELL_USART_DMA_TX_STREAM      LL_DMA_STREAM_3
#define LWCELL_USART_DMA_TX_REQUEST     LL_DMAMUX1_REQ_UART7_TX
#define LWCELL_USART_DMA_TX_CLK_EN      LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1)
#define LWCELL_USART_DMA_TX_IRQ         DMA1_Stream3_IRQn
#define LWCELL_USART_DMA_TX_IRQ_HANDLER DMA1_Stream3_IRQHandler
#define LWCELL_USART_DMA_TX_IS_TC       LL_DMA_IsActiveFlag_TC3(LWCELL_USART_DMA_TX)
#define LWCELL_USART_DMA_TX_CLEAR_TC    LL_DMA_ClearFlag_TC3(LWCELL_USART_DMA_TX)
#define LWCELL_USART_DMA_TX_CLEAR_HT    LL_DMA_ClearFlag_HT3(LWCELL_USART_DMA_TX)
#define LWCELL_USART_DMA_TX_CLEAR_TE    LL_DMA_ClearFlag_TE3(LWCELL_USART_DMA_TX)

/* RX DMA */
#define LWCELL_USART_DMA_RX             DMA1
#define LWCELL_USART_DMA_RX_STREAM      LL_DMA_STREAM_2
#define LWCELL_USART_DMA_RX_REQUEST     LL_DMAMUX1_REQ_UART7_RX
#define LWCELL_USART_DMA_RX_CLK_EN      LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1)
#define LWCELL_USART_DMA_RX_IRQ         DMA1_Stream2_IRQn
#define LWCELL_USART_DMA_RX_IRQ_HANDLER DMA1_Stream2_IRQHandler
#define LWCELL_USART_DMA_RX_IS_TC       LL_DMA_IsActiveFlag_TC2(LWCELL_USART_DMA_RX)
#define LWCELL_USART_DMA_RX_IS_HT       LL_DMA_IsActiveFlag_HT2(LWCELL_USART_DMA_RX)
#define LWCELL_USART_DMA_RX_IS_TE       LL_DMA_IsActiveFlag_TE2(LWCELL_USART_DMA_RX)
#define LWCELL_USART_DMA_RX_CLEAR_TC    LL_DMA_ClearFlag_TC2(LWCELL_USART_DMA_RX)
#define LWCELL_USART_DMA_RX_CLEAR_HT    LL_DMA_ClearFlag_HT2(LWCELL_USART_DMA_RX)
#define LWCELL_USART_DMA_RX_CLEAR_TE    LL_DMA_ClearFlag_TE2(LWCELL_USART_DMA_RX)

/* GPIO configuration */
#define LWCELL_USART_TX_PORT            GPIOF
#define LWCELL_USART_TX_PIN             LL_GPIO_PIN_7
#define LWCELL_USART_TX_PORT_CLK_EN     LL_AHB1_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOF)
#define LWCELL_USART_TX_PIN_AF          LL_GPIO_AF_7
#define LWCELL_USART_RX_PORT            GPIOF
#define LWCELL_USART_RX_PIN             LL_GPIO_PIN_6
#define LWCELL_USART_RX_PORT_CLK_EN     LL_AHB1_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOF)
#define LWCELL_USART_RX_PIN_AF          LL_GPIO_AF_7

/* TX data buffers, must be 32-bytes aligned (cache) and in dma buffer section to make sure DMA has access to the memory region */
ALIGN_32BYTES(static uint8_t __attribute__((section(".dma_buffer"))) lwcell_tx_rb_data[2048]);
static lwrb_t lwcell_tx_rb;
volatile size_t lwcell_tx_len;

/*
 * Max number of bytes to transmit in one DMA transfer
 *
 * See https://github.com/MaJerle/stm32-usart-uart-dma-rx-tx
 * for detailed explanation about impact of this number
 */
#define LWCELL_LL_MAX_TX_LEN 64

/* Raw DMA memory for UART received data */
ALIGN_32BYTES(static uint8_t __attribute__((section(".dma_buffer"))) lwcell_usart_rx_dma_buffer[256]);

/* USART thread for read and data processing */
static void prv_lwcell_read_thread_entry(ULONG arg);
static TX_THREAD lwcell_read_thread;
static UCHAR lwcell_read_thread_stack[4 * LWCELL_SYS_THREAD_SS];
static volatile size_t lwcell_read_old_pos = 0;
static TX_EVENT_FLAGS_GROUP lwcell_ll_event_group;

/* List of flags for read */
#define LWCELL_LL_FLAG_DATA ((ULONG)0x000000001)

/* Status variables */
static uint8_t lwcell_is_running = 0;
static uint8_t lwcell_initialized = 0;

/**
 * \brief           USART data processing thread
 * This is the thread used to enter received data from UART to the LwESP stack for further processing
 * \param[in]       arg: User argument
 */
static void
prv_lwcell_read_thread_entry(ULONG arg) {
    size_t pos;

    LWCELL_UNUSED(arg);

    while (1) {
        ULONG flags;

        /* Wait for any flag from either DMA or UART interrupts */
        tx_event_flags_get(&lwcell_ll_event_group, (ULONG)-1, TX_OR_CLEAR, &flags, TX_WAIT_FOREVER);

        /* Read data */
        pos = sizeof(lwcell_usart_rx_dma_buffer) - LL_DMA_GetDataLength(LWCELL_USART_DMA_RX, LWCELL_USART_DMA_RX_STREAM);
        if (pos != lwcell_read_old_pos && lwcell_is_running) {
            SCB_InvalidateDCache_by_Addr(lwcell_usart_rx_dma_buffer, sizeof(lwcell_usart_rx_dma_buffer));
            if (pos > lwcell_read_old_pos) {
                lwcell_input_process(&lwcell_usart_rx_dma_buffer[lwcell_read_old_pos], pos - lwcell_read_old_pos);
            } else {
                lwcell_input_process(&lwcell_usart_rx_dma_buffer[lwcell_read_old_pos],
                                    sizeof(lwcell_usart_rx_dma_buffer) - lwcell_read_old_pos);
                if (pos > 0) {
                    lwcell_input_process(&lwcell_usart_rx_dma_buffer[0], pos);
                }
            }
            lwcell_read_old_pos = pos;
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
    if (lwcell_tx_len == 0 && (lwcell_tx_len = lwrb_get_linear_block_read_length(&lwcell_tx_rb)) > 0) {
        const void* d = lwrb_get_linear_block_read_address(&lwcell_tx_rb);

        /* Limit tx len up to some size to optimize buffer read/write process */
        lwcell_tx_len = LWCELL_MIN(lwcell_tx_len, LWCELL_LL_MAX_TX_LEN);

        /* Cleanup cache to make sure we have latest data in memory visible by DMA */
        SCB_CleanDCache_by_Addr((void*)d, lwcell_tx_len);

        /* Clear all DMA flags prior transfer */
        LWCELL_USART_DMA_TX_CLEAR_TC;
        LWCELL_USART_DMA_TX_CLEAR_HT;
        LWCELL_USART_DMA_TX_CLEAR_TE;

        /* Configure DMA */
        LL_DMA_SetMemoryAddress(LWCELL_USART_DMA_TX, LWCELL_USART_DMA_TX_STREAM, (uint32_t)d);
        LL_DMA_SetDataLength(LWCELL_USART_DMA_TX, LWCELL_USART_DMA_TX_STREAM, lwcell_tx_len);

        /* Enable instances */
        LL_DMA_EnableStream(LWCELL_USART_DMA_TX, LWCELL_USART_DMA_TX_STREAM);
    }
    __set_PRIMASK(primask);
}

/**
 * \brief           Configure UART using DMA for receive in double buffer mode and IDLE line detection
 * \param[in]       baudrate: Baudrate for UART communication
 */
static void
prv_configure_uart(uint32_t baudrate) {
    LL_GPIO_InitTypeDef gpio_init = {0};
    LL_USART_InitTypeDef usart_init = {0};

    if (!lwcell_initialized) {
        /* Enable peripheral clocks */
        LWCELL_USART_CLK_EN;
        LWCELL_USART_DMA_RX_CLK_EN;
        LWCELL_USART_DMA_TX_CLK_EN;
        LWCELL_USART_TX_PORT_CLK_EN;
        LWCELL_USART_RX_PORT_CLK_EN;

        /* Global pin configuration */
        LL_GPIO_StructInit(&gpio_init);
        gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        gpio_init.Pull = LL_GPIO_PULL_UP;
        gpio_init.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
        gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;

        /* TX PIN */
        gpio_init.Pin = LWCELL_USART_TX_PIN;
        gpio_init.Alternate = LWCELL_USART_TX_PIN_AF;
        LL_GPIO_Init(LWCELL_USART_TX_PORT, &gpio_init);

        /* RX PIN */
        gpio_init.Pin = LWCELL_USART_RX_PIN;
        gpio_init.Alternate = LWCELL_USART_RX_PIN_AF;
        LL_GPIO_Init(LWCELL_USART_RX_PORT, &gpio_init);

        /*******************/
        /*** UART RX DMA ***/
        /*******************/

        /* Enable DMA interrupts */
        NVIC_SetPriority(LWCELL_USART_DMA_RX_IRQ, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0x05, 0x00));
        NVIC_EnableIRQ(LWCELL_USART_DMA_RX_IRQ);

        /* Configure DMA */
        LL_DMA_SetPeriphRequest(LWCELL_USART_DMA_RX, LWCELL_USART_DMA_RX_STREAM, LWCELL_USART_DMA_RX_REQUEST);
        LL_DMA_SetDataTransferDirection(LWCELL_USART_DMA_RX, LWCELL_USART_DMA_RX_STREAM,
                                        LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
        LL_DMA_SetStreamPriorityLevel(LWCELL_USART_DMA_RX, LWCELL_USART_DMA_RX_STREAM, LL_DMA_PRIORITY_MEDIUM);
        LL_DMA_SetMode(LWCELL_USART_DMA_RX, LWCELL_USART_DMA_RX_STREAM, LL_DMA_MODE_CIRCULAR);
        LL_DMA_SetPeriphIncMode(LWCELL_USART_DMA_RX, LWCELL_USART_DMA_RX_STREAM, LL_DMA_PERIPH_NOINCREMENT);
        LL_DMA_SetMemoryIncMode(LWCELL_USART_DMA_RX, LWCELL_USART_DMA_RX_STREAM, LL_DMA_MEMORY_INCREMENT);
        LL_DMA_SetPeriphSize(LWCELL_USART_DMA_RX, LWCELL_USART_DMA_RX_STREAM, LL_DMA_PDATAALIGN_BYTE);
        LL_DMA_SetMemorySize(LWCELL_USART_DMA_RX, LWCELL_USART_DMA_RX_STREAM, LL_DMA_MDATAALIGN_BYTE);
        LL_DMA_DisableFifoMode(LWCELL_USART_DMA_RX, LWCELL_USART_DMA_RX_STREAM);
        LL_DMA_SetPeriphAddress(LWCELL_USART_DMA_RX, LWCELL_USART_DMA_RX_STREAM,
                                LL_USART_DMA_GetRegAddr(LWCELL_USART, LL_USART_DMA_REG_DATA_RECEIVE));
        LL_DMA_SetMemoryAddress(LWCELL_USART_DMA_RX, LWCELL_USART_DMA_RX_STREAM, (uint32_t)lwcell_usart_rx_dma_buffer);
        LL_DMA_SetDataLength(LWCELL_USART_DMA_RX, LWCELL_USART_DMA_RX_STREAM, sizeof(lwcell_usart_rx_dma_buffer));

        /* Enable DMA interrupts */
        LL_DMA_EnableIT_HT(LWCELL_USART_DMA_RX, LWCELL_USART_DMA_RX_STREAM);
        LL_DMA_EnableIT_TC(LWCELL_USART_DMA_RX, LWCELL_USART_DMA_RX_STREAM);
        LL_DMA_EnableIT_TE(LWCELL_USART_DMA_RX, LWCELL_USART_DMA_RX_STREAM);
        LL_DMA_EnableIT_FE(LWCELL_USART_DMA_RX, LWCELL_USART_DMA_RX_STREAM);
        LL_DMA_EnableIT_DME(LWCELL_USART_DMA_RX, LWCELL_USART_DMA_RX_STREAM);

        /*******************/
        /*** UART TX DMA ***/
        /*******************/

        /* Enable DMA interrupts */
        NVIC_SetPriority(LWCELL_USART_DMA_TX_IRQ, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0x05, 0x00));
        NVIC_EnableIRQ(LWCELL_USART_DMA_TX_IRQ);

        /* Configure DMA */
        LL_DMA_SetPeriphRequest(LWCELL_USART_DMA_TX, LWCELL_USART_DMA_TX_STREAM, LWCELL_USART_DMA_TX_REQUEST);
        LL_DMA_SetDataTransferDirection(LWCELL_USART_DMA_TX, LWCELL_USART_DMA_TX_STREAM,
                                        LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
        LL_DMA_SetStreamPriorityLevel(LWCELL_USART_DMA_TX, LWCELL_USART_DMA_TX_STREAM, LL_DMA_PRIORITY_MEDIUM);
        LL_DMA_SetMode(LWCELL_USART_DMA_TX, LWCELL_USART_DMA_TX_STREAM, LL_DMA_MODE_NORMAL);
        LL_DMA_SetPeriphIncMode(LWCELL_USART_DMA_TX, LWCELL_USART_DMA_TX_STREAM, LL_DMA_PERIPH_NOINCREMENT);
        LL_DMA_SetMemoryIncMode(LWCELL_USART_DMA_TX, LWCELL_USART_DMA_TX_STREAM, LL_DMA_MEMORY_INCREMENT);
        LL_DMA_SetPeriphSize(LWCELL_USART_DMA_TX, LWCELL_USART_DMA_TX_STREAM, LL_DMA_PDATAALIGN_BYTE);
        LL_DMA_SetMemorySize(LWCELL_USART_DMA_TX, LWCELL_USART_DMA_TX_STREAM, LL_DMA_MDATAALIGN_BYTE);
        LL_DMA_DisableFifoMode(LWCELL_USART_DMA_TX, LWCELL_USART_DMA_TX_STREAM);
        LL_DMA_SetPeriphAddress(LWCELL_USART_DMA_TX, LWCELL_USART_DMA_TX_STREAM,
                                LL_USART_DMA_GetRegAddr(LWCELL_USART, LL_USART_DMA_REG_DATA_TRANSMIT));

        /* Enable DMA interrupts */
        LL_DMA_EnableIT_TC(LWCELL_USART_DMA_TX, LWCELL_USART_DMA_TX_STREAM);
        LL_DMA_EnableIT_TE(LWCELL_USART_DMA_TX, LWCELL_USART_DMA_TX_STREAM);

        /*******************/
        /***    UART     ***/
        /*******************/

        /* Enable USART interrupts */
        NVIC_SetPriority(LWCELL_USART_IRQ, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0x05, 0x00));
        NVIC_EnableIRQ(LWCELL_USART_IRQ);

        /* Configure UART */
        LL_USART_DeInit(LWCELL_USART);
        LL_USART_StructInit(&usart_init);
        usart_init.BaudRate = baudrate;
        usart_init.DataWidth = LL_USART_DATAWIDTH_8B;
        usart_init.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
        usart_init.OverSampling = LL_USART_OVERSAMPLING_16;
        usart_init.Parity = LL_USART_PARITY_NONE;
        usart_init.StopBits = LL_USART_STOPBITS_1;
        usart_init.TransferDirection = LL_USART_DIRECTION_TX_RX;
        LL_USART_Init(LWCELL_USART, &usart_init);

        /* Enable USART interrupts and DMA request */
        LL_USART_EnableIT_IDLE(LWCELL_USART);
        LL_USART_EnableIT_ERROR(LWCELL_USART);
        LL_USART_EnableDMAReq_RX(LWCELL_USART);
        LL_USART_EnableDMAReq_TX(LWCELL_USART);

        /* Reset DMA position */
        lwcell_read_old_pos = 0;

        /* Start DMA and USART */
        LL_DMA_EnableStream(LWCELL_USART_DMA_RX, LWCELL_USART_DMA_RX_STREAM);
        LL_USART_Enable(LWCELL_USART);

        /* Create mbox and read threads */
        tx_event_flags_create(&lwcell_ll_event_group, "lwcell_ll_group");
        tx_thread_create(&lwcell_read_thread, "lwcell_read_thread", prv_lwcell_read_thread_entry, 0,
                         lwcell_read_thread_stack, sizeof(lwcell_read_thread_stack), TX_MAX_PRIORITIES / 2 - 1,
                         TX_MAX_PRIORITIES / 2 - 1, TX_NO_TIME_SLICE, TX_AUTO_START);

        lwcell_is_running = 1;
    } else {
        //tx_thread_sleep(10);
        //LL_USART_Disable(LWCELL_USART);
        //usart_init.BaudRate = baudrate;
        //LL_USART_Init(LWCELL_USART, &usart_init);
        //LL_USART_Enable(LWCELL_USART);
    }
}

#if defined(LWCELL_RST_PIN)

/**
 * \brief           Hardware reset callback
 * \param[in]       state: Set to `1` to enable reset, `0` to release
 */
static uint8_t
prv_reset_device(uint8_t state) {
    if (state) { /* Activate reset line */
        //LL_GPIO_ResetOutputPin(LWCELL_RESET_PORT, LWCELL_RESET_PIN);
    } else {
        //LL_GPIO_SetOutputPin(LWCELL_RESET_PORT, LWCELL_RESET_PIN);
    }
    return 1;
}

#endif /* defined(LWCELL_RST_PIN) */

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
            written += lwrb_write(&lwcell_tx_rb, &d[written], len - written);
            if (written < len) {
                prv_start_tx_transfer();
                tx_thread_relinquish();
            }
        } while (written < len);
        prv_start_tx_transfer();
    } else {
        for (size_t i = 0; i < len; ++i, ++d) {
            LL_USART_TransmitData8(LWCELL_USART, *d);
            while (!LL_USART_IsActiveFlag_TXE(LWCELL_USART)) {}
        }
    }
    return len;
}

/**
 * \brief           Callback function called from initialization process
 */
lwcellr_t
lwcell_ll_init(lwcell_ll_t* ll) {
    if (!lwcell_initialized) {
        ll->send_fn = prv_send_data; /* Set callback function to send data */
#if defined(LWCELL_RST_PIN)
        ll->reset_fn = prv_reset_device; /* Set callback for hardware reset */
#endif                                   /* defined(LWCELL_RST_PIN) */

        /* Initialize buffer for TX */
        lwcell_tx_len = 0;
        lwrb_init(&lwcell_tx_rb, lwcell_tx_rb_data, sizeof(lwcell_tx_rb_data));
    }
    prv_configure_uart(ll->uart.baudrate); /* Initialize UART for communication */
    lwcell_initialized = 1;
    return lwcellOK;
}

/**
 * \brief           Callback function to de-init low-level communication part
 */
lwcellr_t
lwcell_ll_deinit(lwcell_ll_t* ll) {
    LL_USART_Disable(LWCELL_USART);
    tx_event_flags_delete(&lwcell_ll_event_group);
    tx_thread_delete(&lwcell_read_thread);

    lwcell_initialized = 0;
    LWCELL_UNUSED(ll);
    return lwcellOK;
}

/**
 * \brief           UART global interrupt handler
 */
void
LWCELL_USART_IRQ_HANDLER(void) {
    /* Clear all potential flags */
    LL_USART_ClearFlag_IDLE(LWCELL_USART);
    LL_USART_ClearFlag_PE(LWCELL_USART);
    LL_USART_ClearFlag_FE(LWCELL_USART);
    LL_USART_ClearFlag_ORE(LWCELL_USART);
    LL_USART_ClearFlag_NE(LWCELL_USART);

    /* Set flag to wakeup thread */
    if (lwcell_ll_event_group.tx_event_flags_group_id != TX_CLEAR_ID) {
        tx_event_flags_set(&lwcell_ll_event_group, LWCELL_LL_FLAG_DATA, TX_OR);
    }
}

/**
 * \brief           UART DMA RX stream handler
 */
void
LWCELL_USART_DMA_RX_IRQ_HANDLER(void) {
    LWCELL_USART_DMA_RX_CLEAR_TC;
    LWCELL_USART_DMA_RX_CLEAR_HT;
    LWCELL_USART_DMA_RX_CLEAR_TE;

    /* Set flag to wakeup thread */
    if (lwcell_ll_event_group.tx_event_flags_group_id != TX_CLEAR_ID) {
        tx_event_flags_set(&lwcell_ll_event_group, LWCELL_LL_FLAG_DATA, TX_OR);
    }
}

/**
 * \brief           UART DMA RX stream handler
 */
void
LWCELL_USART_DMA_TX_IRQ_HANDLER(void) {
    /* React on TC event only */
    if (LL_DMA_IsEnabledIT_TC(LWCELL_USART_DMA_TX, LWCELL_USART_DMA_TX_STREAM) && LWCELL_USART_DMA_TX_IS_TC) {
        LWCELL_USART_DMA_TX_CLEAR_TC;

        lwrb_skip(&lwcell_tx_rb, lwcell_tx_len);
        lwcell_tx_len = 0;
        prv_start_tx_transfer();
    }
}

#endif /* !__DOXYGEN__ */
