/**
 * \addtogroup      GSM_SYS
 * \{
 *
 * System functions are required for timing functions and
 * for implementation of OS specific functions.
 *
 * \section         sect_sys_req Required implementation
 *
 * Implementation required functions depend on 
 * usage of OS system or not.
 *
 * System init function \ref gsm_sys_init is called on startup,
 * where user can set default states or initialize protection mutex
 *
 * \par             Always required functions
 *
 * Timing function is always required.
 * it must return time in units of milliseconds.
 *
 * For STM32 users with HAL driver support,
 * this function can be implemented by returning `osKernelSystick` function, implemented by `CMSIS-OS` ready RTOS.
 *
 * \par             OS required functions
 *
 * When OS is involved, all other functions must be created by user.
 * When using provided template, CMSIS-OS implementation is already prepared.
 * Some of CMSIS-OS supported operating systems:
 *
 *  - Keil RTX
 *  - FreeRTOS is not by default, but `cmsis_os.c` wrapper exists
 *      which allows you compatibility with standard CMSIS-OS functions
 * 
 * \section         sect_os_functions OS functions
 *
 * \par             Protection functions
 *
 * Core protection functions must be implemented by user.
 * They are called when we have to protect core from multiple access.
 *
 * \note            Keep in mind that these functions may be called recursively.
 *                  If you do protection using mutex, use recursive mutex support!
 *
 *  - \ref gsm_sys_protect function increments protection counter
 *  - \ref gsm_sys_unprotect function decrements protection counter.
 *          When set to 0, core is unprotected again
 *
 * \include         _example_sys_core.c
 *
 * \par             Mutexes
 *
 * Some functions below must be implemented for mutex management.
 *
 * Please read function documentation what is the purpose of functions
 *
 *  - \ref gsm_sys_mutex_create
 *  - \ref gsm_sys_mutex_delete
 *  - \ref gsm_sys_mutex_lock
 *  - \ref gsm_sys_mutex_unlock
 *  - \ref gsm_sys_mutex_isvalid
 *  - \ref gsm_sys_mutex_invalid
 *
 * \include         _example_sys_mutex.c
 *
 * \par             Semaphores
 *
 * For thread synchronization, binary semaphores are used.
 * They must only use single token and may be free or taken.
 *
 *  - \ref gsm_sys_sem_create(gsm_sys_sem_t* p, uint8_t cnt);
 *  - \ref gsm_sys_sem_delete(gsm_sys_sem_t* p);
 *  - \ref gsm_sys_sem_wait(gsm_sys_sem_t* p, uint32_t timeout);
 *  - \ref gsm_sys_sem_release(gsm_sys_sem_t* p);
 *  - \ref gsm_sys_sem_isvalid(gsm_sys_sem_t* p);
 *  - \ref gsm_sys_sem_invalid(gsm_sys_sem_t* p);
 *
 * \include         _example_sys_semaphore.c
 *
 * \par             Message queues
 *
 * Message queues are used for thread communication in safe way.
 *
 *  - \ref gsm_sys_mbox_create
 *  - \ref gsm_sys_mbox_delete
 *  - \ref gsm_sys_mbox_put
 *  - \ref gsm_sys_mbox_get
 *  - \ref gsm_sys_mbox_putnow
 *  - \ref gsm_sys_mbox_getnow
 *  - \ref gsm_sys_mbox_isvalid
 *  - \ref gsm_sys_mbox_invalid
 *
 * \include         _example_sys_mbox.c
 *
 * \par             Threads
 *
 * To prevent wrongly setup of core threads,
 * threading management functions must be implemented and used on demand from ESP stack.
 *
 *  - \ref gsm_sys_thread_create is called when a new thread should be created
 *  - \ref gsm_sys_thread_terminate is called to terminate thread
 *  - \ref gsm_sys_thread_yield to yield current thread and allow processing other threads
 *
 * \include         _example_sys_thread.c
 *
 * \}
 */