/*
 * \brief           User defined callback function for GSM events
 * \param[in]       evt: Callback event data
 */
lwgsmr_t
lwgsm_callback_function(lwgsm_evt_t* evt) {
    switch (lwgsm_evt_get_type(evt)) {
        case LWGSM_EVT_RESET: {                    /* Reset detected on GSM device */
            if (lwgsm_evt_reset_is_forced(evt)) {  /* Check if forced by user */
                printf("Reset forced by user!\r\n");
            }
            break;
        }
        default: break;
    }
    return lwgsmOK;
}