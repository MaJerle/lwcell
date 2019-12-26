/* Somewhere in thread function */

/* Get device hostname in blocking mode */
/* Function returns actual result */
if (gsm_sms_send("+0123456789", "text", NULL, NULL, 1 /* 1 means blocking call */) == gsmOK) {
	/* At this point we have valid result from device */
	printf("SMS sent successfully\r\n");
} else {
	printf("Error trying to send SMS..\r\n");
}