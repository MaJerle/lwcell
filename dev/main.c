#include "windows.h"
#include "lwcell/lwcell.h"

#include "lwcell/apps/lwcell_mqtt_client_api.h"
#include "lwcell/lwcell_mem.h"
#include "lwcell/lwcell_network_api.h"

#include "mqtt_client_api.h"
#include "netconn_client.h"
#include "sms_send_receive.h"
#include "network_apn_settings.h"
#include "sms_send_receive_thread.h"
#include "client.h"
#include "lwmem/lwmem.h"

static void main_thread(void* arg);
DWORD main_thread_id;

static lwcellr_t lwcell_evt(lwcell_evt_t* evt);

lwcell_operator_t operators[10];
size_t operators_len;

static lwcell_sms_entry_t sms_entry;

lwcell_sms_entry_t sms_entries[10];
size_t sms_entries_read;

lwcell_pb_entry_t pb_entries[10];
size_t pb_entries_read;

lwcell_operator_curr_t operator_curr;

char model_str[20];

typedef struct {
    const char* pin_default;
    const char* pin;
    const char* puk;
} my_sim_t;
my_sim_t sim = {
    .pin = "4591",
    .puk = "10663647",
};

/* Custom memory allocation */
static uint8_t lwmem_region_1[0x4000];
static lwmem_region_t lwmem_regions[] = {
    {lwmem_region_1, sizeof(lwmem_region_1)},
    {NULL, 0},
};

/**
 * \brief           Program entry point
 */
int
main() {
    printf("App start!\r\n");

    /* First step is to setup memory */
    if (!lwmem_assignmem(lwmem_regions)) {
        printf("Could not assign memory for LwMEM!\r\n");
        return -1;
    }

    /* Create start main thread */
    CreateThread(0, 0, (LPTHREAD_START_ROUTINE)main_thread, NULL, 0, &main_thread_id);

    /* Do nothing at this point but do not close the program */
    while (1) {
        lwcell_delay(1000);
    }
}

void
pin_evt(lwcellr_t res, void* arg) {
    LWCELL_UNUSED(res);
    LWCELL_UNUSED(arg);
    printf("PIN EVT function!\r\n");
}

void
puk_evt(lwcellr_t res, void* arg) {
    LWCELL_UNUSED(res);
    LWCELL_UNUSED(arg);
    printf("PUK EVT function!\r\n");
}

/**
 * \brief           Console input thread
 * \param[in]       arg: Thread parameter
 */
static void
input_thread(void* arg) {
    char buff[128];

#define IS_LINE(s)      (strncmp(buff, (s), sizeof(s) - 1) == 0)

    LWCELL_UNUSED(arg);

    /* Notify user */
    printf("Start by writing commands..\r\n");

    /* Very simple input */
    while (1) {
        printf(" > ");
        fgets(buff, sizeof(buff), stdin);
        
        if (IS_LINE("reset")) {
            lwcell_reset(NULL, NULL, 1);
        } else if (IS_LINE("devicemanufacturer")) {
            lwcell_device_get_manufacturer(model_str, sizeof(model_str), NULL, NULL, 1);
            printf("Manuf: %s\r\n", model_str);
        } else if (IS_LINE("devicemodel")) {
            lwcell_device_get_model(model_str, sizeof(model_str), NULL, NULL, 1);
            printf("Model: %s\r\n", model_str);
        } else if (IS_LINE("deviceserial")) {
            lwcell_device_get_serial_number(model_str, sizeof(model_str), NULL, NULL, 1);
            printf("Serial: %s\r\n", model_str);
        } else if (IS_LINE("devicerevision")) {
            lwcell_device_get_revision(model_str, sizeof(model_str), NULL, NULL, 1);
            printf("Revision: %s\r\n", model_str);
        } else if (IS_LINE("simstatus")) {
            switch (lwcell_sim_get_current_state()) {
                case LWCELL_SIM_STATE_READY: printf("SIM state ready!\r\n"); break;
                case LWCELL_SIM_STATE_PIN: printf("SIM state PIN\r\n"); break;
                case LWCELL_SIM_STATE_PUK: printf("SIM state PIN\r\n"); break;
                case LWCELL_SIM_STATE_NOT_READY: printf("SIM state not ready\r\n"); break;
                case LWCELL_SIM_STATE_NOT_INSERTED: printf("SIM state not inserted\r\n"); break;
                default: printf("Unknown pin state\r\n"); break;
            }
        } else if (IS_LINE("simpinadd")) {
            lwcell_sim_pin_add(sim.pin, NULL, NULL, 1);
        } else if (IS_LINE("simpinchange")) {
            lwcell_sim_pin_change(sim.pin, "1234", NULL, NULL, 1);
            lwcell_sim_pin_change("1234", sim.pin, NULL, NULL, 1);
        } else if (IS_LINE("simpinremove")) {
            lwcell_sim_pin_remove(sim.pin, NULL, NULL, 1);
        } else if (IS_LINE("simpinenter")) {
            lwcell_sim_pin_enter(sim.pin, NULL, NULL, 1);
        } else if (IS_LINE("simpuk")) {
            lwcell_sim_puk_enter(sim.puk, sim.pin, puk_evt, NULL, 1);
        } else if (IS_LINE("operatorscan")) {
            lwcell_operator_scan(operators, LWCELL_ARRAYSIZE(operators), &operators_len, NULL, NULL, 1);
        } else if (IS_LINE("join")) {
            lwcell_network_request_attach();
        } else if (IS_LINE("quit")) {
            lwcell_network_request_detach();
        } else if (IS_LINE("netconnclient")) {
            lwcell_sys_sem_t sem;
            lwcell_sys_sem_create(&sem, 0);
            lwcell_sys_thread_create(NULL, "netconn_client", (lwcell_sys_thread_fn)netconn_client_thread, &sem, LWCELL_SYS_THREAD_SS, LWCELL_SYS_THREAD_PRIO);
            lwcell_sys_sem_wait(&sem, 0);
            lwcell_sys_sem_delete(&sem);
#if LWCELL_CFG_SMS
        } else if (IS_LINE("smsenable")) {
            lwcell_sms_enable(NULL, NULL, 1);
        } else if (IS_LINE("smsdisable")) {
            lwcell_sms_disable(NULL, NULL, 1);
        } else if (IS_LINE("smssend")) {
            lwcell_sms_send("+38631779982", "Hello world!", NULL, NULL, 1);
        } else if (IS_LINE("smslist")) {
            lwcell_sms_list(LWCELL_MEM_CURRENT, LWCELL_SMS_STATUS_ALL, sms_entries, LWCELL_ARRAYSIZE(sms_entries), &sms_entries_read, 0, NULL, NULL, 1);
        } else if (IS_LINE("smsdeleteall")) {
            lwcell_sms_delete_all(LWCELL_SMS_STATUS_ALL, NULL, NULL, 1);
        } else if (IS_LINE("smsthread")) {
            lwcell_sys_thread_create(NULL, "sms_recv_send", (lwcell_sys_thread_fn)sms_send_receive_thread, NULL, LWCELL_SYS_THREAD_SS, LWCELL_SYS_THREAD_PRIO);
#endif /* LWCELL_CFG_SMS */
#if LWCELL_CFG_CALL
        } else if (IS_LINE("callenable")) {
            lwcell_call_enable(NULL, NULL, 1);
        } else if (IS_LINE("calldisable")) {
            lwcell_call_disable(NULL, NULL, 1);
        } else if (IS_LINE("callstart")) {
            lwcell_call_start("+38631779982", NULL, NULL, 1);
        } else if (IS_LINE("callhangup")) {
            lwcell_call_hangup(NULL, NULL, 1);
        } else if (IS_LINE("callanswer")) {
            lwcell_call_answer(NULL, NULL, 1);
#endif /* LWCELL_CFG_CALL */
#if LWCELL_CFG_PHONEBOOK
        } else if (IS_LINE("pbenable")) {
            lwcell_pb_enable(NULL, NULL, 1);
        } else if (IS_LINE("pbread")) {
            lwcell_pb_read(LWCELL_MEM_CURRENT, 1, pb_entries, NULL, NULL, 1);
        } else if (IS_LINE("pblist")) {
            lwcell_pb_list(LWCELL_MEM_CURRENT, 1, pb_entries, LWCELL_ARRAYSIZE(pb_entries), &pb_entries_read, NULL, NULL, 1);
#endif /* LWCELL_CFG_PHONEBOOK */
        } else if (IS_LINE("mqttthread")) {
            lwcell_sys_thread_create(NULL, "mqtt_client_api", (lwcell_sys_thread_fn)lwcell_mqtt_client_api_thread, NULL, LWCELL_SYS_THREAD_SS, LWCELL_SYS_THREAD_PRIO);
        } else if (IS_LINE("client")) {
            client_connect();
#if LWCELL_CFG_USSD
        } else if (IS_LINE("ussd")) {
            char response[128];
            lwcell_ussd_run("*123#", response, sizeof(response), NULL, NULL, 1);
            printf("Command finished!\r\n");
#endif /* LWCELL_CFG_USSD */
        } else {
            printf("Unknown input!\r\n");
        }
    }
}

/**
 * \brief           Main thread for init purposes
 */
static void
main_thread(void* arg) {
    lwcell_sim_state_t sim_state, sim_prev_state;

    LWCELL_UNUSED(arg);

    /* Init GSM library */
    lwcell_init(lwcell_evt, 1);

    /* Set global network credentials */
    lwcell_network_set_credentials(NETWORK_APN, NETWORK_APN_USER, NETWORK_APN_PASS);

    /* Start input thread */
    lwcell_sys_thread_create(NULL, "input", (lwcell_sys_thread_fn)input_thread, NULL, 0, LWCELL_SYS_THREAD_PRIO);

    sim_prev_state = LWCELL_SIM_STATE_END;
    while (1) {
        /* Check for sim card */
        if ((sim_state = lwcell_sim_get_current_state()) != sim_prev_state) {
            if (sim_state == LWCELL_SIM_STATE_READY) {
                printf("SIM pin is now ready\r\n");
            } else if (sim_state == LWCELL_SIM_STATE_PIN) {
                printf("GSM state PIN\r\n");
                lwcell_sim_pin_enter(sim.pin, pin_evt, NULL, 1);
            } else if (sim_state == LWCELL_SIM_STATE_PUK) {
                printf("GSM state PUK\r\n");
                lwcell_sim_puk_enter(sim.puk, sim.pin, puk_evt, NULL, 1);
            } else if (sim_state == LWCELL_SIM_STATE_NOT_READY) {
                printf("GSM SIM state not ready!\r\n");
            } else if (sim_state == LWCELL_SIM_STATE_NOT_INSERTED) {
                printf("GSM SIM not inserted!\r\n");
            }
            sim_prev_state = sim_state;
        }

        /* Some delay */
        lwcell_delay(1000);
    }

    /* Terminate thread */
    lwcell_sys_thread_terminate(NULL);
}

/**
 * \brief           Global GSM event function callback
 * \param[in]       cb: Event information
 * \return          lwcellOK on success, member of \ref lwcellr_t otherwise
 */
static lwcellr_t
lwcell_evt(lwcell_evt_t* evt) {
    switch (lwcell_evt_get_type(evt)) {
        case LWCELL_EVT_INIT_FINISH: {
            break;
        }
        case LWCELL_EVT_RESET: {
            if (lwcell_evt_reset_get_result(evt) == lwcellOK) {
                printf("Reset sequence finished with success!\r\n");
            }
            break;
        }
        case LWCELL_EVT_SIM_STATE_CHANGED: {            
            break;
        }
        case LWCELL_EVT_DEVICE_IDENTIFIED: {
            printf("Device has been identified!\r\n");
            break;
        }
        case LWCELL_EVT_SIGNAL_STRENGTH: {
            int16_t rssi = lwcell_evt_signal_strength_get_rssi(evt);
            printf("Signal strength: %d\r\n", (int)rssi);
            break;
        }
        case LWCELL_EVT_NETWORK_REG_CHANGED: {
            lwcell_network_reg_status_t status = lwcell_network_get_reg_status();
            printf("Network registration changed. New status: %d! ", (int)status);
            switch (status) {
                case LWCELL_NETWORK_REG_STATUS_CONNECTED: printf("Connected to home network!\r\n"); break;
                case LWCELL_NETWORK_REG_STATUS_CONNECTED_ROAMING: printf("Connected to network and roaming!\r\n"); break;
                case LWCELL_NETWORK_REG_STATUS_SEARCHING: printf("Searching for network!\r\n"); break;
                case LWCELL_NETWORK_REG_STATUS_SIM_ERR: printf("SIM error\r\n"); break;
                default: break;
            }
            break;
        }
        case LWCELL_EVT_NETWORK_OPERATOR_CURRENT: {
            const lwcell_operator_curr_t* op = lwcell_evt_network_operator_get_current(evt);
            if (op != NULL) {
                if (op->format == LWCELL_OPERATOR_FORMAT_LONG_NAME) {
                    printf("Operator long name: %s\r\n", op->data.long_name);
                } else if (op->format == LWCELL_OPERATOR_FORMAT_SHORT_NAME) {
                    printf("Operator short name: %s\r\n", op->data.short_name);
                } else if (op->format == LWCELL_OPERATOR_FORMAT_NUMBER) {
                    printf("Operator number: %d\r\n", (int)op->data.num);
                }
            }
            break;
        }
        case LWCELL_EVT_OPERATOR_SCAN: {
            lwcell_operator_t* ops;
            size_t length;

            printf("Operator scan finished!\r\n");
            if (lwcell_evt_operator_scan_get_result(evt) == lwcellOK) {
                ops = lwcell_evt_operator_scan_get_entries(evt);
                length = lwcell_evt_operator_scan_get_length(evt);

                for (size_t i = 0; i < length; i++) {
                    printf("Operator %2d: %s: %s\r\n", (int)i, ops[i].short_name, ops[i].long_name);
                }
            } else {
                printf("Operator scan failed!\r\n");
            }
            break;
        }
#if LWCELL_CFG_NETWORK
        case LWCELL_EVT_NETWORK_ATTACHED: {
            lwcell_ip_t ip;

            printf("\r\n---\r\n--- Network attached! ---\r\n---\r\n");
            if (lwcell_network_copy_ip(&ip) == lwcellOK) {
                printf("\r\n---\r\n--- IP: %d.%d.%d.%d ---\r\n---\r\n",
                    (int)ip.ip[0], (int)ip.ip[1], (int)ip.ip[2], (int)ip.ip[3]
                );
            }
            break;
        }
        case LWCELL_EVT_NETWORK_DETACHED: {
            printf("\r\n---\r\n--- Network detached! ---\r\n---\r\n");
            break;
        }
#endif /* LWCELL_CFG_NETWORK */
#if LWCELL_CFG_CALL
        case LWCELL_EVT_CALL_READY: {
            printf("Call is ready!\r\n");
            break;
        }
        case LWCELL_EVT_CALL_CHANGED: {
            const lwcell_call_t* call = evt->evt.call_changed.call;
            printf("Call changed!\r\n");
            if (call->state == LWCELL_CALL_STATE_ACTIVE) {
                printf("Call active!\r\n");
            } else if (call->state == LWCELL_CALL_STATE_INCOMING) {
                printf("Incoming call. Answering...\r\n");
            }
            break;
        }
#endif /* LWCELL_CFG_CALL */
#if LWCELL_CFG_SMS
        case LWCELL_EVT_SMS_READY: {
            printf("SMS is ready!\r\n");
            //lwcell_sms_send("+38640167724", "Device reset and ready for more operations!", 0);
            break;
        }
        case LWCELL_EVT_SMS_SEND: {
            if (evt->evt.sms_send.res == lwcellOK) {
                printf("SMS sent successfully!\r\n");
            } else {
                printf("SMS was not sent!\r\n");
            }
            break;
        }
        case LWCELL_EVT_SMS_RECV: {
            printf("SMS received: %d\r\n", (int)evt->evt.sms_recv.pos);
            lwcell_sms_read(evt->evt.sms_recv.mem, evt->evt.sms_recv.pos, &sms_entry, 0, NULL, NULL, 0);
            //lwcell_sms_delete(evt->evt.sms_recv.mem, evt->evt.sms_recv.pos, NULL, NULL, 0);
            break;
        }
        case LWCELL_EVT_SMS_READ: {
            lwcell_sms_entry_t* e = evt->evt.sms_read.entry;
            printf("SMS read: num: %s, name: %s, data: %s\r\n", e->number, e->name, e->data);
            break;
        }
        case LWCELL_EVT_SMS_LIST: {
            lwcell_sms_entry_t* e = evt->evt.sms_list.entries;
            size_t i;

            for (i = 0; i < evt->evt.sms_list.size; i++) {
                printf("SMS LIST: pos: %d, num: %s, content: %s\r\n",
                    (int)e->pos, e->number, e->data);
                e++;
            }
            break;
        }
#endif /* LWCELL_CFG_SMS */
#if LWCELL_CFG_PHONEBOOK
        case LWCELL_EVT_PB_LIST: {
            lwcell_pb_entry_t* e = evt->evt.pb_list.entries;
            size_t i;

            for (i = 0; i < evt->evt.pb_list.size; i++) {
                printf("PB LIST: pos: %d, num: %s, name: %s\r\n",
                    (int)e->pos, e->number, e->name);
                e++;
            }
            break;
        }
        case LWCELL_EVT_PB_SEARCH: {
            lwcell_pb_entry_t* e = evt->evt.pb_search.entries;
            size_t i;

            for (i = 0; i < evt->evt.pb_search.size; i++) {
                printf("PB READ search: pos: %d, num: %s, name: %s\r\n",
                    (int)e->pos, e->number, e->name);
                e++;
            }
            break;
        }
#endif /* LWCELL_CFG_PHONECALL */
        default: break;
    }
    return lwcellOK;
}

#if LWCELL_CFG_MEM_CUSTOM && 0

void *
lwcell_mem_malloc(size_t size) {
    void* ptr;

    while (WaitForSingleObject(allocation_mutex, INFINITE) != WAIT_OBJECT_0) {}
    ptr = malloc(size);
    ReleaseMutex(allocation_mutex);
    return ptr;
}

void *
lwcell_mem_realloc(void* ptr, size_t size) {
    void* p;

    while (WaitForSingleObject(allocation_mutex, INFINITE) != WAIT_OBJECT_0) {}
    p = realloc(ptr, size);
    ReleaseMutex(allocation_mutex);
    return p;
}

void *
lwcell_mem_calloc(size_t num, size_t size) {
    void* ptr;

    while (WaitForSingleObject(allocation_mutex, INFINITE) != WAIT_OBJECT_0) {}
    ptr = calloc(num, size);
    ReleaseMutex(allocation_mutex);
    return ptr;
}

void
lwcell_mem_free(void* ptr) {
    while (WaitForSingleObject(allocation_mutex, INFINITE) != WAIT_OBJECT_0) {}
    free(ptr);
    ReleaseMutex(allocation_mutex);
}
#endif /* LWCELL_CFG_MEM_CUSTOM */
