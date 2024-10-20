/**
 * \file            lwcell_mqtt.c
 * \brief           MQTT API
 */

/*
 * Copyright (c) 2024 Tilen MAJERLE
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
 * This file is part of LwCELL - Lightweight cellular modem AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.1
 */
#include "lwcell/lwcell_mqtt.h"
#include "lwcell/lwcell_private.h"

#if LWCELL_CFG_MQTT || __DOXYGEN__

lwcellr_t
lwcell_mqtt_connect(lwcell_mqtt_t** instance, const lwcell_mqtt_conn_desc_t* desc, lwcell_evt_fn evt_fn,
                    const uint32_t blocking) {
    LWCELL_UNUSED(instance);
    LWCELL_UNUSED(desc);
    LWCELL_UNUSED(evt_fn);
    LWCELL_UNUSED(blocking);
    return lwcellERRNOTENABLED;
}

lwcellr_t
lwcell_mqtt_disconnect(lwcell_mqtt_t* instance, const uint32_t blocking) {
    LWCELL_UNUSED(instance);
    LWCELL_UNUSED(blocking);
    return lwcellERRNOTENABLED;
}

lwcellr_t
lwcell_mqtt_subscribe(lwcell_mqtt_t* instance, const char* topic, uint8_t qos, const uint32_t blocking) {
    LWCELL_UNUSED(instance);
    LWCELL_UNUSED(topic);
    LWCELL_UNUSED(qos);
    LWCELL_UNUSED(blocking);
    return lwcellERRNOTENABLED;
}

lwcellr_t
lwcell_mqtt_unsubscribe(lwcell_mqtt_t* instance, const char* topic, const uint32_t blocking) {
    LWCELL_UNUSED(instance);
    LWCELL_UNUSED(topic);
    LWCELL_UNUSED(blocking);
    return lwcellERRNOTENABLED;
}

lwcellr_t
lwcell_mqtt_publish(lwcell_mqtt_t* instance, const char* topic, const void* data, size_t data_len,
                    const uint32_t blocking) {
    LWCELL_UNUSED(instance);
    LWCELL_UNUSED(topic);
    LWCELL_UNUSED(data);
    LWCELL_UNUSED(data_len);
    LWCELL_UNUSED(blocking);
    return lwcellERRNOTENABLED;
}

#endif /* LWCELL_CFG_MQTT || __DOXYGEN__ */
