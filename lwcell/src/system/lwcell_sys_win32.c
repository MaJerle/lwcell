/**
 * \file            lwcell_sys_win32.c
 * \brief           System dependant functions for WIN32
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
#include <stdlib.h>
#include <string.h>
#include "lwcell/lwcell_private.h"
#include "system/lwcell_sys.h"

#if !__DOXYGEN__

/**
 * \brief           Custom message queue implementation for WIN32
 */
typedef struct {
    lwcell_sys_sem_t sem_not_empty; /*!< Semaphore indicates not empty */
    lwcell_sys_sem_t sem_not_full;  /*!< Semaphore indicates not full */
    lwcell_sys_sem_t sem;           /*!< Semaphore to lock access */
    size_t in, out, size;
    void* entries[1];
} win32_mbox_t;

static LARGE_INTEGER freq, sys_start_time;
static lwcell_sys_mutex_t sys_mutex; /* Mutex ID for main protection */

static uint8_t
mbox_is_full(win32_mbox_t* m) {
    size_t size = 0;
    if (m->in > m->out) {
        size = (m->in - m->out);
    } else if (m->out > m->in) {
        size = m->size - m->out + m->in;
    }
    return size == m->size - 1;
}

static uint8_t
mbox_is_empty(win32_mbox_t* m) {
    return m->in == m->out;
}

static uint32_t
osKernelSysTick(void) {
    LONGLONG ret;
    LARGE_INTEGER now;

    QueryPerformanceFrequency(&freq); /* Get frequency */
    QueryPerformanceCounter(&now);    /* Get current time */
    ret = now.QuadPart - sys_start_time.QuadPart;
    return (uint32_t)(((ret)*1000) / freq.QuadPart);
}

uint8_t
lwcell_sys_init(void) {
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&sys_start_time);

    lwcell_sys_mutex_create(&sys_mutex);
    return 1;
}

uint32_t
lwcell_sys_now(void) {
    return osKernelSysTick();
}

uint8_t
lwcell_sys_protect(void) {
    lwcell_sys_mutex_lock(&sys_mutex);
    return 1;
}

uint8_t
lwcell_sys_unprotect(void) {
    lwcell_sys_mutex_unlock(&sys_mutex);
    return 1;
}

uint8_t
lwcell_sys_mutex_create(lwcell_sys_mutex_t* p) {
    *p = CreateMutex(NULL, FALSE, NULL);
    return *p != NULL;
}

uint8_t
lwcell_sys_mutex_delete(lwcell_sys_mutex_t* p) {
    return CloseHandle(*p);
}

uint8_t
lwcell_sys_mutex_lock(lwcell_sys_mutex_t* p) {
    DWORD ret;
    ret = WaitForSingleObject(*p, INFINITE);
    if (ret != WAIT_OBJECT_0) {
        return 0;
    }
    return 1;
}

uint8_t
lwcell_sys_mutex_unlock(lwcell_sys_mutex_t* p) {
    return (uint8_t)ReleaseMutex(*p);
}

uint8_t
lwcell_sys_mutex_isvalid(lwcell_sys_mutex_t* p) {
    return p != NULL && *p != NULL;
}

uint8_t
lwcell_sys_mutex_invalid(lwcell_sys_mutex_t* p) {
    *p = LWCELL_SYS_MUTEX_NULL;
    return 1;
}

uint8_t
lwcell_sys_sem_create(lwcell_sys_sem_t* p, uint8_t cnt) {
    HANDLE h;
    h = CreateSemaphore(NULL, !!cnt, 1, NULL);
    *p = h;
    return *p != NULL;
}

uint8_t
lwcell_sys_sem_delete(lwcell_sys_sem_t* p) {
    return CloseHandle(*p);
}

uint32_t
lwcell_sys_sem_wait(lwcell_sys_sem_t* p, uint32_t timeout) {
    DWORD ret;

    if (timeout == 0) {
        ret = WaitForSingleObject(*p, INFINITE);
        return 1;
    } else {
        ret = WaitForSingleObject(*p, timeout);
        if (ret == WAIT_OBJECT_0) {
            return 1;
        } else {
            return LWCELL_SYS_TIMEOUT;
        }
    }
}

uint8_t
lwcell_sys_sem_release(lwcell_sys_sem_t* p) {
    return ReleaseSemaphore(*p, 1, NULL);
}

uint8_t
lwcell_sys_sem_isvalid(lwcell_sys_sem_t* p) {
    return p != NULL && *p != NULL;
}

uint8_t
lwcell_sys_sem_invalid(lwcell_sys_sem_t* p) {
    *p = LWCELL_SYS_SEM_NULL;
    return 1;
}

uint8_t
lwcell_sys_mbox_create(lwcell_sys_mbox_t* b, size_t size) {
    win32_mbox_t* mbox;

    *b = NULL;

    mbox = malloc(sizeof(*mbox) + size * sizeof(void*));
    if (mbox != NULL) {
        memset(mbox, 0x00, sizeof(*mbox));
        mbox->size = size + 1; /* Set it to 1 more as cyclic buffer has only one less than size */
        lwcell_sys_sem_create(&mbox->sem, 1);
        lwcell_sys_sem_create(&mbox->sem_not_empty, 0);
        lwcell_sys_sem_create(&mbox->sem_not_full, 0);
        *b = mbox;
    }
    return *b != NULL;
}

uint8_t
lwcell_sys_mbox_delete(lwcell_sys_mbox_t* b) {
    win32_mbox_t* mbox = *b;
    lwcell_sys_sem_delete(&mbox->sem);
    lwcell_sys_sem_delete(&mbox->sem_not_full);
    lwcell_sys_sem_delete(&mbox->sem_not_empty);
    free(mbox);
    return 1;
}

uint32_t
lwcell_sys_mbox_put(lwcell_sys_mbox_t* b, void* m) {
    win32_mbox_t* mbox = *b;
    uint32_t time = osKernelSysTick(); /* Get start time */

    lwcell_sys_sem_wait(&mbox->sem, 0); /* Wait for access */

    /*
     * Since function is blocking until ready to write something to queue,
     * wait and release the semaphores to allow other threads
     * to process the queue before we can write new value.
     */
    while (mbox_is_full(mbox)) {
        lwcell_sys_sem_release(&mbox->sem);          /* Release semaphore */
        lwcell_sys_sem_wait(&mbox->sem_not_full, 0); /* Wait for semaphore indicating not full */
        lwcell_sys_sem_wait(&mbox->sem, 0);          /* Wait availability again */
    }
    mbox->entries[mbox->in] = m;
    if (++mbox->in >= mbox->size) {
        mbox->in = 0;
    }
    lwcell_sys_sem_release(&mbox->sem_not_empty); /* Signal non-empty state */
    lwcell_sys_sem_release(&mbox->sem);           /* Release access for other threads */
    return osKernelSysTick() - time;
}

uint32_t
lwcell_sys_mbox_get(lwcell_sys_mbox_t* b, void** m, uint32_t timeout) {
    win32_mbox_t* mbox = *b;
    uint32_t time;

    time = osKernelSysTick();

    /* Get exclusive access to message queue */
    if (lwcell_sys_sem_wait(&mbox->sem, timeout) == LWCELL_SYS_TIMEOUT) {
        return LWCELL_SYS_TIMEOUT;
    }
    while (mbox_is_empty(mbox)) {
        lwcell_sys_sem_release(&mbox->sem);
        if (lwcell_sys_sem_wait(&mbox->sem_not_empty, timeout) == LWCELL_SYS_TIMEOUT) {
            return LWCELL_SYS_TIMEOUT;
        }
        lwcell_sys_sem_wait(&mbox->sem, timeout);
    }
    *m = mbox->entries[mbox->out];
    if (++mbox->out >= mbox->size) {
        mbox->out = 0;
    }
    lwcell_sys_sem_release(&mbox->sem_not_full);
    lwcell_sys_sem_release(&mbox->sem);

    return osKernelSysTick() - time;
}

uint8_t
lwcell_sys_mbox_putnow(lwcell_sys_mbox_t* b, void* m) {
    win32_mbox_t* mbox = *b;

    lwcell_sys_sem_wait(&mbox->sem, 0);
    if (mbox_is_full(mbox)) {
        lwcell_sys_sem_release(&mbox->sem);
        return 0;
    }
    mbox->entries[mbox->in] = m;
    if (mbox->in == mbox->out) {
        lwcell_sys_sem_release(&mbox->sem_not_empty);
    }
    if (++mbox->in >= mbox->size) {
        mbox->in = 0;
    }
    lwcell_sys_sem_release(&mbox->sem);
    return 1;
}

uint8_t
lwcell_sys_mbox_getnow(lwcell_sys_mbox_t* b, void** m) {
    win32_mbox_t* mbox = *b;

    lwcell_sys_sem_wait(&mbox->sem, 0); /* Wait exclusive access */
    if (mbox->in == mbox->out) {
        lwcell_sys_sem_release(&mbox->sem); /* Release access */
        return 0;
    }

    *m = mbox->entries[mbox->out];
    if (++mbox->out >= mbox->size) {
        mbox->out = 0;
    }
    lwcell_sys_sem_release(&mbox->sem_not_full); /* Queue not full anymore */
    lwcell_sys_sem_release(&mbox->sem);          /* Release semaphore */
    return 1;
}

uint8_t
lwcell_sys_mbox_isvalid(lwcell_sys_mbox_t* b) {
    return b != NULL && *b != NULL; /* Return status if message box is valid */
}

uint8_t
lwcell_sys_mbox_invalid(lwcell_sys_mbox_t* b) {
    *b = LWCELL_SYS_MBOX_NULL; /* Invalidate message box */
    return 1;
}

uint8_t
lwcell_sys_thread_create(lwcell_sys_thread_t* t, const char* name, lwcell_sys_thread_fn thread_func, void* const arg,
                        size_t stack_size, lwcell_sys_thread_prio_t prio) {
    HANDLE h;
    DWORD id;

    LWCELL_UNUSED(name);
    LWCELL_UNUSED(stack_size);
    LWCELL_UNUSED(prio);

    h = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)thread_func, arg, 0, &id);
    if (t != NULL) {
        *t = h;
    }
    return h != NULL;
}

uint8_t
lwcell_sys_thread_terminate(lwcell_sys_thread_t* t) {
    if (t == NULL) { /* Shall we terminate ourself? */
        ExitThread(0);
    } else {
        /* We have known thread, find handle by looking at ID */
        TerminateThread(*t, 0);
    }
    return 1;
}

uint8_t
lwcell_sys_thread_yield(void) {
    /* Not implemented */
    return 1;
}

#endif /* !__DOXYGEN__ */
