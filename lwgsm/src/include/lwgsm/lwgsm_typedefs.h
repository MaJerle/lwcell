/**
 * \file            lwgsm_typedefs.h
 * \brief           List of structures and enumerations for public usage
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
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.0
 */
#ifndef LWGSM_HDR_DEFS_H
#define LWGSM_HDR_DEFS_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \ingroup         LWGSM
 * \defgroup        LWGSM_TYPEDEFS Structures and enumerations
 * \brief           List of core structures and enumerations
 * \{
 */

/**
 * \}
 */

/**
 * \ingroup         LWGSM_TYPEDEFS
 * \brief           Result enumeration used across application functions
 */
typedef enum {
    lwgsmOK = 0,                                /*!< Function returned OK */
    lwgsmOKIGNOREMORE,                          /*!< Function succedded, should continue as \ref lwgsmOK
                                                        but ignore sending more data.
                                                        This result is possible on connection data receive callback */
    lwgsmERR,                                   /*!< Generic error */
    lwgsmPARERR,                                /*!< Wrong parameters on function call */
    lwgsmERRMEM,                                /*!< Memory error occurred */
    lwgsmTIMEOUT,                               /*!< Timeout occurred on command */
    lwgsmCONT,                                  /*!< There is still some command to be processed in current command */
    lwgsmCLOSED,                                /*!< Connection just closed */
    lwgsmINPROG,                                /*!< Operation is in progress */

    lwgsmERRNOTENABLED,                         /*!< Feature not enabled error */
    lwgsmERRNOIP,                               /*!< Station does not have IP address */
    lwgsmERRNOFREECONN,                         /*!< There is no free connection available to start */
    lwgsmERRCONNTIMEOUT,                        /*!< Timeout received when connection to access point */
    lwgsmERRPASS,                               /*!< Invalid password for access point */
    lwgsmERRNOAP,                               /*!< No access point found with specific SSID and MAC address */
    lwgsmERRCONNFAIL,                           /*!< Connection failed to access point */
    lwgsmERRWIFINOTCONNECTED,                   /*!< Wifi not connected to access point */
    lwgsmERRNODEVICE,                           /*!< Device is not present */
    lwgsmERRBLOCKING,                           /*!< Blocking mode command is not allowed */
} lwgsmr_t;

/**
 * \ingroup         LWGSM_TYPEDEFS
 * \brief           GSM device model type
 */
typedef enum {
#define LWGSM_DEVICE_MODEL_ENTRY(name, str_id, is_2g, is_lte)     LWGSM_DEVICE_MODEL_ ## name,
#include "lwgsm/lwgsm_models.h"
    LWGSM_DEVICE_MODEL_END,                     /*!< End of device model */
    LWGSM_DEVICE_MODEL_UNKNOWN,                 /*!< Unknown device model */
} lwgsm_device_model_t;

/**
 * \ingroup         LWGSM_SIM
 * \brief           SIM state
 */
typedef enum {
    LWGSM_SIM_STATE_NOT_INSERTED,               /*!< SIM is not inserted in socket */
    LWGSM_SIM_STATE_READY,                      /*!< SIM is ready for operations */
    LWGSM_SIM_STATE_NOT_READY,                  /*!< SIM is not ready for any operation */
    LWGSM_SIM_STATE_PIN,                        /*!< SIM is waiting for SIM to be given */
    LWGSM_SIM_STATE_PUK,                        /*!< SIM is waiting for PUT to be given */
    LWGSM_SIM_STATE_PH_PIN,
    LWGSM_SIM_STATE_PH_PUK,
} lwgsm_sim_state_t;

/**
 * \ingroup         LWGSM_TYPEDEFS
 * \brief           IP structure
 */
typedef struct {
    uint8_t ip[4];                              /*!< IPv4 address */
} lwgsm_ip_t;

/**
 * \ingroup         LWGSM_TYPEDEFS
 * \brief           Port variable
 */
typedef uint16_t    lwgsm_port_t;

/**
 * \ingroup         LWGSM_TYPEDEFS
 * \brief           MAC address
 */
typedef struct {
    uint8_t mac[6];                             /*!< MAC address */
} lwgsm_mac_t;

/**
 * \ingroup         LWGSM_TYPEDEFS
 * \brief           Date and time structure
 */
typedef struct {
    uint8_t date;                               /*!< Day in a month, from `1` to up to `31` */
    uint8_t month;                              /*!< Month in a year, from `1` to `12` */
    uint16_t year;                              /*!< Year */
    uint8_t day;                                /*!< Day in a week, from `1` to `7`, 0 = invalid */
    uint8_t hours;                              /*!< Hours in a day, from `0` to `23` */
    uint8_t minutes;                            /*!< Minutes in a hour, from `0` to `59` */
    uint8_t seconds;                            /*!< Seconds in a minute, from `0` to `59` */
} lwgsm_datetime_t;

/**
 * \ingroup         LWGSM_CONN
 * \brief           List of possible connection types
 */
typedef enum {
    LWGSM_CONN_TYPE_TCP,                        /*!< Connection type is TCP */
    LWGSM_CONN_TYPE_UDP,                        /*!< Connection type is UDP */
    LWGSM_CONN_TYPE_SSL,                        /*!< Connection type is TCP over SSL */
} lwgsm_conn_type_t;

/**
 * \ingroup         LWGSM_TYPEDEFS
 * \brief           Available device memories
 */
typedef enum {
#define LWGSM_DEV_MEMORY_ENTRY(name, str_code)    LWGSM_MEM_ ## name,
#include "lwgsm/lwgsm_memories.h"
    LWGSM_MEM_END,                              /*!< End of memory list */
    LWGSM_MEM_CURRENT,                          /*!< Use current memory for read/delete operation */
    LWGSM_MEM_UNKNOWN = 0x1F,                   /*!< Unknown memory */
} lwgsm_mem_t;

/**
 * \ingroup         LWGSM_TYPEDEFS
 * \brief           GSM number type
 */
typedef enum {
    LWGSM_NUMBER_TYPE_NATIONAL = 129,           /*!< Number is national */
    LWGSM_NUMBER_TYPE_INTERNATIONAL = 145,      /*!< Number is international */
} lwgsm_number_type_t;

/**
 * \ingroup         LWGSM_SMS
 * \brief           SMS status in current memory
 */
typedef enum {
    LWGSM_SMS_STATUS_ALL,                       /*!< Process all SMS, used for mass delete or SMS list */
    LWGSM_SMS_STATUS_READ,                      /*!< SMS status is read */
    LWGSM_SMS_STATUS_UNREAD,                    /*!< SMS status is unread */
    LWGSM_SMS_STATUS_SENT,                      /*!< SMS status is sent */
    LWGSM_SMS_STATUS_UNSENT,                    /*!< SMS status is unsent */
    LWGSM_SMS_STATUS_INBOX,                     /*!< SMS status, used only for mass delete operation */
} lwgsm_sms_status_t;

/**
 * \ingroup         LWGSM_SMS
 * \brief           SMS entry structure
 */
typedef struct {
    lwgsm_mem_t mem;                            /*!< Memory storage */
    size_t pos;                                 /*!< Memory position */
    lwgsm_datetime_t datetime;                  /*!< Date and time */
    lwgsm_sms_status_t status;                  /*!< Message status */
    char number[26];                            /*!< Phone number */
    char name[20];                              /*!< Name in phonebook if exists */
    char data[161];                             /*!< Data memory */
    size_t length;                              /*!< Length of SMS data */
} lwgsm_sms_entry_t;

/**
 * \ingroup         LWGSM_PB
 * \brief           Phonebook entry structure
 */
typedef struct {
    lwgsm_mem_t mem;                            /*!< Memory position */
    size_t pos;                                 /*!< Position in memory */
    char name[20];                              /*!< Name of phonebook entry */
    char number[26];                            /*!< Phone number */
    lwgsm_number_type_t type;                   /*!< Phone number type */
} lwgsm_pb_entry_t;

/**
 * \ingroup         LWGSM_OPERATOR
 * \brief           Operator status value
 */
typedef enum {
    LWGSM_OPERATOR_STATUS_UNKNOWN = 0x00,       /*!< Unknown operator */
    LWGSM_OPERATOR_STATUS_AVAILABLE,            /*!< Operator is available */
    LWGSM_OPERATOR_STATUS_CURRENT,              /*!< Operator is currently active */
    LWGSM_OPERATOR_STATUS_FORBIDDEN             /*!< Operator is forbidden */
} lwgsm_operator_status_t;

/**
 * \ingroup         LWGSM_OPERATOR
 * \brief           Operator selection mode
 */
typedef enum {
    LWGSM_OPERATOR_MODE_AUTO = 0x00,            /*!< Operator automatic mode */
    LWGSM_OPERATOR_MODE_MANUAL = 0x01,          /*!< Operator manual mode */
    LWGSM_OPERATOR_MODE_DEREGISTER = 0x02,      /*!< Operator deregistered from network */
    LWGSM_OPERATOR_MODE_MANUAL_AUTO = 0x04,     /*!< Operator manual mode first. If fails, auto mode enabled */
} lwgsm_operator_mode_t;

/**
 * \ingroup         LWGSM_OPERATOR
 * \brief           Operator data format
 */
typedef enum {
    LWGSM_OPERATOR_FORMAT_LONG_NAME = 0x00,     /*!< COPS command returned long name */
    LWGSM_OPERATOR_FORMAT_SHORT_NAME,           /*!< COPS command returned short name */
    LWGSM_OPERATOR_FORMAT_NUMBER,               /*!< COPS command returned number */
    LWGSM_OPERATOR_FORMAT_INVALID               /*!< Unknown format */
} lwgsm_operator_format_t;

/**
 * \ingroup         LWGSM_OPERATOR
 * \brief           Operator details for scan
 */
typedef struct {
    lwgsm_operator_status_t stat;               /*!< Operator status */
    char long_name[20];                         /*!< Operator long name */
    char short_name[20];                        /*!< Operator short name */
    uint32_t num;                               /*!< Operator numeric value */
} lwgsm_operator_t;

/**
 * \ingroup         LWGSM_OPERATOR
 * \brief           Current operator info
 */
typedef struct {
    lwgsm_operator_mode_t mode;                 /*!< Operator mode */
    lwgsm_operator_format_t format;             /*!< Data format */
    union {
        char long_name[20];                     /*!< Long name format */
        char short_name[20];                    /*!< Short name format */
        uint32_t num;                           /*!< Number format */
    } data;                                     /*!< Operator data union */
} lwgsm_operator_curr_t;

/**
 * \ingroup         LWGSM_NETWORK
 * \brief           Network Registration status
 */
typedef enum {
    LWGSM_NETWORK_REG_STATUS_SIM_ERR = 0x00,    /*!< SIM card error */
    LWGSM_NETWORK_REG_STATUS_CONNECTED = 0x01,  /*!< Device is connected to network */
    LWGSM_NETWORK_REG_STATUS_SEARCHING = 0x02,  /*!< Network search is in progress */
    LWGSM_NETWORK_REG_STATUS_DENIED = 0x03,     /*!< Registration denied */
    LWGSM_NETWORK_REG_STATUS_CONNECTED_ROAMING = 0x05,  /*!< Device is connected and is roaming */
    LWGSM_NETWORK_REG_STATUS_CONNECTED_SMS_ONLY = 0x06, /*!< Device is connected to home network in SMS-only mode */
    LWGSM_NETWORK_REG_STATUS_CONNECTED_ROAMING_SMS_ONLY = 0x07  /*!< Device is roaming in SMS-only mode */
} lwgsm_network_reg_status_t;

/**
 * \ingroup         LWGSM_CALL
 * \brief           List of call directions
 */
typedef enum {
    LWGSM_CALL_DIR_MO = 0x00,                   /*!< Mobile Originated, outgoing call */
    LWGSM_CALL_DIR_MT,                          /*!< Mobile Terminated, incoming call */
} lwgsm_call_dir_t;

/**
 * \ingroup         LWGSM_CALL
 * \brief           List of call states
 */
typedef enum {
    LWGSM_CALL_STATE_ACTIVE = 0x00,             /*!< Call is active */
    LWGSM_CALL_STATE_HELD,                      /*!< Call is held */
    LWGSM_CALL_STATE_DIALING,                   /*!< Call is dialing */
    LWGSM_CALL_STATE_ALERTING,                  /*!< Call is alerting */
    LWGSM_CALL_STATE_INCOMING,                  /*!< Call is incoming */
    LWGSM_CALL_STATE_WAITING,                   /*!< Call is waiting */
    LWGSM_CALL_STATE_DISCONNECT,                /*!< Call disconnected, call finished */
} lwgsm_call_state_t;

/**
 * \ingroup         LWGSM_CALL
 * \brief           List of call types
 */
typedef enum {
    LWGSM_CALL_TYPE_VOICE = 0x00,               /*!< Voice call */
    LWGSM_CALL_TYPE_DATA,                       /*!< Data call */
    LWGSM_CALL_TYPE_FAX,                        /*!< Fax call */
} lwgsm_call_type_t;

/**
 * \ingroup         LWGSM_CALL
 * \brief           Call information
 * \note            Data received on `+CLCC` info
 */
typedef struct {
    uint8_t ready;                              /*!< Flag indicating feature ready by device */
    uint8_t enabled;                            /*!< Flag indicating feature enabled */

    uint8_t id;                                 /*!< Call identification number, 0-7 */
    lwgsm_call_dir_t dir;                       /*!< Call direction */
    lwgsm_call_state_t state;                   /*!< Call state */
    lwgsm_call_type_t type;                     /*!< Call type */
    char number[20];                            /*!< Phone number */
    char is_multipart;                          /*!< Multipart status */
    uint8_t addr_type;                          /*!< Address type */
    char name[20];                              /*!< Phone book name if exists for current number */
} lwgsm_call_t;

/* Forward declarations */
struct lwgsm_evt;
struct lwgsm_conn;
struct lwgsm_pbuf;

/**
 * \ingroup         LWGSM_CONN
 * \brief           Pointer to \ref lwgsm_conn_t structure
 */
typedef struct lwgsm_conn* lwgsm_conn_p;

/**
 * \ingroup         LWGSM_PBUF
 * \brief           Pointer to \ref lwgsm_pbuf_t structure
 */
typedef struct lwgsm_pbuf* lwgsm_pbuf_p;

/**
 * \ingroup         LWGSM_EVT
 * \brief           Event function prototype
 * \param[in]       evt: Callback event data
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
typedef lwgsmr_t  (*lwgsm_evt_fn)(struct lwgsm_evt* evt);

/**
 * \ingroup         LWGSM_EVT
 * \brief           List of possible callback types received to user
 */
typedef enum lwgsm_cb_type_t {
    LWGSM_EVT_INIT_FINISH,                      /*!< Initialization has been finished at this point */

    LWGSM_EVT_RESET,                            /*!< Device reset operation finished */
    LWGSM_EVT_RESTORE,                          /*!< Device restore operation finished */

    LWGSM_EVT_CMD_TIMEOUT,                      /*!< Timeout on command.
                                                        When application receives this event,
                                                        it may reset system as there was (maybe) a problem in device */

    LWGSM_EVT_DEVICE_PRESENT,                   /*!< Notification when device present status changes */
    LWGSM_EVT_DEVICE_IDENTIFIED,                /*!< Device identified event */

    LWGSM_EVT_SIGNAL_STRENGTH,                  /*!< Signal strength event */

    LWGSM_EVT_SIM_STATE_CHANGED,                /*!< SIM card state changed */

    LWGSM_EVT_OPERATOR_SCAN,                    /*!< Operator scan finished event */

    LWGSM_EVT_NETWORK_OPERATOR_CURRENT,         /*!< Current operator event */
    LWGSM_EVT_NETWORK_REG_CHANGED,              /*!< Network registration changed.
                                                         Available even when \ref LWGSM_CFG_NETWORK is disabled */
#if LWGSM_CFG_NETWORK || __DOXYGEN__
    LWGSM_EVT_NETWORK_ATTACHED,                 /*!< Attached to network, PDP context active and ready for TCP/IP application */
    LWGSM_EVT_NETWORK_DETACHED,                 /*!< Detached from network, PDP context not active anymore */
#endif /* LWGSM_CFG_NETWORK || __DOXYGEN__ */

#if LWGSM_CFG_CONN || __DOXYGEN__
    LWGSM_EVT_CONN_RECV,                        /*!< Connection data received */
    LWGSM_EVT_CONN_SEND,                        /*!< Connection data send */
    LWGSM_EVT_CONN_ACTIVE,                      /*!< Connection just became active */
    LWGSM_EVT_CONN_ERROR,                       /*!< Client connection start was not successful */
    LWGSM_EVT_CONN_CLOSE,                       /*!< Connection close event. Check status if successful */
    LWGSM_EVT_CONN_POLL,                        /*!< Poll for connection if there are any changes */
#endif /* LWGSM_CFG_CONN || __DOXYGEN__ */

#if LWGSM_CFG_SMS || __DOXYGEN__
    LWGSM_EVT_SMS_ENABLE,                       /*!< SMS enable event */
    LWGSM_EVT_SMS_READY,                        /*!< SMS ready event */
    LWGSM_EVT_SMS_SEND,                         /*!< SMS send event */
    LWGSM_EVT_SMS_RECV,                         /*!< SMS received */
    LWGSM_EVT_SMS_READ,                         /*!< SMS read */
    LWGSM_EVT_SMS_DELETE,                       /*!< SMS delete */
    LWGSM_EVT_SMS_LIST,                         /*!< SMS list */
#endif /* LWGSM_CFG_SMS || __DOXYGEN__ */
#if LWGSM_CFG_CALL || __DOXYGEN__
    LWGSM_EVT_CALL_ENABLE,                      /*!< Call enable event */
    LWGSM_EVT_CALL_READY,                       /*!< Call ready event */
    LWGSM_EVT_CALL_CHANGED,                     /*!< Call info changed, `+CLCK` statement received */
    LWGSM_EVT_CALL_RING,                        /*!< Call is ringing event */
    LWGSM_EVT_CALL_BUSY,                        /*!< Call is busy */
    LWGSM_EVT_CALL_NO_CARRIER,                  /*!< No carrier to make a call */
#endif /* LWGSM_CFG_CALL || __DOXYGEN__ */
#if LWGSM_CFG_PHONEBOOK || __DOXYGEN__
    LWGSM_EVT_PB_ENABLE,                        /*!< Phonebook enable event */
    LWGSM_EVT_PB_LIST,                          /*!< Phonebook list event */
    LWGSM_EVT_PB_SEARCH,                        /*!< Phonebook search event */
#endif /* LWGSM_CFG_PHONEBOOK || __DOXYGEN__ */
} lwgsm_evt_type_t;

/**
 * \ingroup         LWGSM_EVT
 * \brief           Global callback structure to pass as parameter to callback function
 */
typedef struct lwgsm_evt {
    lwgsm_evt_type_t type;                      /*!< Callback type */
    union {
        struct {
            lwgsmr_t res;                       /*!< Reset operation result */
        } reset;                                /*!< Reset sequence finish. Use with \ref LWGSM_EVT_RESET event */
        struct {
            lwgsmr_t res;                       /*!< Restore operation result */
        } restore;                              /*!< Restore sequence finish. Use with \ref LWGSM_EVT_RESTORE event */

        struct {
            lwgsm_sim_state_t state;            /*!< SIM state */
        } cpin;                                 /*!< CPIN event */
        struct {
            const lwgsm_operator_curr_t* operator_current;  /*!< Current operator info */
        } operator_current;                     /*!< Current operator event. Use with \ref LWGSM_EVT_NETWORK_OPERATOR_CURRENT event */
        struct {
            lwgsm_operator_t* ops;              /*!< Pointer to operators */
            size_t opf;                         /*!< Number of operators found */
            lwgsmr_t res;                       /*!< Scan operation result */
        } operator_scan;                        /*!< Operator scan event. Use with \ref LWGSM_EVT_OPERATOR_SCAN event */

        struct {
            int16_t rssi;                       /*!< Strength in units of dBm */
        } rssi;                                 /*!< Signal strength event. Use with \ref LWGSM_EVT_SIGNAL_STRENGTH event */

#if LWGSM_CFG_CONN || __DOXYGEN__
        struct {
            lwgsm_conn_p conn;                  /*!< Connection where data were received */
            lwgsm_pbuf_p buff;                  /*!< Pointer to received data */
        } conn_data_recv;                       /*!< Network data received. Use with \ref LWGSM_EVT_CONN_RECV event */
        struct {
            lwgsm_conn_p conn;                  /*!< Connection where data were sent */
            size_t sent;                        /*!< Number of bytes sent on connection */
            lwgsmr_t res;                       /*!< Send data result */
        } conn_data_send;                       /*!< Data successfully sent. Use with \ref LWGSM_EVT_CONN_SEND event */
        struct {
            const char* host;                   /*!< Host to use for connection */
            lwgsm_port_t port;                  /*!< Remote port used for connection */
            lwgsm_conn_type_t type;             /*!< Connection type */
            void* arg;                          /*!< Connection argument used on connection */
            lwgsmr_t err;                       /*!< Error value */
        } conn_error;                           /*!< Client connection start error. Use with \ref LWGSM_EVT_CONN_ERROR event */
        struct {
            lwgsm_conn_p conn;                  /*!< Pointer to connection */
            uint8_t client;                     /*!< Set to `1` if connection is/was client mode */
            uint8_t forced;                     /*!< Set to `1` if connection action was forced */
            lwgsmr_t res;                       /*!< Result of close event. Set to \ref lwgsmOK on success. */
        } conn_active_close;                    /*!< Process active and closed statuses at the same time. Use with \ref LWGSM_EVT_CONN_ACTIVE or \ref LWGSM_EVT_CONN_CLOSE events */
        struct {
            lwgsm_conn_p conn;                  /*!< Set connection pointer */
        } conn_poll;                            /*!< Polling active connection to check for timeouts. Use with \ref LWGSM_EVT_CONN_POLL event */
#endif /* LWGSM_CFG_CONN || __DOXYGEN__ */

#if LWGSM_CFG_SMS || __DOXYGEN__
        struct {
            lwgsmr_t status;                    /*!< Enable status */
        } sms_enable;                           /*!< SMS enable event. Use with \ref LWGSM_EVT_SMS_ENABLE event */
        struct {
            size_t pos;                         /*!< Position in memory */
            lwgsmr_t res;                       /*!< SMS send result information */
        } sms_send;                             /*!< SMS sent info. Use with \ref LWGSM_EVT_SMS_SEND event */
        struct {
            lwgsm_mem_t mem;                    /*!< Memory of received message */
            size_t pos;                         /*!< Received position in memory for sent SMS */
        } sms_recv;                             /*!< SMS received info. Use with \ref LWGSM_EVT_SMS_RECV event */
        struct {
            lwgsm_sms_entry_t* entry;           /*!< SMS entry */
            lwgsmr_t res;                       /*!< SMS read result information */
        } sms_read;                             /*!< SMS read. Use with \ref LWGSM_EVT_SMS_READ event */
        struct {
            lwgsm_mem_t mem;                    /*!< Memory of deleted message */
            size_t pos;                         /*!< Deleted position in memory for sent SMS */
            lwgsmr_t res;                       /*!< Operation success */
        } sms_delete;                           /*!< SMS delete. Use with \ref LWGSM_EVT_SMS_DELETE event */
        struct {
            lwgsm_mem_t mem;                    /*!< Memory used for scan */
            lwgsm_sms_entry_t* entries;         /*!< Pointer to entries */
            size_t size;                        /*!< Number of valid entries */
            lwgsmr_t res;                       /*!< Result on command */
        } sms_list;                             /*!< SMS list. Use with \ref LWGSM_EVT_SMS_LIST event */
#endif /* LWGSM_CFG_SMS || __DOXYGEN__ */
#if LWGSM_CFG_CALL || __DOXYGEN__
        struct {
            lwgsmr_t res;                       /*!< Enable status */
        } call_enable;                          /*!< Call enable event. Use with \ref LWGSM_EVT_CALL_ENABLE event */
        struct {
            const lwgsm_call_t* call;           /*!< Call information */
        } call_changed;                         /*!< Call changed info. Use with \ref LWGSM_EVT_CALL_CHANGED event */
#endif /* LWGSM_CFG_CALL || __DOXYGEN__ */
#if LWGSM_CFG_PHONEBOOK || __DOXYGEN__
        struct {
            lwgsmr_t res;                       /*!< Enable status */
        } pb_enable;                            /*!< Phonebook enable event. Use with \ref LWGSM_EVT_PB_ENABLE event */
        struct {
            lwgsm_mem_t mem;                    /*!< Memory used for scan */
            lwgsm_pb_entry_t* entries;          /*!< Pointer to entries */
            size_t size;                        /*!< Number of valid entries */
            lwgsmr_t res;                       /*!< Operation success */
        } pb_list;                              /*!< Phonebok list. Use with \ref LWGSM_EVT_PB_LIST event */
        struct {
            const char* search;                 /*!< Search string */
            lwgsm_mem_t mem;                    /*!< Memory used for scan */
            lwgsm_pb_entry_t* entries;          /*!< Pointer to entries */
            size_t size;                        /*!< Number of valid entries */
            lwgsmr_t res;                       /*!< Operation success */
        } pb_search;                            /*!< Phonebok search list. Use with \ref LWGSM_EVT_PB_SEARCH event */
#endif /* LWGSM_CFG_PHONEBOOK || __DOXYGEN__ */
    } evt;                                      /*!< Callback event union */
} lwgsm_evt_t;

#define LWGSM_SIZET_MAX                         ((size_t)(-1))  /*!< Maximal value of size_t variable type */

/**
 * \ingroup         LWGSM_LL
 * \brief           Function prototype for AT output data
 * \param[in]       data: Pointer to data to send. This parameter can be set to `NULL`
 * \param[in]       len: Number of bytes to send. This parameter can be set to `0`
 *                      to indicate that internal buffer can be flushed to stream.
 *                      This is implementation defined and feature might be ignored
 * \return          Number of bytes sent
 */
typedef size_t  (*lwgsm_ll_send_fn)(const void* data, size_t len);

/**
 * \ingroup         LWGSM_LL
 * \brief           Function prototype for hardware reset of GSM device
 * \param[in]       state: State indicating reset. When set to `1`, reset must be active (usually pin active low),
 *                      or set to `0` when reset is cleared
 * \return          `1` on successful action, `0` otherwise
 */
typedef uint8_t (*lwgsm_ll_reset_fn)(uint8_t state);

/**
 * \ingroup         LWGSM_LL
 * \brief           Low level user specific functions
 */
typedef struct {
    lwgsm_ll_send_fn send_fn;                   /*!< Callback function to transmit data */
    lwgsm_ll_reset_fn reset_fn;                 /*!< Reset callback function */
    struct {
        uint32_t baudrate;                      /*!< UART baudrate value */
    } uart;                                     /*!< UART communication parameters */
} lwgsm_ll_t;

/**
 * \ingroup         LWGSM_TIMEOUT
 * \brief           Timeout callback function prototype
 * \param[in]       arg: Custom user argument
 */
typedef void (*lwgsm_timeout_fn)(void* arg);

/**
 * \ingroup         LWGSM_TIMEOUT
 * \brief           Timeout structure
 */
typedef struct lwgsm_timeout {
    struct lwgsm_timeout* next;                 /*!< Pointer to next timeout entry */
    uint32_t time;                              /*!< Time difference from previous entry */
    void* arg;                                  /*!< Argument to pass to callback function */
    lwgsm_timeout_fn fn;                        /*!< Callback function for timeout */
} lwgsm_timeout_t;

/**
 * \ingroup         LWGSM_BUFF
 * \brief           Buffer structure
 */
typedef struct {
    uint8_t* buff;                              /*!< Pointer to buffer data.
                                                        Buffer is considered initialized when `buff != NULL` */
    size_t size;                                /*!< Size of buffer data.
                                                        Size of actual buffer is `1` byte less than this value */
    size_t r;                                   /*!< Next read pointer.
                                                        Buffer is considered empty when `r == w` and full when `w == r - 1` */
    size_t w;                                   /*!< Next write pointer.
                                                        Buffer is considered empty when `r == w` and full when `w == r - 1` */
} lwgsm_buff_t;

/**
 * \ingroup         LWGSM_TYPEDEFS
 * \brief           Linear buffer structure
 */
typedef struct {
    uint8_t* buff;                              /*!< Pointer to buffer data array */
    size_t len;                                 /*!< Length of buffer array */
    size_t ptr;                                 /*!< Current buffer pointer */
} lwgsm_linbuff_t;

/**
 * \ingroup         LWGSM_TYPEDEFS
 * \brief           Function declaration for API function command event callback function
 * \param[in]       res: Operation result, member of \ref lwgsmr_t enumeration
 * \param[in]       arg: Custom user argument
 */
typedef void (*lwgsm_api_cmd_evt_fn)(lwgsmr_t res, void* arg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWGSM_HDR_DEFS_H */
