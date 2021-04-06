/**
 * \file            lwgsm_fs.c
 * \brief           File System functions
 */

/*
 * Copyright (c) 2020 Tilen MAJERLE
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of LwGSM - Lightweight GSM-AT library.
 *
 * Authors:         Tilen MAJERLE <tilen@majerle.eu>,
 *                  Ilya Kargapolov <d3vil.st@gmail.com>
 * Version:         v0.1.0
 */
#include <lwgsm/lwgsm_private.h>
#include "lwgsm/lwgsm_mem.h"

#if LWGSM_CFG_MQTT || __DOXYGEN__

/**
 * \brief           Configure MQTT client
 * \param[in]       param: Parameter name
 * \param[in]       value: Value of parameter
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_mqtt_conf(const char* param, const char* value, const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_MQTT_CONF;
    LWGSM_MSG_VAR_REF(msg).msg.mqtt.conf.param = param;
    LWGSM_MSG_VAR_REF(msg).msg.mqtt.conf.value = value;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

/**
 * \brief           Configure MQTT SSL Mode
 * \param[in]       value: `1` enable SSL, `0` Disable SSL
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_mqtt_ssl(uint8_t value, const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_MQTT_SSL;
    LWGSM_MSG_VAR_REF(msg).msg.mqtt.ssl = value;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

/**
 * \brief           Connect to a MQTT Server
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_mqtt_connect(const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_MQTT_CONNECT;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

/**
 * \brief           Disconnect from a MQTT Server
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_mqtt_disconnect(const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_MQTT_DISCONNECT;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

/**
 * \brief           Subscribe to a topic
 * \param[in]       topic: topic name
 * \param[in]       qos: QoS value
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_mqtt_subscribe(const char* topic, uint8_t qos, const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_MQTT_SUBSCRIBE;
    LWGSM_MSG_VAR_REF(msg).msg.mqtt.topic.topic = topic;
    LWGSM_MSG_VAR_REF(msg).msg.mqtt.topic.qos = qos;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

/**
 * \brief           Unsubscribe from a topic
 * \param[in]       topic: topic name
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_mqtt_unsubscribe(const char* topic, const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_MQTT_UNSUBSCRIBE;
    LWGSM_MSG_VAR_REF(msg).msg.mqtt.topic.topic = topic;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

/**
 * \brief           Publish a message to a topic
 * \param[in]       topic: topic name
 * \param[in]       message: content of a message
 * \param[in]       qos: QoS value
 * \param[in]       retain: Retain flag of message
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_mqtt_publish(char *topic, char *message, uint8_t qos, uint8_t retain, const lwgsm_api_cmd_evt_fn evt_fn,
                   void *const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_MQTT_PUBLISH;
    LWGSM_MSG_VAR_REF(msg).msg.mqtt.topic.topic = topic;
    LWGSM_MSG_VAR_REF(msg).msg.mqtt.topic.qos = qos;
    LWGSM_MSG_VAR_REF(msg).msg.mqtt.topic.retain = retain;
    LWGSM_MSG_VAR_REF(msg).msg.mqtt.message = message;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

/**
 * \brief           Get MQTT connection status
 * \param[out]      state: Pointer to return result. Set to `NULL` when not used
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_mqtt_state(void* state, const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_MQTT_STATE;
    LWGSM_MSG_VAR_REF(msg).msg.mqtt.state = state;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

#endif /* LWGSM_CFG_MQTT || __DOXYGEN__ */