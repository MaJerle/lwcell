/*
 * Read device information
 */
#include "gsm/gsm.h"

/**
 * \brief           Device info string array
 */
static char dev_str[20];

/**
 * \brief           Start SMS send receive procedure
 */
void
read_device_info(void) {
    /* Read information */

    /* Read device manufacturer */
    gsm_device_get_manufacturer(dev_str, sizeof(dev_str), NULL, NULL, 1);
    printf("Manuf: %s\r\n", dev_str);

    /* Read device model */
    gsm_device_get_model(dev_str, sizeof(dev_str), NULL, NULL, 1);
    printf("Model: %s\r\n", dev_str);
    
    /* Read device serial number */
    gsm_device_get_serial_number(dev_str, sizeof(dev_str), NULL, NULL, 1);
    printf("Serial: %s\r\n", dev_str);
    
    /* Read device revision */
    gsm_device_get_revision(dev_str, sizeof(dev_str), NULL, NULL, 1);
    printf("Revision: %s\r\n", dev_str);
}
