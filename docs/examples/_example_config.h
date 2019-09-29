#ifndef GSM_HDR_CONFIG_H
#define GSM_HDR_CONFIG_H

/* Rename this file to "gsm_config.h" for your application */

/* Increase default receive buffer length */
#define GSM_RCV_BUFF_SIZE                   0x800
 
/* After user configuration, call default config to merge config together */
#include "gsm/gsm_config_default.h"

#endif /* GSM_HDR_CONFIG_H */