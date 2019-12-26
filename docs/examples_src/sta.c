size_t i, apf;
gsm_ap_t aps[100];
 
/* Search for access points around GSM station */
if (gsm_sta_list_ap(NULL, aps, GSM_ARRAYSIZE(aps), &apf, NULL, NULL, 1) == espOK) {
    for (i = 0; i < apf; i++) {
        printf("AP found: %s\r\n", aps[i].ssid);
    }
}