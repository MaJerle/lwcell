/*
 * SMS send and receive basic event based example
 *
 * Waits for received SMS and then replies with
 */
#include "call_sms.h"
#include "lwgsm/lwgsm.h"

#if !LWGSM_CFG_SMS || !LWGSM_CFG_CALL
#error "SMS & CALL must be enabled to run this example"
#endif /* !LWGSM_CFG_SMS || !LWGSM_CFG_CALL */

static lwgsmr_t call_sms_evt_func(lwgsm_evt_t* evt);

/**
 * \brief           SMS entry
 */
static lwgsm_sms_entry_t
sms_entry;

/**
 * \brief           Start CALL & SMS combined example
 */
void
call_sms_start(void) {
    /* Add custom callback */
    lwgsm_evt_register(call_sms_evt_func);

    /* First enable SMS functionality */
    if (lwgsm_sms_enable(NULL, NULL, 1) == lwgsmOK) {
        printf("SMS enabled. Send new SMS from your phone to device.\r\n");
    } else {
        printf("Cannot enable SMS functionality!\r\n");
    }

    /* Then enable call functionality */
    if (lwgsm_call_enable(NULL, NULL, 1) == lwgsmOK) {
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
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
static lwgsmr_t
call_sms_evt_func(lwgsm_evt_t* evt) {
    switch (lwgsm_evt_get_type(evt)) {
        case LWGSM_EVT_SMS_READY: {               /* SMS is ready notification from device */
            printf("SIM device SMS service is ready!\r\n");
            break;
        }
        case LWGSM_EVT_SMS_RECV: {                /* New SMS received indicator */
            lwgsmr_t res;

            printf("New SMS received!\r\n");    /* Notify user */

            /* Try to read SMS */
            res = lwgsm_sms_read(lwgsm_evt_sms_recv_get_mem(evt), lwgsm_evt_sms_recv_get_pos(evt), &sms_entry, 1, NULL, NULL, 0);
            if (res == lwgsmOK) {
                printf("SMS read in progress!\r\n");
            } else {
                printf("Cannot start SMS read procedure!\r\n");
            }
            break;
        }
        case LWGSM_EVT_SMS_READ: {                /* SMS read event */
            lwgsm_sms_entry_t* entry = lwgsm_evt_sms_read_get_entry(evt);
            if (lwgsm_evt_sms_read_get_result(evt) == lwgsmOK && entry != NULL) {
                /* Print SMS data */
                printf("SMS read. From: %s, content: %s\r\n",
                       entry->number, entry->data);

                /* Try to send SMS back */
                if (lwgsm_sms_send(entry->number, entry->data, NULL, NULL, 0) == lwgsmOK) {
                    printf("SMS send in progress!\r\n");
                } else {
                    printf("Cannot start SMS send procedure!\r\n");
                }

                /* Delete SMS from device memory */
                lwgsm_sms_delete(entry->mem, entry->pos, NULL, NULL, 0);
            }
            break;
        }
        case LWGSM_EVT_SMS_SEND: {                /* SMS send event */
            if (lwgsm_evt_sms_send_get_result(evt) == lwgsmOK) {
                printf("SMS has been successfully sent!\r\n");
            } else {
                printf("SMS has not been sent successfully!\r\n");
            }
            break;
        }

        case LWGSM_EVT_CALL_READY: {              /* Call is ready notification from device */
            printf("SIM device Call service is ready!\r\n");
            break;
        }
        case LWGSM_EVT_CALL_CHANGED: {
            const lwgsm_call_t* call = lwgsm_evt_call_changed_get_call(evt);
            if (call->state == LWGSM_CALL_STATE_INCOMING) {   /* On incoming call */
                lwgsm_call_hangup(NULL, NULL, 0); /* Hangup call */
                lwgsm_sms_send(call->number, "Cannot answer call. Please send SMS\r\n", NULL, NULL, 0);
            }
            break;
        }
        default:
            break;
    }
    return lwgsmOK;
}
