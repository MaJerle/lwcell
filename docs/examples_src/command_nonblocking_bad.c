/* Hostname event function, called when gsm_sms_send() function finishes */
void
sms_send_fn(lwgsmr_t res, void* arg) {
    /* Check actual result from device */
    if (res == gsmOK) {
        printf("SMS sent successfully\r\n");
    } else {
        printf("Error trying to send SMS\r\n");
    }
}

/* Check hostname */
void
check_hostname(void) {
    char message[] = "text message";

    /* Send SMS in non-blocking mode */
    /* Function now returns if command has been sent to internal message queue */
    /* It uses pointer to local data but w/o blocking command */
    if (gsm_sms_send("number", message, sms_send_fn, NULL, 0 /* 0 means non-blocking call */) == gsmOK) {
        /* At this point we only know that command has been sent to queue */
        printf("SMS send message command sent to queue.\r\n");
    } else {
        /* Error writing message to queue */
        printf("Cannot send SMS send message command to queue. Maybe out of memory? Check result from function\r\n");
    }
}