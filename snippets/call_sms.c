/*
 * SMS send and receive basic event based example
 *
 * Waits for received SMS and then replies with
 */
#include "call_sms.h"
#include "gsm/gsm.h"

#if !GSM_CFG_SMS || !GSM_CFG_CALL
#error "SMS & CALL must be enabled to run this example"
#endif /* !GSM_CFG_SMS || !GSM_CFG_CALL */

static gsmr_t call_sms_evt_func(gsm_evt_t* evt);

/**
 * \brief           SMS entry
 */
static gsm_sms_entry_t
sms_entry;

/**
 * \brief           Start CALL & SMS combined example
 */
void
call_sms_start(void) {
    /* Add custom callback */
    gsm_evt_register(call_sms_evt_func);

    /* First enable SMS functionality */
    if (gsm_sms_enable(NULL, NULL, 1) == gsmOK) {
        printf("SMS enabled. Send new SMS from your phone to device.\r\n");
    } else {
        printf("Cannot enable SMS functionality!\r\n");
    }

    /* Then enable call functionality */
    if (gsm_call_enable(NULL, NULL, 1) == gsmOK) {
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
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
static gsmr_t
call_sms_evt_func(gsm_evt_t* evt) {
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

        case GSM_EVT_CALL_READY: {              /* Call is ready notification from device */
            printf("SIM device Call service is ready!\r\n");
            break;
        }
        case GSM_EVT_CALL_CHANGED: {
            const gsm_call_t* call = gsm_evt_call_changed_get_call(evt);
            if (call->state == GSM_CALL_STATE_INCOMING) {   /* On incoming call */
                gsm_call_hangup(NULL, NULL, 0); /* Hangup call */
                gsm_sms_send(call->number, "Cannot answer call. Please send SMS\r\n", NULL, NULL, 0);
            }
            break;
        }
        default:
            break;
    }

    return gsmOK;
}
