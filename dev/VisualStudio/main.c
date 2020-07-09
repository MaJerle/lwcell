// lwgsm_dev_os.cpp : Defines the entry point for the console application.
//

#include "windows.h"
#include "lwgsm/lwgsm.h"

#include "lwgsm/apps/lwgsm_mqtt_client_api.h"
#include "lwgsm/lwgsm_mem.h"
#include "lwgsm/lwgsm_network_api.h"

#include "mqtt_client_api.h"
#include "netconn_client.h"
#include "sms_send_receive.h"
#include "network_apn_settings.h"
#include "sms_send_receive_thread.h"
#include "client.h"
#include "lwmem/lwmem.h"

static void main_thread(void* arg);
DWORD main_thread_id;

static lwgsmr_t lwgsm_evt(lwgsm_evt_t* evt);
static lwgsmr_t lwgsm_conn_evt(lwgsm_evt_t* evt);

lwgsm_operator_t operators[10];
size_t operators_len;

static lwgsm_sms_entry_t sms_entry;

lwgsm_sms_entry_t sms_entries[10];
size_t sms_entries_read;

lwgsm_pb_entry_t pb_entries[10];
size_t pb_entries_read;

lwgsm_operator_curr_t operator_curr;

char model_str[20];

typedef struct {
    const char* pin_default;
    const char* pin;
    const char* puk;
} my_sim_t;
my_sim_t sim = {
    .pin = "7958",
    .puk = "10663647",
};

uint8_t lwmem_region_1[0x4000];
lwmem_region_t lwmem_regions[] = {
    {lwmem_region_1, sizeof(lwmem_region_1)}
};

/**
 * \brief           Program entry point
 */
int
main() {
    printf("App start!\r\n");

    if (!lwmem_assignmem(lwmem_regions, sizeof(lwmem_regions) / sizeof(lwmem_regions[0]))) {
        printf("Could not assign memory for LwMEM!\r\n");
        return -1;
    }

    /* Create start main thread */
    CreateThread(0, 0, (LPTHREAD_START_ROUTINE)main_thread, NULL, 0, &main_thread_id);

    /* Do nothing at this point but do not close the program */
    while (1) {
        lwgsm_delay(1000);
    }
}

/* COnnection request data */
static const
uint8_t request_data[] = ""
"GET / HTTP/1.1\r\n"
"Host: example.com\r\n"
"Connection: Close\r\n"
"\r\n";

void
pin_evt(lwgsmr_t res, void* arg) {
    printf("PIN EVT function!\r\n");
}

void
puk_evt(lwgsmr_t res, void* arg) {
    printf("PUK EVT function!\r\n");
}

/**
 * \brief           Console input thread
 */
static void
input_thread(void* arg) {
    char buff[128];

#define IS_LINE(s)      (strncmp(buff, (s), sizeof(s) - 1) == 0)

    /* Notify user */
    printf("Start by writing commands..\r\n");

    /* Very simple input */
    while (1) {
        printf(" > ");
        fgets(buff, sizeof(buff), stdin);
        
        if (IS_LINE("reset")) {
            lwgsm_reset(NULL, NULL, 1);
        } else if (IS_LINE("devicemanufacturer")) {
            lwgsm_device_get_manufacturer(model_str, sizeof(model_str), NULL, NULL, 1);
            printf("Manuf: %s\r\n", model_str);
        } else if (IS_LINE("devicemodel")) {
            lwgsm_device_get_model(model_str, sizeof(model_str), NULL, NULL, 1);
            printf("Model: %s\r\n", model_str);
        } else if (IS_LINE("deviceserial")) {
            lwgsm_device_get_serial_number(model_str, sizeof(model_str), NULL, NULL, 1);
            printf("Serial: %s\r\n", model_str);
        } else if (IS_LINE("devicerevision")) {
            lwgsm_device_get_revision(model_str, sizeof(model_str), NULL, NULL, 1);
            printf("Revision: %s\r\n", model_str);
        } else if (IS_LINE("simstatus")) {
            switch (lwgsm_sim_get_current_state()) {
                case GSM_SIM_STATE_READY: printf("SIM state ready!\r\n"); break;
                case GSM_SIM_STATE_PIN: printf("SIM state PIN\r\n"); break;
                case GSM_SIM_STATE_PUK: printf("SIM state PIN\r\n"); break;
                case GSM_SIM_STATE_NOT_READY: printf("SIM state not ready\r\n"); break;
                case GSM_SIM_STATE_NOT_INSERTED: printf("SIM state not inserted\r\n"); break;
                default: printf("Unknown pin state\r\n"); break;
            }
        } else if (IS_LINE("simpinadd")) {
            lwgsm_sim_pin_add(sim.pin, NULL, NULL, 1);
        } else if (IS_LINE("simpinchange")) {
            lwgsm_sim_pin_change(sim.pin, "1234", NULL, NULL, 1);
            lwgsm_sim_pin_change("1234", sim.pin, NULL, NULL, 1);
        } else if (IS_LINE("simpinremove")) {
            lwgsm_sim_pin_remove(sim.pin, NULL, NULL, 1);
        } else if (IS_LINE("simpinenter")) {
            lwgsm_sim_pin_enter(sim.pin, NULL, NULL, 1);
        } else if (IS_LINE("simpuk")) {
            lwgsm_sim_puk_enter(sim.puk, sim.pin, puk_evt, NULL, 1);
        } else if (IS_LINE("operatorscan")) {
            lwgsm_operator_scan(operators, GSM_ARRAYSIZE(operators), &operators_len, NULL, NULL, 1);
        } else if (IS_LINE("join")) {
            lwgsm_network_request_attach();
        } else if (IS_LINE("quit")) {
            lwgsm_network_request_detach();
        } else if (IS_LINE("netconnclient")) {
            lwgsm_sys_sem_t sem;
            lwgsm_sys_sem_create(&sem, 0);
            lwgsm_sys_thread_create(NULL, "netconn_client", (lwgsm_sys_thread_fn)netconn_client_thread, &sem, GSM_SYS_THREAD_SS, GSM_SYS_THREAD_PRIO);
            lwgsm_sys_sem_wait(&sem, 0);
            lwgsm_sys_sem_delete(&sem);
#if GSM_CFG_SMS
        } else if (IS_LINE("smsenable")) {
            lwgsm_sms_enable(NULL, NULL, 1);
        } else if (IS_LINE("smsdisable")) {
            lwgsm_sms_disable(NULL, NULL, 1);
        } else if (IS_LINE("smssend")) {
            lwgsm_sms_send("+38631779982", "Hello world!", NULL, NULL, 1);
        } else if (IS_LINE("smslist")) {
            lwgsm_sms_list(GSM_MEM_CURRENT, GSM_SMS_STATUS_ALL, sms_entries, GSM_ARRAYSIZE(sms_entries), &sms_entries_read, 0, NULL, NULL, 1);
        } else if (IS_LINE("smsdeleteall")) {
            lwgsm_sms_delete_all(GSM_SMS_STATUS_ALL, NULL, NULL, 1);
        } else if (IS_LINE("smsthread")) {
            lwgsm_sys_thread_create(NULL, "sms_recv_send", (lwgsm_sys_thread_fn)sms_send_receive_thread, NULL, GSM_SYS_THREAD_SS, GSM_SYS_THREAD_PRIO);
#endif /* GSM_CFG_SMS */
#if GSM_CFG_CALL
        } else if (IS_LINE("callenable")) {
            lwgsm_call_enable(NULL, NULL, 1);
        } else if (IS_LINE("calldisable")) {
            lwgsm_call_disable(NULL, NULL, 1);
        } else if (IS_LINE("callstart")) {
            lwgsm_call_start("+38631779982", NULL, NULL, 1);
        } else if (IS_LINE("callhangup")) {
            lwgsm_call_hangup(NULL, NULL, 1);
        } else if (IS_LINE("callanswer")) {
            lwgsm_call_answer(NULL, NULL, 1);
#endif /* GSM_CFG_CALL */
#if GSM_CFG_PHONEBOOK
        } else if (IS_LINE("pbenable")) {
            lwgsm_pb_enable(NULL, NULL, 1);
        } else if (IS_LINE("pbread")) {
            lwgsm_pb_read(GSM_MEM_CURRENT, 1, pb_entries, NULL, NULL, 1);
        } else if (IS_LINE("pblist")) {
            lwgsm_pb_list(GSM_MEM_CURRENT, 1, pb_entries, GSM_ARRAYSIZE(pb_entries), &pb_entries_read, NULL, NULL, 1);
#endif /* GSM_CFG_PHONEBOOK */
        } else if (IS_LINE("mqttthread")) {
            lwgsm_sys_thread_create(NULL, "mqtt_client_api", (lwgsm_sys_thread_fn)mqtt_client_api_thread, NULL, GSM_SYS_THREAD_SS, GSM_SYS_THREAD_PRIO);
        } else if (IS_LINE("client")) {
            client_connect();
#if GSM_CFG_USSD
        } else if (IS_LINE("ussd")) {
            char response[128];
            lwgsm_ussd_run("*123#", response, sizeof(response), NULL, NULL, 1);
            printf("Command finished!\r\n");
#endif /* GSM_CFG_USSD */
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
    lwgsm_sim_state_t sim_state;

    /* Init GSM library */
    lwgsm_init(lwgsm_evt, 1);

    /* Set global network credentials */
    lwgsm_network_set_credentials(NETWORK_APN, NETWORK_APN_USER, NETWORK_APN_PASS);

    /* Start input thread */
    lwgsm_sys_thread_create(NULL, "input", (lwgsm_sys_thread_fn)input_thread, NULL, 0, GSM_SYS_THREAD_PRIO);

    while (1) {
        /* Check for sim card */
        while ((sim_state = lwgsm_sim_get_current_state()) != GSM_SIM_STATE_READY) {
            if (sim_state == GSM_SIM_STATE_PIN) {
                printf("GSM state PIN\r\n");
                lwgsm_sim_pin_enter(sim.pin, pin_evt, NULL, 1);
            } else if (sim_state == GSM_SIM_STATE_PUK) {
                printf("GSM state PUK\r\n");
                lwgsm_sim_puk_enter(sim.puk, sim.pin, puk_evt, NULL, 1);
            } else if (sim_state == GSM_SIM_STATE_NOT_READY) {
                printf("GSM SIM state not ready!\r\n");
            } else if (sim_state == GSM_SIM_STATE_NOT_INSERTED) {
                printf("GSM SIM not inserted!\r\n");
            }
            lwgsm_delay(1000);
        }

        /* Some delay */
        lwgsm_delay(1000);
    }

    /* Terminate thread */
    lwgsm_sys_thread_terminate(NULL);
}

static lwgsmr_t
lwgsm_conn_evt(lwgsm_evt_t* evt) {
    lwgsm_conn_p c;
    c = lwgsm_conn_get_from_evt(evt);
    switch (lwgsm_evt_get_type(evt)) {
#if GSM_CFG_CONN
        case GSM_EVT_CONN_ACTIVE: {
            printf("Connection active\r\n");
            //lwgsm_conn_send(c, request_data, sizeof(request_data) - 1, NULL, 0);
            break;
        }
        case GSM_EVT_CONN_ERROR: {
            printf("Connection error\r\n");
            break;
        }
        case GSM_EVT_CONN_CLOSE: {
            printf("Connection closed\r\n");
            break;
        }
        case GSM_EVT_CONN_SEND: {
            lwgsmr_t res = lwgsm_evt_conn_send_get_result(evt);
            if (res == gsmOK) {
                printf("Data sent!\r\n");
            } else {
                printf("Data send error!\r\n");
            }
            break;
        }
        case GSM_EVT_CONN_RECV: {
            lwgsm_pbuf_p p = lwgsm_evt_conn_recv_get_buff(evt);
            printf("DATA RECEIVED: %d\r\n", (int)lwgsm_pbuf_length(p, 1));
            lwgsm_conn_recved(c, p);
            break;
        }
#endif /* GSM_CFG_CONN */
        default: break;
    }
    return gsmOK;
}

/**
 * \brief           Global GSM event function callback
 * \param[in]       cb: Event information
 * \return          gsmOK on success, member of \ref lwgsmr_t otherwise
 */
static lwgsmr_t
lwgsm_evt(lwgsm_evt_t* evt) {
    switch (lwgsm_evt_get_type(evt)) {
        case GSM_EVT_INIT_FINISH: {
            break;
        }
        case GSM_EVT_RESET: {
            if (lwgsm_evt_reset_get_result(evt) == gsmOK) {
                printf("Reset sequence finished with success!\r\n");
            }
            break;
        }
        case GSM_EVT_SIM_STATE_CHANGED: {            
            break;
        }
        case GSM_EVT_DEVICE_IDENTIFIED: {
            printf("Device has been identified!\r\n");
            break;
        }
        case GSM_EVT_SIGNAL_STRENGTH: {
            int16_t rssi = lwgsm_evt_signal_strength_get_rssi(evt);
            printf("Signal strength: %d\r\n", (int)rssi);
            break;
        }
        case GSM_EVT_NETWORK_REG_CHANGED: {
            lwgsm_network_reg_status_t status = lwgsm_network_get_reg_status();
            printf("Network registration changed. New status: %d! ", (int)status);
            switch (status) {
                case GSM_NETWORK_REG_STATUS_CONNECTED: printf("Connected to home network!\r\n"); break;
                case GSM_NETWORK_REG_STATUS_CONNECTED_ROAMING: printf("Connected to network and roaming!\r\n"); break;
                case GSM_NETWORK_REG_STATUS_SEARCHING: printf("Searching for network!\r\n"); break;
                case GSM_NETWORK_REG_STATUS_SIM_ERR: printf("SIM error\r\n"); break;
                default: break;
            }
            break;
        }
        case GSM_EVT_NETWORK_OPERATOR_CURRENT: {
            const lwgsm_operator_curr_t* op = lwgsm_evt_network_operator_get_current(evt);
            if (op != NULL) {
                if (op->format == GSM_OPERATOR_FORMAT_LONG_NAME) {
                    printf("Operator long name: %s\r\n", op->data.long_name);
                } else if (op->format == GSM_OPERATOR_FORMAT_SHORT_NAME) {
                    printf("Operator short name: %s\r\n", op->data.short_name);
                } else if (op->format == GSM_OPERATOR_FORMAT_NUMBER) {
                    printf("Operator number: %d\r\n", (int)op->data.num);
                }
            }
            break;
        }
        case GSM_EVT_OPERATOR_SCAN: {
            lwgsm_operator_t* ops;
            size_t length;

            printf("Operator scan finished!\r\n");
            if (lwgsm_evt_operator_scan_get_result(evt) == gsmOK) {
                ops = lwgsm_evt_operator_scan_get_entries(evt);
                length = lwgsm_evt_operator_scan_get_length(evt);

                for (size_t i = 0; i < length; i++) {
                    printf("Operator %2d: %s: %s\r\n", (int)i, ops[i].short_name, ops[i].long_name);
                }
            } else {
                printf("Operator scan failed!\r\n");
            }
            break;
        }
#if GSM_CFG_NETWORK
        case GSM_EVT_NETWORK_ATTACHED: {
            lwgsm_ip_t ip;

            printf("\r\n---\r\n--- Network attached! ---\r\n---\r\n");
            if (lwgsm_network_copy_ip(&ip) == gsmOK) {
                printf("\r\n---\r\n--- IP: %d.%d.%d.%d ---\r\n---\r\n",
                    (int)ip.ip[0], (int)ip.ip[1], (int)ip.ip[2], (int)ip.ip[3]
                );
            }
            break;
        }
        case GSM_EVT_NETWORK_DETACHED: {
            printf("\r\n---\r\n--- Network detached! ---\r\n---\r\n");
            break;
        }
#endif /* GSM_CFG_NETWORK */
#if GSM_CFG_CALL
        case GSM_EVT_CALL_READY: {
            printf("Call is ready!\r\n");
            break;
        }
        case GSM_EVT_CALL_CHANGED: {
            const lwgsm_call_t* call = evt->evt.call_changed.call;
            printf("Call changed!\r\n");
            if (call->state == GSM_CALL_STATE_ACTIVE) {
                printf("Call active!\r\n");
            } else if (call->state == GSM_CALL_STATE_INCOMING) {
                printf("Incoming call. Answering...\r\n");
            }
            break;
        }
#endif /* GSM_CFG_CALL */
#if GSM_CFG_SMS
        case GSM_EVT_SMS_READY: {
            printf("SMS is ready!\r\n");
            //lwgsm_sms_send("+38640167724", "Device reset and ready for more operations!", 0);
            break;
        }
        case GSM_EVT_SMS_SEND: {
            if (evt->evt.sms_send.res == gsmOK) {
                printf("SMS sent successfully!\r\n");
            } else {
                printf("SMS was not sent!\r\n");
            }
            break;
        }
        case GSM_EVT_SMS_RECV: {
            printf("SMS received: %d\r\n", (int)evt->evt.sms_recv.pos);
            lwgsm_sms_read(evt->evt.sms_recv.mem, evt->evt.sms_recv.pos, &sms_entry, 0, NULL, NULL, 0);
            //lwgsm_sms_delete(evt->evt.sms_recv.mem, evt->evt.sms_recv.pos, NULL, NULL, 0);
            break;
        }
        case GSM_EVT_SMS_READ: {
            lwgsm_sms_entry_t* e = evt->evt.sms_read.entry;
            printf("SMS read: num: %s, name: %s, data: %s\r\n", e->number, e->name, e->data);
            break;
        }
        case GSM_EVT_SMS_LIST: {
            lwgsm_sms_entry_t* e = evt->evt.sms_list.entries;
            size_t i;

            for (i = 0; i < evt->evt.sms_list.size; i++) {
                printf("SMS LIST: pos: %d, num: %s, content: %s\r\n",
                    (int)e->pos, e->number, e->data);
                e++;
            }
            break;
        }
#endif /* GSM_CFG_SMS */
#if GSM_CFG_PHONEBOOK
        case GSM_EVT_PB_LIST: {
            lwgsm_pb_entry_t* e = evt->evt.pb_list.entries;
            size_t i;

            for (i = 0; i < evt->evt.pb_list.size; i++) {
                printf("PB LIST: pos: %d, num: %s, name: %s\r\n",
                    (int)e->pos, e->number, e->name);
                e++;
            }
            break;
        }
        case GSM_EVT_PB_SEARCH: {
            lwgsm_pb_entry_t* e = evt->evt.pb_search.entries;
            size_t i;

            for (i = 0; i < evt->evt.pb_search.size; i++) {
                printf("PB READ search: pos: %d, num: %s, name: %s\r\n",
                    (int)e->pos, e->number, e->name);
                e++;
            }
            break;
        }
#endif /* GSM_CFG_PHONECALL */
        default: break;
    }
    return gsmOK;
}

#if GSM_CFG_MEM_CUSTOM && 0

void *
lwgsm_mem_malloc(size_t size) {
    void* ptr;

    while (WaitForSingleObject(allocation_mutex, INFINITE) != WAIT_OBJECT_0) {}
    ptr = malloc(size);
    ReleaseMutex(allocation_mutex);
    return ptr;
}

void *
lwgsm_mem_realloc(void* ptr, size_t size) {
    void* p;

    while (WaitForSingleObject(allocation_mutex, INFINITE) != WAIT_OBJECT_0) {}
    p = realloc(ptr, size);
    ReleaseMutex(allocation_mutex);
    return p;
}

void *
lwgsm_mem_calloc(size_t num, size_t size) {
    void* ptr;

    while (WaitForSingleObject(allocation_mutex, INFINITE) != WAIT_OBJECT_0) {}
    ptr = calloc(num, size);
    ReleaseMutex(allocation_mutex);
    return ptr;
}

void
lwgsm_mem_free(void* ptr) {
    while (WaitForSingleObject(allocation_mutex, INFINITE) != WAIT_OBJECT_0) {}
    free(ptr);
    ReleaseMutex(allocation_mutex);
}
#endif /* GSM_CFG_MEM_CUSTOM */
