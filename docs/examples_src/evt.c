/*
 * \brief           User defined callback function for GSM events
 * \param[in]       evt: Callback event data
 */
lwcellr_t
lwcell_callback_function(lwcell_evt_t* evt) {
    switch (lwcell_evt_get_type(evt)) {
        case LWCELL_EVT_RESET: {                    /* Reset detected on GSM device */
            if (lwcell_evt_reset_is_forced(evt)) {  /* Check if forced by user */
                printf("Reset forced by user!\r\n");
            }
            break;
        }
        default: break;
    }
    return lwcellOK;
}