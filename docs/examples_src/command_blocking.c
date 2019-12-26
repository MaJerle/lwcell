char hostname[20];

/* Somewhere in thread function */

/* Get device hostname in blocking mode */
/* Function returns actual result */
if (gsm_hostname_get(hostname, sizeof(hostname), NULL, NULL, 1 /* 1 means blocking call */) == espOK) {
    /* At this point we have valid result and parameters from API function */
    printf("GSM hostname is %s\r\n", hostname);
} else {
    printf("Error reading GSM hostname..\r\n");
}