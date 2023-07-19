/**
 * \file            lwcell_ll_stm32f429zi_nucleo.c
 * \brief           Low-level communication with GSM device for STM32F429ZI-Nucleo using DMA
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

/*
 * Default UART configuration is:
 *
 * UART:                USART6
 * STM32 TX (GSM RX):   GPIOC, GPIO_PIN_6
 * STM32 RX (GSM TX):   GPIOC, GPIO_PIN_7
 * RESET:               GPIOC, GPIO_PIN_5
 *
 * USART_DMA:           DMA2
 * USART_DMA_STREAM:    DMA_STREAM_1
 * USART_DMA_CHANNEL:   DMA_CHANNEL_5
 */
#include "lwcell/lwcell_input.h"
#include "lwcell/lwcell_mem.h"
#include "lwcell/lwcell_types.h"
#include "system/lwcell_ll.h"

#if !__DOXYGEN__

#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_usart.h"

/* USART */
#define LWCELL_USART                   USART6
#define LWCELL_USART_CLK               LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART6)
#define LWCELL_USART_IRQ               USART6_IRQn
#define LWCELL_USART_IRQHANDLER        USART6_IRQHandler
#define LWCELL_USART_RDR_NAME          DR

/* DMA settings */
#define LWCELL_USART_DMA               DMA2
#define LWCELL_USART_DMA_CLK           LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2)
#define LWCELL_USART_DMA_RX_STREAM     LL_DMA_STREAM_1
#define LWCELL_USART_DMA_RX_CH         LL_DMA_CHANNEL_5
#define LWCELL_USART_DMA_RX_IRQ        DMA2_Stream1_IRQn
#define LWCELL_USART_DMA_RX_IRQHANDLER DMA2_Stream1_IRQHandler

/* DMA flags management */
#define LWCELL_USART_DMA_RX_IS_TC      LL_DMA_IsActiveFlag_TC1(LWCELL_USART_DMA)
#define LWCELL_USART_DMA_RX_IS_HT      LL_DMA_IsActiveFlag_HT1(LWCELL_USART_DMA)
#define LWCELL_USART_DMA_RX_CLEAR_TC   LL_DMA_ClearFlag_TC1(LWCELL_USART_DMA)
#define LWCELL_USART_DMA_RX_CLEAR_HT   LL_DMA_ClearFlag_HT1(LWCELL_USART_DMA)

/* USART TX PIN */
#define LWCELL_USART_TX_PORT_CLK       LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC)
#define LWCELL_USART_TX_PORT           GPIOC
#define LWCELL_USART_TX_PIN            LL_GPIO_PIN_6
#define LWCELL_USART_TX_PIN_AF         LL_GPIO_AF_8

/* USART RX PIN */
#define LWCELL_USART_RX_PORT_CLK       LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC)
#define LWCELL_USART_RX_PORT           GPIOC
#define LWCELL_USART_RX_PIN            LL_GPIO_PIN_7
#define LWCELL_USART_RX_PIN_AF         LL_GPIO_AF_8

/* RESET PIN */
#define LWCELL_RESET_PORT_CLK          LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC)
#define LWCELL_RESET_PORT              GPIOC
#define LWCELL_RESET_PIN               LL_GPIO_PIN_5

/* Include STM32 generic driver */
#include "../system/lwcell_ll_stm32.c"

#endif /* !__DOXYGEN__ */
