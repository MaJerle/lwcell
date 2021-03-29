/**
 * \file            lwgsm_ll_esp32.c
 * \brief           Low-level communication with GSM device for Esp32
 */

/*
 * Copyright (c) 2021 Tilen MAJERLE and Ilya Kargapolov
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
 * Authors:          Tilen MAJERLE <tilen@majerle.eu>,
 *                   Ilya Kargapolov <d3vil.st@gmail.com>
 * Version:         v0.1.0
 */
#include "system/lwgsm_ll.h"
#include "lwgsm/lwgsm.h"
#include "lwgsm/lwgsm_mem.h"
#include "lwgsm/lwgsm_input.h"
#include "driver/uart.h"
#include "esp_log.h"

#if !__DOXYGEN__

/* Define TAG for log messages */
#define TAG "lwGSM"

/* Defines ESP uart number to use */
#define GSM_UART_NUM UART_NUM_1

#if !defined(LWGSM_USART_DMA_RX_BUFF_SIZE)
#define LWGSM_USART_DMA_RX_BUFF_SIZE      0x1000
#endif /* !defined(LWGSM_USART_DMA_RX_BUFF_SIZE) */

#if !defined(LWGSM_MEM_SIZE)
#define LWGSM_MEM_SIZE                    0x1000
#endif /* !defined(LWGSM_MEM_SIZE) */

static QueueHandle_t gsm_uart_queue;

static uint8_t initialized = 0;

char* uart_buffer[LWGSM_USART_DMA_RX_BUFF_SIZE];

/**
 * \brief           Send data to GSM device, function called from GSM stack when we have data to send
 * \param[in]       data: Pointer to data to send
 * \param[in]       len: Number of bytes to send
 * \return          Number of bytes sent
 */
static size_t
send_data(const void* data, size_t len) {
    /* Implement send function here */
    if (len) {
      len = uart_write_bytes(GSM_UART_NUM, (const char*) data, len);
      //uart_wait_tx_done(GSM_UART_NUM, portMAX_DELAY);
      ESP_LOG_BUFFER_HEXDUMP(">", data, len, ESP_LOG_DEBUG);
    }
    return len;                                 /* Return number of bytes actually sent to AT port */
}

static void uart_event_task(void *pvParameters)
{
  uart_event_t event;
  size_t buffer_len;

  for(;;) {
    //Waiting for UART event.
    if(xQueueReceive(gsm_uart_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
      switch(event.type) {
        case UART_DATA:
          uart_get_buffered_data_len(GSM_UART_NUM, &buffer_len);
          buffer_len = uart_read_bytes(GSM_UART_NUM, (void*) uart_buffer, buffer_len, portMAX_DELAY);
          ESP_LOG_BUFFER_HEXDUMP("<", uart_buffer, buffer_len, ESP_LOG_DEBUG);
          if (buffer_len) {
            #if LWGSM_CFG_INPUT_USE_PROCESS
              lwgsm_input_process(uart_buffer, buffer_len);
            #else
              lwgsm_input(uart_buffer, buffer_len);
            #endif
          }
          break;
        case UART_FIFO_OVF:
          ESP_LOGW(TAG, "UART_FIFO_OVF");
          uart_flush_input(GSM_UART_NUM);
          xQueueReset(gsm_uart_queue);
          break;
        case UART_BUFFER_FULL:
          ESP_LOGW(TAG, "UART_BUFFER_FULL");
          uart_flush_input(GSM_UART_NUM);
          xQueueReset(gsm_uart_queue);
          break;
        default:
          break;
      }
    }
  }
  vTaskDelete(NULL);
}
/**
 * \brief           Configure UART using DMA for receive in double buffer mode and IDLE line detection
 */
static void
configure_uart(uint32_t baudrate) {
  uart_config_t uart_config = {
    .baud_rate = baudrate,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .source_clk = UART_SCLK_REF_TICK,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
  };
  ESP_ERROR_CHECK(uart_driver_install(GSM_UART_NUM, LWGSM_USART_DMA_RX_BUFF_SIZE * 2,
                                      LWGSM_USART_DMA_RX_BUFF_SIZE * 2, 20, &gsm_uart_queue, 0));
  ESP_ERROR_CHECK(uart_param_config(GSM_UART_NUM, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(GSM_UART_NUM, CONFIG_LWGSM_TX, CONFIG_LWGSM_RX, 0, 0));
}
/**
 * \brief           Callback function called from initialization process
 *
 * \note            This function may be called multiple times if AT baudrate is changed from application.
 *                  It is important that every configuration except AT baudrate is configured only once!
 *
 * \note            This function may be called from different threads in GSM stack when using OS.
 *                  When \ref LWGSM_CFG_INPUT_USE_PROCESS is set to 1, this function may be called from user UART thread.
 *
 * \param[in,out]   ll: Pointer to \ref lwgsm_ll_t structure to fill data for communication functions
 * \return          lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_ll_init(lwgsm_ll_t* ll) {
#if !LWGSM_CFG_MEM_CUSTOM
    /* Step 1: Configure memory for dynamic allocations */
    static uint8_t memory[0x10000];             /* Create memory for dynamic allocations with specific size */

    /*
     * Create region(s) of memory.
     * If device has internal/external memory available,
     * multiple memories may be used
     */
    lwgsm_mem_region_t mem_regions[] = {
        { memory, sizeof(memory) }
    };
    if (!initialized) {
        lwgsm_mem_assignmemory(mem_regions, LWGSM_ARRAYSIZE(mem_regions));  /* Assign memory for allocations to GSM library */
    }
#endif /* !LWGSM_CFG_MEM_CUSTOM */

    /* Step 2: Set AT port send function to use when we have data to transmit */
    if (!initialized) {
        ll->send_fn = send_data;                /* Set callback function to send data */
    }

    /* Step 3: Configure AT port to be able to send/receive data to/from GSM device */
    configure_uart(ll->uart.baudrate);          /* Initialize UART for communication */
    xTaskCreate(uart_event_task, "uart_lwgsm_task0", 4096, NULL, 5, NULL);
    initialized = 1;
    return lwgsmOK;
}

/**
 * \brief           Callback function to de-init low-level communication part
 * \param[in,out]   ll: Pointer to \ref lwgsm_ll_t structure to fill data for communication functions
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_ll_deinit(lwgsm_ll_t* ll) {
    ESP_ERROR_CHECK(uart_driver_delete(GSM_UART_NUM));
    initialized = 0;                            /* Clear initialized flag */
    return lwgsmOK;
}

#endif /* !__DOXYGEN__ */
