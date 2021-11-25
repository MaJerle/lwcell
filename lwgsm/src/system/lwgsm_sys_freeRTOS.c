/**
 * \file            lwgsm_sys_freertos.c
 * \brief           System dependant functions for FreeRTOS
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
 * Authors:         Tilen MAJERLE <tilen@majerle.eu>,
 *                  Ilya Kargapolov <d3vil.st@gmail.com>
 * Version:         v0.1.1
 */
#include "system/lwgsm_sys.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#if !__DOXYGEN__

/* Mutex ID for main protection */
static SemaphoreHandle_t sys_mutex;

typedef struct {
    void* d;
} freertos_mbox_t;

uint8_t
lwgsm_sys_init(void) {
    sys_mutex = xSemaphoreCreateMutex();
    return sys_mutex == NULL ? 0 : 1;
}

uint32_t
lwgsm_sys_now(void) {
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

uint8_t
lwgsm_sys_protect(void) {
    lwgsm_sys_mutex_lock(&sys_mutex);
    return 1;
}

uint8_t
lwgsm_sys_unprotect(void) {
    lwgsm_sys_mutex_unlock(&sys_mutex);
    return 1;
}

uint8_t
lwgsm_sys_mutex_create(lwgsm_sys_mutex_t* p) {
    *p = xSemaphoreCreateRecursiveMutex();
    return *p != NULL;
}

uint8_t
lwgsm_sys_mutex_delete(lwgsm_sys_mutex_t* p) {
    vSemaphoreDelete(*p);
    return 1;
}

uint8_t
lwgsm_sys_mutex_lock(lwgsm_sys_mutex_t* p) {
    return xSemaphoreTakeRecursive(*p, portMAX_DELAY) == pdPASS;
}

uint8_t
lwgsm_sys_mutex_unlock(lwgsm_sys_mutex_t* p) {
    return xSemaphoreGiveRecursive(*p) == pdPASS;
}

uint8_t
lwgsm_sys_mutex_isvalid(lwgsm_sys_mutex_t* p) {
    return p != NULL && *p != NULL;
}

uint8_t
lwgsm_sys_mutex_invalid(lwgsm_sys_mutex_t* p) {
    *p = LWGSM_SYS_MUTEX_NULL;
    return 1;
}

uint8_t
lwgsm_sys_sem_create(lwgsm_sys_sem_t* p, uint8_t cnt) {
    *p = xSemaphoreCreateBinary();

    if (*p != NULL && cnt) {
        xSemaphoreGive(*p);
    }
    return *p != NULL;
}

uint8_t
lwgsm_sys_sem_delete(lwgsm_sys_sem_t* p) {
    vSemaphoreDelete(*p);
    return 1;
}

uint32_t
lwgsm_sys_sem_wait(lwgsm_sys_sem_t* p, uint32_t timeout) {
    uint32_t t = xTaskGetTickCount();
    return xSemaphoreTake(*p, !timeout ? portMAX_DELAY : pdMS_TO_TICKS(timeout)) == pdPASS ? ((xTaskGetTickCount() - t) * portTICK_PERIOD_MS): LWGSM_SYS_TIMEOUT;
}

uint8_t
lwgsm_sys_sem_release(lwgsm_sys_sem_t* p) {
    return xSemaphoreGive(*p) == pdPASS;
}

uint8_t
lwgsm_sys_sem_isvalid(lwgsm_sys_sem_t* p) {
    return p != NULL && *p != NULL;
}

uint8_t
lwgsm_sys_sem_invalid(lwgsm_sys_sem_t* p) {
    *p = LWGSM_SYS_SEM_NULL;
    return 1;
}

uint8_t
lwgsm_sys_mbox_create(lwgsm_sys_mbox_t* b, size_t size) {
    *b = xQueueCreate(size, sizeof(freertos_mbox_t));
    return *b != NULL;
}

uint8_t
lwgsm_sys_mbox_delete(lwgsm_sys_mbox_t* b) {
    if (uxQueueMessagesWaiting(*b)) {
        return 0;
    }
    vQueueDelete(*b);
    return 1;
}

uint32_t
lwgsm_sys_mbox_put(lwgsm_sys_mbox_t* b, void* m) {
    freertos_mbox_t mb;
    uint32_t t = xTaskGetTickCount();

    mb.d = m;
    xQueueSend(*b, &mb, portMAX_DELAY);
    return xTaskGetTickCount() - t;
}

uint32_t
lwgsm_sys_mbox_get(lwgsm_sys_mbox_t* b, void** m, uint32_t timeout) {
    freertos_mbox_t mb;
    uint32_t t = xTaskGetTickCount();

    if (xQueueReceive(*b, &mb, !timeout ? portMAX_DELAY : pdMS_TO_TICKS(timeout))) {
        *m = mb.d;
        return (xTaskGetTickCount() - t) * portTICK_PERIOD_MS;
    }
    return LWGSM_SYS_TIMEOUT;
}

uint8_t
lwgsm_sys_mbox_putnow(lwgsm_sys_mbox_t* b, void* m) {
    freertos_mbox_t mb;

    mb.d = m;
    return xQueueSendFromISR(*b, &mb, 0) == pdPASS;
}

uint8_t
lwgsm_sys_mbox_getnow(lwgsm_sys_mbox_t* b, void** m) {
    freertos_mbox_t mb;

    if (xQueueReceive(*b, &mb, 0)) {
        *m = mb.d;
        return 1;
    }
    return 0;
}

uint8_t
lwgsm_sys_mbox_isvalid(lwgsm_sys_mbox_t* b) {
    return b != NULL && *b != NULL;
}

uint8_t
lwgsm_sys_mbox_invalid(lwgsm_sys_mbox_t* b) {
    *b = LWGSM_SYS_MBOX_NULL;
    return 1;
}

uint8_t
lwgsm_sys_thread_create(lwgsm_sys_thread_t* t, const char* name, lwgsm_sys_thread_fn thread_func, void* const arg, size_t stack_size, lwgsm_sys_thread_prio_t prio) {
    return xTaskCreate(thread_func, name, stack_size / sizeof(portSTACK_TYPE), arg, prio, t) == pdPASS ? 1 : 0;
}

uint8_t
lwgsm_sys_thread_terminate(lwgsm_sys_thread_t* t) {
    vTaskDelete(*t);
    return 1;
}

uint8_t
lwgsm_sys_thread_yield(void) {
    taskYIELD();
    return 1;
}

#endif /* !__DOXYGEN__ */