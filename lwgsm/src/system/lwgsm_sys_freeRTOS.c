/**
 * \file            lwgsm_sys_freeRTOS.c
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
 * Authors:          Tilen MAJERLE <tilen@majerle.eu>,
 *                   Ilya Kargapolov <d3vil.st@gmail.com>
 * Version:         v0.1.0
 */
#include "system/lwgsm_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#if !__DOXYGEN__

static SemaphoreHandle_t sys_mutex;

/**
 * \brief           Init system dependant parameters
 *
 * After this function is called,
 * all other system functions must be fully ready.
 *
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_sys_init(void) {
    sys_mutex = xSemaphoreCreateRecursiveMutex();
    xSemaphoreGive(sys_mutex);
    return 1;
}

/**
 * \brief           Get current time in units of milliseconds
 * \return          Current time in units of milliseconds
 */
uint32_t
lwgsm_sys_now(void) {
    return xTaskGetTickCount()/portTICK_PERIOD_MS;
}

/**
 * \brief           Protect middleware core
 *
 * Stack protection must support recursive mode.
 * This function may be called multiple times,
 * even if access has been granted before.
 *
 * \note            Most operating systems support recursive mutexes.
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_sys_protect(void) {
    return xSemaphoreTakeRecursive(sys_mutex, portMAX_DELAY) == pdTRUE;
}

/**
 * \brief           Unprotect middleware core
 *
 * This function must follow number of calls of \ref lwgsm_sys_protect
 * and unlock access only when counter reached back zero.
 *
 * \note            Most operating systems support recursive mutexes.
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_sys_unprotect(void) {
    xSemaphoreGive(sys_mutex);
    return 1;
}

/**
 * \brief           Create new recursive mutex
 * \note            Recursive mutex has to be created as it may be locked multiple times before unlocked
 * \param[out]      p: Pointer to mutex structure to allocate
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_sys_mutex_create(lwgsm_sys_mutex_t* p) {
    *p = xSemaphoreCreateRecursiveMutex();
    xSemaphoreGive(*p);
    return *p != NULL;
}

/**
 * \brief           Delete recursive mutex from system
 * \param[in]       p: Pointer to mutex structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_sys_mutex_delete(lwgsm_sys_mutex_t* p) {
    vSemaphoreDelete(*p);
    return *p == NULL;
}

/**
 * \brief           Lock recursive mutex, wait forever to lock
 * \param[in]       p: Pointer to mutex structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_sys_mutex_lock(lwgsm_sys_mutex_t* p) {
    return xSemaphoreTakeRecursive(*p, portMAX_DELAY) == pdTRUE;
}

/**
 * \brief           Unlock recursive mutex
 * \param[in]       p: Pointer to mutex structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_sys_mutex_unlock(lwgsm_sys_mutex_t* p) {
    return xSemaphoreGiveRecursive(*p) == pdTRUE;
}

/**
 * \brief           Check if mutex structure is valid system
 * \param[in]       p: Pointer to mutex structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_sys_mutex_isvalid(lwgsm_sys_mutex_t* p) {
    return p != NULL && *p != NULL;
}

/**
 * \brief           Set recursive mutex structure as invalid
 * \param[in]       p: Pointer to mutex structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_sys_mutex_invalid(lwgsm_sys_mutex_t* p) {
    *p = LWGSM_SYS_MUTEX_NULL;
    return 1;
}

/**
 * \brief           Create a new binary semaphore and set initial state
 * \note            Semaphore may only have `1` token available
 * \param[out]      p: Pointer to semaphore structure to fill with result
 * \param[in]       cnt: Count indicating default semaphore state:
 *                     `0`: Take semaphore token immediately
 *                     `1`: Keep token available
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_sys_sem_create(lwgsm_sys_sem_t* p, uint8_t cnt) {
    *p = xSemaphoreCreateBinary();
    xSemaphoreGive(*p);
    if (!cnt)
      xSemaphoreTake(*p, portMAX_DELAY);
    return *p != NULL;
}

/**
 * \brief           Delete binary semaphore
 * \param[in]       p: Pointer to semaphore structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_sys_sem_delete(lwgsm_sys_sem_t* p) {
    vSemaphoreDelete(*p);
    return *p == NULL;
}

/**
 * \brief           Wait for semaphore to be available
 * \param[in]       p: Pointer to semaphore structure
 * \param[in]       timeout: Timeout to wait in milliseconds. When `0` is applied, wait forever
 * \return          Number of milliseconds waited for semaphore to become available or
 *                      \ref LWGSM_SYS_TIMEOUT if not available within given time
 */
uint32_t
lwgsm_sys_sem_wait(lwgsm_sys_sem_t* p, uint32_t timeout) {
    uint32_t tick = xTaskGetTickCount();
    if (xSemaphoreTake(*p, !timeout ? portMAX_DELAY : timeout / portTICK_PERIOD_MS) == pdTRUE) {
      return (xTaskGetTickCount() - tick)/portTICK_PERIOD_MS;
    }
    return LWGSM_SYS_TIMEOUT;
}

/**
 * \brief           Release semaphore
 * \param[in]       p: Pointer to semaphore structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_sys_sem_release(lwgsm_sys_sem_t* p) {
    return xSemaphoreGive(*p) == pdTRUE;
}

/**
 * \brief           Check if semaphore is valid
 * \param[in]       p: Pointer to semaphore structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_sys_sem_isvalid(lwgsm_sys_sem_t* p) {
    return p != NULL && *p != NULL;
}

/**
 * \brief           Invalid semaphore
 * \param[in]       p: Pointer to semaphore structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_sys_sem_invalid(lwgsm_sys_sem_t* p) {
    *p = LWGSM_SYS_SEM_NULL;
    return 1;
}

/**
 * \brief           Create a new message queue with entry type of `void *`
 * \param[out]      b: Pointer to message queue structure
 * \param[in]       size: Number of entries for message queue to hold
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_sys_mbox_create(lwgsm_sys_mbox_t* b, size_t size) {
    *b = xQueueCreate(size, sizeof(void*));
    return *b != NULL;
}

/**
 * \brief           Delete message queue
 * \param[in]       b: Pointer to message queue structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_sys_mbox_delete(lwgsm_sys_mbox_t* b) {
    if (uxQueueMessagesWaiting(*b)) {
        return 0;
    }
    vQueueDelete(*b);
    return b == NULL;
}

/**
 * \brief           Put a new entry to message queue and wait until memory available
 * \param[in]       b: Pointer to message queue structure
 * \param[in]       m: Pointer to entry to insert to message queue
 * \return          Time in units of milliseconds needed to put a message to queue
 */
uint32_t
lwgsm_sys_mbox_put(lwgsm_sys_mbox_t* b, void* m) {
    uint32_t tick = xTaskGetTickCount();
    if (xQueueSend(*b, &m, portMAX_DELAY) == pdTRUE)
    {
      return (xTaskGetTickCount() - tick)/portTICK_PERIOD_MS;
    }
    return LWGSM_SYS_TIMEOUT;
}

/**
 * \brief           Get a new entry from message queue with timeout
 * \param[in]       b: Pointer to message queue structure
 * \param[in]       m: Pointer to pointer to result to save value from message queue to
 * \param[in]       timeout: Maximal timeout to wait for new message. When `0` is applied, wait for unlimited time
 * \return          Time in units of milliseconds needed to put a message to queue
 *                      or \ref LWGSM_SYS_TIMEOUT if it was not successful
 */
uint32_t
lwgsm_sys_mbox_get(lwgsm_sys_mbox_t* b, void** m, uint32_t timeout) {
    uint32_t tick = xTaskGetTickCount();
    if(xQueueReceive(*b, m, !timeout ? portMAX_DELAY : timeout / portTICK_PERIOD_MS) == pdTRUE) {
      return (xTaskGetTickCount() - tick)/portTICK_PERIOD_MS;
    }
    return LWGSM_SYS_TIMEOUT;
}

/**
 * \brief           Put a new entry to message queue without timeout (now or fail)
 * \param[in]       b: Pointer to message queue structure
 * \param[in]       m: Pointer to message to save to queue
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_sys_mbox_putnow(lwgsm_sys_mbox_t* b, void* m) {
    return xQueueSend(*b, &m, 0) == pdTRUE;
}

/**
 * \brief           Get an entry from message queue immediately
 * \param[in]       b: Pointer to message queue structure
 * \param[in]       m: Pointer to pointer to result to save value from message queue to
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_sys_mbox_getnow(lwgsm_sys_mbox_t* b, void** m) {
    return xQueueReceive(*b, m, 0) == pdTRUE;
}

/**
 * \brief           Check if message queue is valid
 * \param[in]       b: Pointer to message queue structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_sys_mbox_isvalid(lwgsm_sys_mbox_t* b) {
    return b != NULL && *b != NULL;
}

/**
 * \brief           Invalid message queue
 * \param[in]       b: Pointer to message queue structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_sys_mbox_invalid(lwgsm_sys_mbox_t* b) {
    *b = LWGSM_SYS_MBOX_NULL;
    return 1;
}

/**
 * \brief           Create a new thread
 * \param[out]      t: Pointer to thread identifier if create was successful.
 *                     It may be set to `NULL`
 * \param[in]       name: Name of a new thread
 * \param[in]       thread_func: Thread function to use as thread body
 * \param[in]       arg: Thread function argument
 * \param[in]       stack_size: Size of thread stack in uints of bytes. If set to 0, reserve default stack size
 * \param[in]       prio: Thread priority
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_sys_thread_create(lwgsm_sys_thread_t* t, const char* name, lwgsm_sys_thread_fn thread_func,
                      void* const arg, size_t stack_size, lwgsm_sys_thread_prio_t prio) {
    lwgsm_sys_thread_t id = NULL;
    if (xTaskCreate(thread_func,
                    name,
                    4096,
                    //stack_size/sizeof(portSTACK_TYPE)*2,
                    arg,
                    prio,
                    id) == pdPASS) {
      *t = id;
      return 1;
    }
    return 0;
}

/**
 * \brief           Terminate thread (shut it down and remove)
 * \param[in]       t: Pointer to thread handle to terminate.
 *                      If set to `NULL`, terminate current thread (thread from where function is called)
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_sys_thread_terminate(lwgsm_sys_thread_t* t) {
    vTaskDelete(t != NULL ? *t : NULL);
    return 1;
}

/**
 * \brief           Yield current thread
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_sys_thread_yield(void) {
    vTaskDelay(0);
    return 1;
}

#endif /* !__DOXYGEN__ */
