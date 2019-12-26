/*
 * \brief           User defined callback function for GSM events
 * \param[in]       evt: Callback event data
 */
gsmr_t
gsm_callback_function(gsm_evt_t* evt) {
    switch (gsm_evt_get_type(evt)) {
        case GSM_EVT_RESET: {                    /* Reset detected on GSM device */
            if (gsm_evt_reset_is_forced(evt)) {  /* Check if forced by user */
                printf("Reset forced by user!\r\n");
            }
            break;
        }
        default: break;
    }
    return gsmOK;
}