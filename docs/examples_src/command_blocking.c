/* Somewhere in thread function */

/* Get device hostname in blocking mode */
/* Function returns actual result */
if (lwcell_sms_send("+0123456789", "text", NULL, NULL, 1 /* 1 means blocking call */) == lwcellOK) {
    /* At this point we have valid result from device */
    printf("SMS sent successfully\r\n");
} else {
    printf("Error trying to send SMS..\r\n");
}