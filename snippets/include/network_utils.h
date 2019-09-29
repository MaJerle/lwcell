#ifndef __NETWORK_UTILS_H
#define __NETWORK_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "gsm/gsm.h"

void    network_utils_process_reg_change(gsm_evt_t* evt);
void    network_utils_process_curr_operator(gsm_evt_t* evt);
void    network_utils_process_rssi(gsm_evt_t* evt);

#ifdef __cplusplus
}
#endif

#endif
