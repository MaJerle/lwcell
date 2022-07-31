#include <stdio.h>
#include "examples_common_lwgsm_callback_func.h"
#include "network_utils.h"
#include "lwgsm/lwgsm.h"

/**
 * \brief           Core LwGSM callback function for all examples in the repository.
 * 
 * This reduces redundancy of the same code being written multiple times.
 * 
 * \param[in]       evt: Event data
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
examples_common_lwgsm_callback_func(lwgsm_evt_t* evt) {
    switch (lwgsm_evt_get_type(evt)) {
        case LWGSM_EVT_INIT_FINISH: {
            printf("Library initialized!\r\n");
            break;
        }

        /* Process and print registration change */
        case LWGSM_EVT_NETWORK_REG_CHANGED: { 
            network_utils_process_reg_change(evt);
            break;
        }

        /* Process current network operator */
        case LWGSM_EVT_NETWORK_OPERATOR_CURRENT: {
            network_utils_process_curr_operator(evt);
            break;
        }

        /* Process signal strength */
        case LWGSM_EVT_SIGNAL_STRENGTH: {
            network_utils_process_rssi(evt);
            break;
        }

        /* Other user events here... */

        default: break;
    }
    return lwgsmOK;
}
