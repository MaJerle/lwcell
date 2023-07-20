/*
 * SMS send and receive basic event based example
 *
 * Waits for received SMS and then replies with
 */
#include "sms_send_receive.h"
#include "lwcell/lwcell.h"

#if !LWCELL_CFG_SMS
#error "SMS must be enabled to run this example"
#endif /* !LWCELL_CFG_SMS */

static lwcellr_t sms_evt_func(lwcell_evt_t* evt);

/**
 * \brief           SMS entry
 */
static lwcell_sms_entry_t sms_entry;

/**
 * \brief           Start SMS send receive procedure
 */
void
sms_send_receive_start(void) {
    /* Add custom callback */
    lwcell_evt_register(sms_evt_func);

    /* First enable SMS functionality */
    if (lwcell_sms_enable(NULL, NULL, 1) == lwcellOK) {
        printf("SMS enabled. Send new SMS from your phone to device.\r\n");
    } else {
        printf("Cannot enable SMS functionality!\r\n");
    }

    /* Now send SMS from phone to device */
    printf("Start by sending SMS message to device...\r\n");
}

/**
 * \brief           Event function for received SMS
 * \param[in]       evt: GSM event
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
static lwcellr_t
sms_evt_func(lwcell_evt_t* evt) {
    switch (lwcell_evt_get_type(evt)) {
        case LWCELL_EVT_SMS_READY: {             /* SMS is ready notification from device */
            printf("SIM device SMS service is ready!\r\n");
            break;
        }
        case LWCELL_EVT_SMS_RECV: {              /* New SMS received indicator */
            lwcellr_t res;

            printf("New SMS received!\r\n");    /* Notify user */

            /* Try to read SMS */
            res = lwcell_sms_read(lwcell_evt_sms_recv_get_mem(evt), lwcell_evt_sms_recv_get_pos(evt), &sms_entry, 1, NULL, NULL, 0);
            if (res == lwcellOK) {
                printf("SMS read in progress!\r\n");
            } else {
                printf("Cannot start SMS read procedure!\r\n");
            }
            break;
        }
        case LWCELL_EVT_SMS_READ: {              /* SMS read event */
            lwcell_sms_entry_t* entry = lwcell_evt_sms_read_get_entry(evt);
            if (lwcell_evt_sms_read_get_result(evt) == lwcellOK && entry != NULL) {
                /* Print SMS data */
                printf("SMS read. From: %s, content: %s\r\n",
                       entry->number, entry->data);

                /* Try to send SMS back */
                if (lwcell_sms_send(entry->number, entry->data, NULL, NULL, 0) == lwcellOK) {
                    printf("SMS send in progress!\r\n");
                } else {
                    printf("Cannot start SMS send procedure!\r\n");
                }

                /* Delete SMS from device memory */
                lwcell_sms_delete(entry->mem, entry->pos, NULL, NULL, 0);
            }
            break;
        }
        case LWCELL_EVT_SMS_SEND: {              /* SMS send event */
            if (lwcell_evt_sms_send_get_result(evt) == lwcellOK) {
                printf("SMS has been successfully sent!\r\n");
            } else {
                printf("SMS has not been sent successfully!\r\n");
            }
            break;
        }
        case LWCELL_EVT_SMS_DELETE: {            /* SMS delete event */
            if (lwcell_evt_sms_delete_get_result(evt) == lwcellOK) {
                printf("SMS deleted, memory position: %d\r\n", (int)lwcell_evt_sms_delete_get_pos(evt));
            } else {
                printf("SMS delete operation failed!\r\n");
            }
            break;
        }
        default:
            break;
    }

    return lwcellOK;
}
