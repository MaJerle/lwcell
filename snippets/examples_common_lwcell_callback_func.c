#include <stdio.h>
#include "examples_common_lwcell_callback_func.h"
#include "network_utils.h"
#include "lwcell/lwcell.h"

/**
 * \brief           Core LwCELL callback function for all examples in the repository.
 * 
 * This reduces redundancy of the same code being written multiple times.
 * 
 * \param[in]       evt: Event data
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
lwcellr_t
examples_common_lwcell_callback_func(lwcell_evt_t* evt) {
    switch (lwcell_evt_get_type(evt)) {
        case LWCELL_EVT_INIT_FINISH: {
            printf("Library initialized!\r\n");
            break;
        }

        /* Process and print registration change */
        case LWCELL_EVT_NETWORK_REG_CHANGED: { 
            network_utils_process_reg_change(evt);
            break;
        }

        /* Process current network operator */
        case LWCELL_EVT_NETWORK_OPERATOR_CURRENT: {
            network_utils_process_curr_operator(evt);
            break;
        }

        /* Process signal strength */
        case LWCELL_EVT_SIGNAL_STRENGTH: {
            network_utils_process_rssi(evt);
            break;
        }

        /* Other user events here... */
        default: break;
    }
    return lwcellOK;
}
