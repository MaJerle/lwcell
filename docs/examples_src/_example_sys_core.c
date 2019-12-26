static osMutexId sys_mutex;                     /* Mutex ID for main protection */

/*
 * \brief           Init system dependant parameters
 * \note            Called from high-level application layer when required
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_init(void) {
    gsm_sys_mutex_create(&sys_mutex);           /* Create system mutex */
    return 1;
}

/*
 * \brief           Get current time in units of milliseconds
 * \return          Current time in units of milliseconds
 */
uint32_t
gsm_sys_now(void) {
    return HAL_GetTick();                       /* Get current tick in units of milliseconds */
}

/*
 * \brief           Protect stack core
 * \note            This function is required with OS
 *
 * \note            This function may be called multiple times, recursive protection is required
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_protect(void) {
    gsm_sys_mutex_lock(&sys_mutex);             /* Lock system and protect it */
    return 1;
}

/*
 * \brief           Protect stack core
 * \note            This function is required with OS
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsm_sys_unprotect(void) {
    gsm_sys_mutex_unlock(&sys_mutex);           /* Release lock */
    return 1;
}