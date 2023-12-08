/*
 * MQTT client API example with GSM device.
 *
 * Once device is connected to network,
 * it will try to connect to mosquitto test server and start the MQTT.
 *
 * If successfully connected, it will publish data to "lwcell_mqtt_topic" topic every x seconds.
 *
 * To check if data are sent, you can use mqtt-spy PC software to inspect
 * test.mosquitto.org server and subscribe to publishing topic
 */

#include "mqtt_client_api.h"
#include "lwcell/apps/lwcell_mqtt_client_api.h"
#include "lwcell/lwcell.h"
#include "lwcell/lwcell_mem.h"
#include "lwcell/lwcell_network_api.h"

/**
 * \brief           Connection information for MQTT CONNECT packet
 */
static const lwcell_mqtt_client_info_t mqtt_client_info = {
    .keep_alive = 10,

    /* Server login data */
    .user = "8a215f70-a644-11e8-ac49-e932ed599553",
    .pass = "26aa943f702e5e780f015cd048a91e8fb54cca28",

    /* Device identifier address */
    .id = "2c3573a0-0176-11e9-a056-c5cffe7f75f9",
};

/**
 * \brief           Memory for temporary topic
 */
static char mqtt_topic_str[256];

/**
 * \brief           Generate random number and write it to string
 * \param[out]      str: Output string with new number
 */
static void
generate_random(char* str) {
    static uint32_t random_beg = 0x8916;
    random_beg = random_beg * 0x00123455 + 0x85654321;
    sprintf(str, "%u", (unsigned)((random_beg >> 8) & 0xFFFF));
}

/**
 * \brief           MQTT client API thread
 */
void
lwcell_mqtt_client_api_thread(void const* arg) {
    lwcell_mqtt_client_api_p client;
    lwcell_mqtt_conn_status_t conn_status;
    lwcell_mqtt_client_api_buf_p buf;
    lwcellr_t res;
    char random_str[10];

    LWCELL_UNUSED(arg);

    /* Request network attach */
    while (lwcell_network_request_attach() != lwcellOK) {
        lwcell_delay(1000);
    }

    /* Create new MQTT API */
    if ((client = lwcell_mqtt_client_api_new(256, 128)) == NULL) {
        goto terminate;
    }

    while (1) {
        /* Make a connection */
        printf("Joining MQTT server\r\n");

        /* Try to join */
        conn_status = lwcell_mqtt_client_api_connect(client, "mqtt.mydevices.com", 1883, &mqtt_client_info);
        if (conn_status == LWCELL_MQTT_CONN_STATUS_ACCEPTED) {
            printf("Connected and accepted!\r\n");
            printf("Client is ready to subscribe and publish to new messages\r\n");
        } else {
            printf("Connect API response: %d\r\n", (int)conn_status);
            lwcell_delay(5000);
            continue;
        }

        /* Subscribe to topics */
        sprintf(mqtt_topic_str, "v1/%s/things/%s/cmd/#", mqtt_client_info.user, mqtt_client_info.id);
        if (lwcell_mqtt_client_api_subscribe(client, mqtt_topic_str, LWCELL_MQTT_QOS_AT_LEAST_ONCE) == lwcellOK) {
            printf("Subscribed to topic\r\n");
        } else {
            printf("Problem subscribing to topic!\r\n");
        }

        while (1) {
            /* Receive MQTT packet with timeout */
            if ((res = lwcell_mqtt_client_api_receive(client, &buf, 5000)) == lwcellOK) {
                if (buf != NULL) {
                    printf("Publish received!\r\n");
                    printf("Topic: %s, payload: %s\r\n", buf->topic, buf->payload);
                    lwcell_mqtt_client_api_buf_free(buf);
                    buf = NULL;
                }
            } else if (res == lwcellCLOSED) {
                printf("MQTT connection closed!\r\n");
                break;
            } else if (res == lwcellTIMEOUT) {
                printf("Timeout on MQTT receive function. Manually publishing.\r\n");

                /* Publish data on channel 1 */
                generate_random(random_str);
                sprintf(mqtt_topic_str, "v1/%s/things/%s/data/1", mqtt_client_info.user, mqtt_client_info.id);
                lwcell_mqtt_client_api_publish(client, mqtt_topic_str, random_str, strlen(random_str),
                                               LWCELL_MQTT_QOS_AT_LEAST_ONCE, 0);
            }
        }
        goto terminate;
    }

terminate:
    lwcell_mqtt_client_api_delete(client);
    lwcell_network_request_detach();
    printf("MQTT client thread terminate\r\n");
    lwcell_sys_thread_terminate(NULL);
}
