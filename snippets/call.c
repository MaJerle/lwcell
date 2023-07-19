/*
 * Receive call and answer to it example
 */
#include "call.h"
#include "lwcell/lwcell.h"

#if !LWCELL_CFG_CALL
#error "CALL must be enabled to run this example"
#endif /* !LWCELL_CFG_CALL */

static lwcellr_t call_evt_func(lwcell_evt_t* evt);

/**
 * \brief           Start CALL example
 */
void
call_start(void) {
    /* Add custom callback */
    lwcell_evt_register(call_evt_func);

    /* Enable calls */
    if (lwcell_call_enable(NULL, NULL, 1) == lwcellOK) {
        printf("Calls enabled. You may take your phone and call GSM device number.\r\n");
    } else {
        printf("Could not enable call functionality!\r\n");
    }
}

/**
 * \brief           Event function for calls
 * \param[in]       evt: GSM event
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
static lwcellr_t
call_evt_func(lwcell_evt_t* evt) {
    switch (lwcell_evt_get_type(evt)) {
        case LWCELL_EVT_CALL_CHANGED: {
            const lwcell_call_t* call = lwcell_evt_call_changed_get_call(evt);
            if (call->state == LWCELL_CALL_STATE_ACTIVE) {
                printf("Call is active!\r\n");

                /* In case of mobile originated direction */
                if (call->dir == LWCELL_CALL_DIR_MO) {
                    lwcell_call_hangup(NULL, NULL, 0); /* Manually hangup call */
                }
            } else if (call->state == LWCELL_CALL_STATE_INCOMING) {
                printf("Incoming call received! Phone number: %s\r\n", call->number);
                lwcell_call_answer(NULL, NULL, 0); /* Answer to a call */
            } else if (call->state == LWCELL_CALL_STATE_DIALING) {
                printf("Call is dialing!\r\n");
            } else if (call->state == LWCELL_CALL_STATE_DISCONNECT) {
                printf("Call ended!\r\n");
            }
            break;
        }
        default:
            break;
    }
    return lwcellOK;
}
