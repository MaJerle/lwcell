/*
 * SMS send and receive basic event based example
 *
 * Waits for received SMS and then replies with
 */
#include "sms_send_receive.h"
#include "gsm/gsm.h"

#if !GSM_CFG_SMS
#error "SMS must be enabled to run this example"
#endif /* !GSM_CFG_SMS */

static gsmr_t sms_evt_func(gsm_evt_t* evt);

/**
 * \brief           SMS entry
 */
static gsm_sms_entry_t
sms_entry;

/**
 * \brief           Start SMS send receive procedure
 */
void
sms_send_receive_start(void) {
    /* Add custom callback */
    gsm_evt_register(sms_evt_func);

    /* First enable SMS functionality */
    if (gsm_sms_enable(NULL, NULL, 1) == gsmOK) {
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
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
static gsmr_t
sms_evt_func(gsm_evt_t* evt) {
    switch (gsm_evt_get_type(evt)) {
        case GSM_EVT_SMS_READY: {               /* SMS is ready notification from device */
            printf("SIM device SMS service is ready!\r\n");
            break;
        }
        case GSM_EVT_SMS_RECV: {                /* New SMS received indicator */
            gsmr_t res;

            printf("New SMS received!\r\n");    /* Notify user */

            /* Try to read SMS */
            res = gsm_sms_read(gsm_evt_sms_recv_get_mem(evt), gsm_evt_sms_recv_get_pos(evt), &sms_entry, 1, NULL, NULL, 0);
            if (res == gsmOK) {
                printf("SMS read in progress!\r\n");
            } else {
                printf("Cannot start SMS read procedure!\r\n");
            }
            break;
        }
        case GSM_EVT_SMS_READ: {                /* SMS read event */
            gsm_sms_entry_t* entry = gsm_evt_sms_read_get_entry(evt);
            if (gsm_evt_sms_read_get_result(evt) == gsmOK && entry != NULL) {
                /* Print SMS data */
                printf("SMS read. From: %s, content: %s\r\n",
                       entry->number, entry->data
                      );

                /* Try to send SMS back */
                if (gsm_sms_send(entry->number, entry->data, NULL, NULL, 0) == gsmOK) {
                    printf("SMS send in progress!\r\n");
                } else {
                    printf("Cannot start SMS send procedure!\r\n");
                }

                /* Delete SMS from device memory */
                gsm_sms_delete(entry->mem, entry->pos, NULL, NULL, 0);
            }
            break;
        }
        case GSM_EVT_SMS_SEND: {                /* SMS send event */
            if (gsm_evt_sms_send_get_result(evt) == gsmOK) {
                printf("SMS has been successfully sent!\r\n");
            } else {
                printf("SMS has not been sent successfully!\r\n");
            }
            break;
        }
        case GSM_EVT_SMS_DELETE: {              /* SMS delete event */
            if (gsm_evt_sms_delete_get_result(evt) == gsmOK) {
                printf("SMS deleted, memory position: %d\r\n", (int)gsm_evt_sms_delete_get_pos(evt));
            } else {
                printf("SMS delete operation failed!\r\n");
            }
            break;
        }
        default:
            break;
    }

    return gsmOK;
}
