uint32_t time;

/* Try to ping domain example.com and print time */
if (gsm_ping("example.com", &time, NULL, NULL, 1) == gsmOK) {
    printf("Ping successful. Time: %d ms\r\n", (int)time);
}