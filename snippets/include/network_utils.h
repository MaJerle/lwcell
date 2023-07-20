#ifndef SNIPPET_HDR_NETWORK_UTILS_H
#define SNIPPET_HDR_NETWORK_UTILS_H

#include <stdint.h>
#include "lwcell/lwcell.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void    network_utils_process_reg_change(lwcell_evt_t* evt);
void    network_utils_process_curr_operator(lwcell_evt_t* evt);
void    network_utils_process_rssi(lwcell_evt_t* evt);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SNIPPET_HDR_NETWORK_UTILS_H */
