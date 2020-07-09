/*
 * Receive call and answer to it example
 */
#include "call.h"
#include "lwgsm/lwgsm.h"

#if !GSM_CFG_CALL
#error "CALL must be enabled to run this example"
#endif /* !GSM_CFG_CALL */

static lwgsmr_t call_evt_func(lwgsm_evt_t* evt);

/**
 * \brief           Start CALL example
 */
void
call_start(void) {
    /* Add custom callback */
    lwgsm_evt_register(call_evt_func);

    /* Enable calls */
    if (lwgsm_call_enable(NULL, NULL, 1) == gsmOK) {
        printf("Calls enabled. You may take your phone and call GSM device number.\r\n");
    } else {
        printf("Could not enable call functionality!\r\n");
    }
}

/**
 * \brief           Event function for calls
 * \param[in]       evt: GSM event
 * \return          \ref gsmOK on success, member of \ref lwgsmr_t otherwise
 */
static lwgsmr_t
call_evt_func(lwgsm_evt_t* evt) {
    switch (lwgsm_evt_get_type(evt)) {
        case GSM_EVT_CALL_CHANGED: {
            const lwgsm_call_t* call = lwgsm_evt_call_changed_get_call(evt);
            if (call->state == GSM_CALL_STATE_ACTIVE) {
                printf("Call is active!\r\n");

                /* In case of mobile originated direction */
                if (call->dir == GSM_CALL_DIR_MO) {
                    lwgsm_call_hangup(NULL, NULL, 0); /* Manually hangup call */
                }
            } else if (call->state == GSM_CALL_STATE_INCOMING) {
                printf("Incoming call received! Phone number: %s\r\n", call->number);
                lwgsm_call_answer(NULL, NULL, 0); /* Answer to a call */
            } else if (call->state == GSM_CALL_STATE_DIALING) {
                printf("Call is dialing!\r\n");
            } else if (call->state == GSM_CALL_STATE_DISCONNECT) {
                printf("Call ended!\r\n");
            }
            break;
        }
        default:
            break;
    }

    return gsmOK;
}
