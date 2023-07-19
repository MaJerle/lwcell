/*
 * SMS send and receive thread.
 *
 * Waits for received SMS and then replies with
 */
#include "sms_send_receive_thread.h"
#include "lwcell/lwcell.h"
#include "lwcell/lwcell_mem.h"

#if !LWCELL_CFG_SMS
#error "SMS must be enabled to run this example"
#endif /* !LWCELL_CFG_SMS */

static lwcellr_t sms_evt_func(lwcell_evt_t* evt);

/**
 * \brief           SMS entry information
 */
typedef struct {
    lwcell_mem_t mem;
    size_t pos;
} sms_receive_t;

/**
 * \brief           SMS message box
 */
static lwcell_sys_mbox_t sms_mbox;

/**
 * \brief           SMS read entry
 */
static lwcell_sms_entry_t sms_entry;

/**
 * \brief           SMS Receive Send thread function
 */
void
sms_send_receive_thread(void const* arg) {
    sms_receive_t* sms;

    LWCELL_UNUSED(arg);

    /* Create message box */
    if (!lwcell_sys_mbox_create(&sms_mbox, 5)) {
        goto terminate;
    }

    /* Register callback function for SMS */
    if (lwcell_evt_register(sms_evt_func) != lwcellOK) {
        goto terminate;
    }

    /* First enable SMS functionality */
    if (lwcell_sms_enable(NULL, NULL, 1) == lwcellOK) {
        printf("SMS enabled. Send new SMS from your phone to device.\r\n");
    } else {
        printf("Cannot enable SMS functionality!\r\n");
        while (1) {
            lwcell_delay(1000);
        }
    }

    /* User can start now */
    printf("Start by sending first SMS to device...\r\n");

    while (1) {
        /* Get SMS entry from message queue */
        while (lwcell_sys_mbox_get(&sms_mbox, (void**)&sms, 0) == LWCELL_SYS_TIMEOUT || sms == NULL) {}

        /* We have new SMS now */
        printf("New SMS received!\r\n");

        /* Read SMS from device */
        if (lwcell_sms_read(sms->mem, sms->pos, &sms_entry, 1, NULL, NULL, 1) == lwcellOK) {
            printf("SMS read ok. Number: %s, content: %s\r\n", sms_entry.number, sms_entry.data);

            /* Send reply back */
            if (lwcell_sms_send(sms_entry.number, sms_entry.data, NULL, NULL, 1) == lwcellOK) {
                printf("SMS sent back successfully!\r\n");
            } else {
                printf("Cannot send SMS back!\r\n");
            }

            /* Delete SMS from device memory */
            if (lwcell_sms_delete(sms->mem, sms->pos, NULL, NULL, 1) == lwcellOK) {
                printf("Received SMS deleted!\r\n");
            } else {
                printf("Cannot delete received SMS!\r\n");
            }
        } else {
            printf("Cannot read SMS!\r\n");
        }

        /* Now free the memory */
        lwcell_mem_free_s((void**)&sms);
    }

terminate:
    if (lwcell_sys_mbox_isvalid(&sms_mbox)) {
        /* Lock to make sure GSM stack won't process any callbacks while we are cleaning */
        lwcell_core_lock();

        /* Clean mbox first */
        while (lwcell_sys_mbox_getnow(&sms_mbox, (void**)&sms)) {
            lwcell_mem_free_s((void**)&sms);
        }

        /* Delete mbox */
        lwcell_sys_mbox_delete(&sms_mbox);
        lwcell_sys_mbox_invalid(&sms_mbox);

        lwcell_core_unlock();

        /* Now mbox is not valid anymore and event won't write any data */
    }
}

/**
 * \brief           Event function for received SMS
 * \param[in]       evt: GSM event
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
static lwcellr_t
sms_evt_func(lwcell_evt_t* evt) {
    switch (lwcell_evt_get_type(evt)) {
        case LWCELL_EVT_SMS_RECV: {                /* New SMS received indicator */
            uint8_t success = 0;
            sms_receive_t* sms_rx = lwcell_mem_malloc(sizeof(*sms_rx));
            if (sms_rx != NULL) {
                sms_rx->mem = lwcell_evt_sms_recv_get_mem(evt);
                sms_rx->pos = lwcell_evt_sms_recv_get_pos(evt);

                /* Write to receive queue */
                if (!lwcell_sys_mbox_isvalid(&sms_mbox) || !lwcell_sys_mbox_putnow(&sms_mbox, sms_rx)) {
                    lwcell_mem_free_s((void**)&sms_rx);
                } else {
                    success = 1;
                }
            }

            /* Force SMS delete if not written successfully */
            if (!success) {
                lwcell_sms_delete(lwcell_evt_sms_recv_get_mem(evt), lwcell_evt_sms_recv_get_pos(evt), NULL, NULL, 0);
            }
            break;
        }
        default:
            break;
    }
    return lwcellOK;
}
