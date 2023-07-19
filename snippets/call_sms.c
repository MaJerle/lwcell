/*
 * SMS send and receive basic event based example
 *
 * Waits for received SMS and then replies with
 */
#include "call_sms.h"
#include "lwcell/lwcell.h"

#if !LWCELL_CFG_SMS || !LWCELL_CFG_CALL
#error "SMS & CALL must be enabled to run this example"
#endif /* !LWCELL_CFG_SMS || !LWCELL_CFG_CALL */

static lwcellr_t call_sms_evt_func(lwcell_evt_t* evt);

/**
 * \brief           SMS entry
 */
static lwcell_sms_entry_t
sms_entry;

/**
 * \brief           Start CALL & SMS combined example
 */
void
call_sms_start(void) {
    /* Add custom callback */
    lwcell_evt_register(call_sms_evt_func);

    /* First enable SMS functionality */
    if (lwcell_sms_enable(NULL, NULL, 1) == lwcellOK) {
        printf("SMS enabled. Send new SMS from your phone to device.\r\n");
    } else {
        printf("Cannot enable SMS functionality!\r\n");
    }

    /* Then enable call functionality */
    if (lwcell_call_enable(NULL, NULL, 1) == lwcellOK) {
        printf("Call enabled. You may now take your phone and call modem\r\n");
    } else {
        printf("Cannot enable call functionality!\r\n");
    }

    /* Now send SMS from phone to device */
    printf("Start by sending SMS message or call device...\r\n");
}

/**
 * \brief           Event function for received SMS or calls
 * \param[in]       evt: GSM event
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
static lwcellr_t
call_sms_evt_func(lwcell_evt_t* evt) {
    switch (lwcell_evt_get_type(evt)) {
        case LWCELL_EVT_SMS_READY: {               /* SMS is ready notification from device */
            printf("SIM device SMS service is ready!\r\n");
            break;
        }
        case LWCELL_EVT_SMS_RECV: {                /* New SMS received indicator */
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
        case LWCELL_EVT_SMS_READ: {                /* SMS read event */
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
        case LWCELL_EVT_SMS_SEND: {                /* SMS send event */
            if (lwcell_evt_sms_send_get_result(evt) == lwcellOK) {
                printf("SMS has been successfully sent!\r\n");
            } else {
                printf("SMS has not been sent successfully!\r\n");
            }
            break;
        }

        case LWCELL_EVT_CALL_READY: {              /* Call is ready notification from device */
            printf("SIM device Call service is ready!\r\n");
            break;
        }
        case LWCELL_EVT_CALL_CHANGED: {
            const lwcell_call_t* call = lwcell_evt_call_changed_get_call(evt);
            if (call->state == LWCELL_CALL_STATE_INCOMING) {   /* On incoming call */
                lwcell_call_hangup(NULL, NULL, 0); /* Hangup call */
                lwcell_sms_send(call->number, "Cannot answer call. Please send SMS\r\n", NULL, NULL, 0);
            }
            break;
        }
        default:
            break;
    }
    return lwcellOK;
}
