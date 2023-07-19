#include "network_utils.h"
#include "lwcell/lwcell.h"

/**
 * \brief           RSSI state on network
 */
static int16_t
rssi;

/**
 * \brief           Process and print network registration status update
 * \param[in]       evt: GSM event data
 */
void
network_utils_process_reg_change(lwcell_evt_t* evt) {
    lwcell_network_reg_status_t stat;

    stat = lwcell_network_get_reg_status();        /* Get network status */

    /* Print to console */
    printf("Network registration status changed. New status is: ");
    switch (stat) {
        case LWCELL_NETWORK_REG_STATUS_CONNECTED:
            printf("Connected to home network!\r\n");
            break;
        case LWCELL_NETWORK_REG_STATUS_CONNECTED_ROAMING:
            printf("Connected to network and roaming!\r\n");
            break;
        case LWCELL_NETWORK_REG_STATUS_SEARCHING:
            printf("Searching for network!\r\n");
            break;
        case LWCELL_NETWORK_REG_STATUS_SIM_ERR:
            printf("SIM CARD ERROR!\r\n");
            break;
        default:
            printf("Other\r\n");
    }

    LWCELL_UNUSED(evt);
}

/**
 * \brief           Process and print network current operator status
 * \param[in]       evt: GSM event data
 */
void
network_utils_process_curr_operator(lwcell_evt_t* evt) {
    const lwcell_operator_curr_t* o;
    if ((o = lwcell_evt_network_operator_get_current(evt)) != NULL) {
        switch (o->format) {
            case LWCELL_OPERATOR_FORMAT_LONG_NAME:
                printf("Operator long name: %s\r\n", o->data.long_name);
                break;
            case LWCELL_OPERATOR_FORMAT_SHORT_NAME:
                printf("Operator short name: %s\r\n", o->data.short_name);
                break;
            case LWCELL_OPERATOR_FORMAT_NUMBER:
                printf("Operator number: %d\r\n", (int)o->data.num);
                break;
            default:
                break;
        }
    }
    /* Start RSSI info */
    lwcell_network_rssi(&rssi, NULL, NULL, 0);
}

/**
 * \brief           Process and print RSSI info
 * \param[in]       evt: GSM event data
 */
void
network_utils_process_rssi(lwcell_evt_t* evt) {
    int16_t rssi;

    /* Get RSSi from event */
    rssi = lwcell_evt_signal_strength_get_rssi(evt);

    /* Print message to screen */
    printf("Network operator RSSI: %d dBm\r\n", (int)rssi);
}
