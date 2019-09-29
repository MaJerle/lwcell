/* Hostname event function, called when gsm_sms_send() function finishes */
void
sms_send_fn(gsmr_t res, void* arg) {
	/* Check actual result from device */
	if (res == gsmOK) {
		printf("SMS sent successfully\r\n", hostname);
	} else {
		printf("Error trying to send SMS..\r\n");
	}
}

/* Somewhere in thread and/or other GSM event function */

/* Send SMS in non-blocking mode */
/* Function now returns if command has been sent to internal message queue */
if (gsm_sms_send(hostname, sizeof(hostname), sms_send_fn, NULL, 0 /* 0 means non-blocking call */) == gsmOK) {
	/* At this point we only know that command has been sent to queue */
	printf("SMS send message sent to queue.\r\n");
} else {
	/* Error writing message to queue */
	printf("Cannot send SMS send message to queue. Maybe out of memory? Check result from function\r\n");
}