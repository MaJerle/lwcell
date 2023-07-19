/**
 * \file            lwcell_types.h
 * \brief           List of structures and enumerations for public usage
 */

/*
 * Copyright (c) 2023 Tilen MAJERLE
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
 * This file is part of LwCELL - Lightweight GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.1
 */
#ifndef LWCELL_TYPES_HDR_H
#define LWCELL_TYPES_HDR_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "lwcell/lwcell_opt.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \ingroup         LWCELL
 * \defgroup        LWCELL_TYPES Structures and enumerations
 * \brief           List of core structures and enumerations
 * \{
 */

/**
 * \}
 */

/**
 * \ingroup         LWCELL_TYPES
 * \brief           Result enumeration used across application functions
 */
typedef enum {
    lwcellOK = 0,              /*!< Function returned OK */
    lwcellOKIGNOREMORE,        /*!< Function succedded, should continue as \ref lwcellOK
                                                        but ignore sending more data.
                                                        This result is possible on connection data receive callback */
    lwcellERR,                 /*!< Generic error */
    lwcellERRPAR,              /*!< Wrong parameters on function call */
    lwcellERRMEM,              /*!< Memory error occurred */
    lwcellTIMEOUT,             /*!< Timeout occurred on command */
    lwcellCONT,                /*!< There is still some command to be processed in current command */
    lwcellCLOSED,              /*!< Connection just closed */
    lwcellINPROG,              /*!< Operation is in progress */

    lwcellERRNOTENABLED,       /*!< Feature not enabled error */
    lwcellERRNOIP,             /*!< Station does not have IP address */
    lwcellERRNOFREECONN,       /*!< There is no free connection available to start */
    lwcellERRCONNTIMEOUT,      /*!< Timeout received when connection to access point */
    lwcellERRPASS,             /*!< Invalid password for access point */
    lwcellERRNOAP,             /*!< No access point found with specific SSID and MAC address */
    lwcellERRCONNFAIL,         /*!< Connection failed to access point */
    lwcellERRWIFINOTCONNECTED, /*!< Wifi not connected to access point */
    lwcellERRNODEVICE,         /*!< Device is not present */
    lwcellERRBLOCKING,         /*!< Blocking mode command is not allowed */
} lwcellr_t;

/**
 * \ingroup         LWCELL_TYPES
 * \brief           GSM device model type
 */
typedef enum {

#define LWCELL_DEVICE_MODEL_ENTRY(name, str_id, is_2g, is_lte) LWCELL_DEVICE_MODEL_##name,
#include "lwcell/lwcell_models.h"
    LWCELL_DEVICE_MODEL_END,     /*!< End of device model */
    LWCELL_DEVICE_MODEL_UNKNOWN, /*!< Unknown device model */
} lwcell_device_model_t;

/**
 * \ingroup         LWCELL_SIM
 * \brief           SIM state
 */
typedef enum {
    LWCELL_SIM_STATE_NOT_INSERTED, /*!< SIM is not inserted in socket */
    LWCELL_SIM_STATE_READY,        /*!< SIM is ready for operations */
    LWCELL_SIM_STATE_NOT_READY,    /*!< SIM is not ready for any operation */
    LWCELL_SIM_STATE_PIN,          /*!< SIM is waiting for SIM to be given */
    LWCELL_SIM_STATE_PUK,          /*!< SIM is waiting for PUT to be given */
    LWCELL_SIM_STATE_PH_PIN,
    LWCELL_SIM_STATE_PH_PUK,
    LWCELL_SIM_STATE_END,
} lwcell_sim_state_t;

/**
 * \ingroup         LWCELL_TYPES
 * \brief           IP structure
 */
typedef struct {
    uint8_t ip[4]; /*!< IPv4 address */
} lwcell_ip_t;

/**
 * \ingroup         LWCELL_TYPES
 * \brief           Port variable
 */
typedef uint16_t lwcell_port_t;

/**
 * \ingroup         LWCELL_TYPES
 * \brief           MAC address
 */
typedef struct {
    uint8_t mac[6]; /*!< MAC address */
} lwcell_mac_t;

/**
 * \ingroup         LWCELL_CONN
 * \brief           List of possible connection types
 */
typedef enum {
    LWCELL_CONN_TYPE_TCP, /*!< Connection type is TCP */
    LWCELL_CONN_TYPE_UDP, /*!< Connection type is UDP */
    LWCELL_CONN_TYPE_SSL, /*!< Connection type is TCP over SSL */
} lwcell_conn_type_t;

/**
 * \ingroup         LWCELL_TYPES
 * \brief           Available device memories
 */
typedef enum {

#define LWCELL_DEV_MEMORY_ENTRY(name, str_code) LWCELL_MEM_##name,
#include "lwcell/lwcell_memories.h"
    LWCELL_MEM_END,            /*!< End of memory list */
    LWCELL_MEM_CURRENT,        /*!< Use current memory for read/delete operation */
    LWCELL_MEM_UNKNOWN = 0x1F, /*!< Unknown memory */
} lwcell_mem_t;

/**
 * \ingroup         LWCELL_TYPES
 * \brief           GSM number type
 */
typedef enum {
    LWCELL_NUMBER_TYPE_UNKNOWN = 129,       /*!< Number is unknown */
    LWCELL_NUMBER_TYPE_INTERNATIONAL = 145, /*!< Number is international */
    LWCELL_NUMBER_TYPE_NATIONAL = 161,      /*!< Number is national */
    LWCELL_NUMBER_TYPE_NETWORK = 177,       /*!< Number is network specific */
} lwcell_number_type_t;

/**
 * \ingroup         LWCELL_SMS
 * \brief           SMS status in current memory
 */
typedef enum {
    LWCELL_SMS_STATUS_ALL,    /*!< Process all SMS, used for mass delete or SMS list */
    LWCELL_SMS_STATUS_READ,   /*!< SMS status is read */
    LWCELL_SMS_STATUS_UNREAD, /*!< SMS status is unread */
    LWCELL_SMS_STATUS_SENT,   /*!< SMS status is sent */
    LWCELL_SMS_STATUS_UNSENT, /*!< SMS status is unsent */
    LWCELL_SMS_STATUS_INBOX,  /*!< SMS status, used only for mass delete operation */
} lwcell_sms_status_t;

/**
 * \ingroup         LWCELL_SMS
 * \brief           SMS entry structure
 */
typedef struct {
    lwcell_mem_t mem;           /*!< Memory storage */
    size_t pos;                /*!< Memory position */
    struct tm dt;              /*!< Date and time */
    lwcell_sms_status_t status; /*!< Message status */
    char number[26];           /*!< Phone number */
    char name[20];             /*!< Name in phonebook if exists */
    char data[161];            /*!< Data memory */
    size_t length;             /*!< Length of SMS data */
} lwcell_sms_entry_t;

/**
 * \ingroup         LWCELL_PB
 * \brief           Phonebook entry structure
 */
typedef struct {
    lwcell_mem_t mem;          /*!< Memory position */
    size_t pos;               /*!< Position in memory */
    char name[20];            /*!< Name of phonebook entry */
    char number[26];          /*!< Phone number */
    lwcell_number_type_t type; /*!< Phone number type */
} lwcell_pb_entry_t;

/**
 * \ingroup         LWCELL_OPERATOR
 * \brief           Operator status value
 */
typedef enum {
    LWCELL_OPERATOR_STATUS_UNKNOWN = 0x00, /*!< Unknown operator */
    LWCELL_OPERATOR_STATUS_AVAILABLE,      /*!< Operator is available */
    LWCELL_OPERATOR_STATUS_CURRENT,        /*!< Operator is currently active */
    LWCELL_OPERATOR_STATUS_FORBIDDEN       /*!< Operator is forbidden */
} lwcell_operator_status_t;

/**
 * \ingroup         LWCELL_OPERATOR
 * \brief           Operator selection mode
 */
typedef enum {
    LWCELL_OPERATOR_MODE_AUTO = 0x00,        /*!< Operator automatic mode */
    LWCELL_OPERATOR_MODE_MANUAL = 0x01,      /*!< Operator manual mode */
    LWCELL_OPERATOR_MODE_DEREGISTER = 0x02,  /*!< Operator deregistered from network */
    LWCELL_OPERATOR_MODE_MANUAL_AUTO = 0x04, /*!< Operator manual mode first. If fails, auto mode enabled */
} lwcell_operator_mode_t;

/**
 * \ingroup         LWCELL_OPERATOR
 * \brief           Operator data format
 */
typedef enum {
    LWCELL_OPERATOR_FORMAT_LONG_NAME = 0x00, /*!< COPS command returned long name */
    LWCELL_OPERATOR_FORMAT_SHORT_NAME,       /*!< COPS command returned short name */
    LWCELL_OPERATOR_FORMAT_NUMBER,           /*!< COPS command returned number */
    LWCELL_OPERATOR_FORMAT_INVALID           /*!< Unknown format */
} lwcell_operator_format_t;

/**
 * \ingroup         LWCELL_OPERATOR
 * \brief           Operator details for scan
 */
typedef struct {
    lwcell_operator_status_t stat; /*!< Operator status */
    char long_name[20];           /*!< Operator long name */
    char short_name[20];          /*!< Operator short name */
    uint32_t num;                 /*!< Operator numeric value */
} lwcell_operator_t;

/**
 * \ingroup         LWCELL_OPERATOR
 * \brief           Current operator info
 */
typedef struct {
    lwcell_operator_mode_t mode;     /*!< Operator mode */
    lwcell_operator_format_t format; /*!< Data format */

    union {
        char long_name[20];  /*!< Long name format */
        char short_name[20]; /*!< Short name format */
        uint32_t num;        /*!< Number format */
    } data;                  /*!< Operator data union */
} lwcell_operator_curr_t;

/**
 * \ingroup         LWCELL_NETWORK
 * \brief           Network Registration status
 */
typedef enum {
    LWCELL_NETWORK_REG_STATUS_SIM_ERR = 0x00,            /*!< SIM card error */
    LWCELL_NETWORK_REG_STATUS_CONNECTED = 0x01,          /*!< Device is connected to network */
    LWCELL_NETWORK_REG_STATUS_SEARCHING = 0x02,          /*!< Network search is in progress */
    LWCELL_NETWORK_REG_STATUS_DENIED = 0x03,             /*!< Registration denied */
    LWCELL_NETWORK_REG_STATUS_CONNECTED_ROAMING = 0x05,  /*!< Device is connected and is roaming */
    LWCELL_NETWORK_REG_STATUS_CONNECTED_SMS_ONLY = 0x06, /*!< Device is connected to home network in SMS-only mode */
    LWCELL_NETWORK_REG_STATUS_CONNECTED_ROAMING_SMS_ONLY = 0x07 /*!< Device is roaming in SMS-only mode */
} lwcell_network_reg_status_t;

/**
 * \ingroup         LWCELL_CALL
 * \brief           List of call directions
 */
typedef enum {
    LWCELL_CALL_DIR_MO = 0x00, /*!< Mobile Originated, outgoing call */
    LWCELL_CALL_DIR_MT,        /*!< Mobile Terminated, incoming call */
} lwcell_call_dir_t;

/**
 * \ingroup         LWCELL_CALL
 * \brief           List of call states
 */
typedef enum {
    LWCELL_CALL_STATE_ACTIVE = 0x00, /*!< Call is active */
    LWCELL_CALL_STATE_HELD,          /*!< Call is held */
    LWCELL_CALL_STATE_DIALING,       /*!< Call is dialing */
    LWCELL_CALL_STATE_ALERTING,      /*!< Call is alerting */
    LWCELL_CALL_STATE_INCOMING,      /*!< Call is incoming */
    LWCELL_CALL_STATE_WAITING,       /*!< Call is waiting */
    LWCELL_CALL_STATE_DISCONNECT,    /*!< Call disconnected, call finished */
} lwcell_call_state_t;

/**
 * \ingroup         LWCELL_CALL
 * \brief           List of call types
 */
typedef enum {
    LWCELL_CALL_TYPE_VOICE = 0x00, /*!< Voice call */
    LWCELL_CALL_TYPE_DATA,         /*!< Data call */
    LWCELL_CALL_TYPE_FAX,          /*!< Fax call */
} lwcell_call_type_t;

/**
 * \ingroup         LWCELL_CALL
 * \brief           Call information
 * \note            Data received on `+CLCC` info
 */
typedef struct {
    uint8_t ready;            /*!< Flag indicating feature ready by device */
    uint8_t enabled;          /*!< Flag indicating feature enabled */

    uint8_t id;               /*!< Call identification number, 0-7 */
    lwcell_call_dir_t dir;     /*!< Call direction */
    lwcell_call_state_t state; /*!< Call state */
    lwcell_call_type_t type;   /*!< Call type */
    char number[20];          /*!< Phone number */
    char is_multipart;        /*!< Multipart status */
    uint8_t addr_type;        /*!< Address type */
    char name[20];            /*!< Phone book name if exists for current number */
} lwcell_call_t;

/* Forward declarations */
struct lwcell_evt;
struct lwcell_conn;
struct lwcell_pbuf;

/**
 * \ingroup         LWCELL_CONN
 * \brief           Pointer to \ref lwcell_conn_t structure
 */
typedef struct lwcell_conn* lwcell_conn_p;

/**
 * \ingroup         LWCELL_PBUF
 * \brief           Pointer to \ref lwcell_pbuf_t structure
 */
typedef struct lwcell_pbuf* lwcell_pbuf_p;

/**
 * \ingroup         LWCELL_EVT
 * \brief           Event function prototype
 * \param[in]       evt: Callback event data
 * \return          \ref lwcellOK on success, member of \ref lwcellr_t otherwise
 */
typedef lwcellr_t (*lwcell_evt_fn)(struct lwcell_evt* evt);

/**
 * \ingroup         LWCELL_EVT
 * \brief           List of possible callback types received to user
 */
typedef enum lwcell_cb_type_t {
    LWCELL_EVT_INIT_FINISH,              /*!< Initialization has been finished at this point */

    LWCELL_EVT_RESET,                    /*!< Device reset operation finished */
    LWCELL_EVT_RESTORE,                  /*!< Device restore operation finished */

    LWCELL_EVT_CMD_TIMEOUT,              /*!< Timeout on command.
                                                        When application receives this event,
                                                        it may reset system as there was (maybe) a problem in device */

    LWCELL_EVT_DEVICE_PRESENT,           /*!< Notification when device present status changes */
    LWCELL_EVT_DEVICE_IDENTIFIED,        /*!< Device identified event */

    LWCELL_EVT_KEEP_ALIVE,               /*!< Generic keep-alive event type, used as periodic timeout.
                                                    Optionally enabled with \ref LWCELL_CFG_KEEP_ALIVE */

    LWCELL_EVT_SIGNAL_STRENGTH,          /*!< Signal strength event */

    LWCELL_EVT_SIM_STATE_CHANGED,        /*!< SIM card state changed */

    LWCELL_EVT_OPERATOR_SCAN,            /*!< Operator scan finished event */

    LWCELL_EVT_NETWORK_OPERATOR_CURRENT, /*!< Current operator event */
    LWCELL_EVT_NETWORK_REG_CHANGED,      /*!< Network registration changed.
                                                         Available even when \ref LWCELL_CFG_NETWORK is disabled */
#if LWCELL_CFG_NETWORK || __DOXYGEN__
    LWCELL_EVT_NETWORK_ATTACHED,         /*!< Attached to network, PDP context active and ready for TCP/IP application */
    LWCELL_EVT_NETWORK_DETACHED,         /*!< Detached from network, PDP context not active anymore */
#endif                                  /* LWCELL_CFG_NETWORK || __DOXYGEN__ */

#if LWCELL_CFG_CONN || __DOXYGEN__
    LWCELL_EVT_CONN_RECV,   /*!< Connection data received */
    LWCELL_EVT_CONN_SEND,   /*!< Connection data send */
    LWCELL_EVT_CONN_ACTIVE, /*!< Connection just became active */
    LWCELL_EVT_CONN_ERROR,  /*!< Client connection start was not successful */
    LWCELL_EVT_CONN_CLOSE,  /*!< Connection close event. Check status if successful */
    LWCELL_EVT_CONN_POLL,   /*!< Poll for connection if there are any changes */
#endif                     /* LWCELL_CFG_CONN || __DOXYGEN__ */

#if LWCELL_CFG_SMS || __DOXYGEN__
    LWCELL_EVT_SMS_ENABLE,      /*!< SMS enable event */
    LWCELL_EVT_SMS_READY,       /*!< SMS ready event */
    LWCELL_EVT_SMS_SEND,        /*!< SMS send event */
    LWCELL_EVT_SMS_RECV,        /*!< SMS received */
    LWCELL_EVT_SMS_READ,        /*!< SMS read */
    LWCELL_EVT_SMS_DELETE,      /*!< SMS delete */
    LWCELL_EVT_SMS_LIST,        /*!< SMS list */
#endif                         /* LWCELL_CFG_SMS || __DOXYGEN__ */
#if LWCELL_CFG_CALL || __DOXYGEN__
    LWCELL_EVT_CALL_ENABLE,     /*!< Call enable event */
    LWCELL_EVT_CALL_READY,      /*!< Call ready event */
    LWCELL_EVT_CALL_CHANGED,    /*!< Call info changed, `+CLCK` statement received */
    LWCELL_EVT_CALL_RING,       /*!< Call is ringing event */
    LWCELL_EVT_CALL_BUSY,       /*!< Call is busy */
    LWCELL_EVT_CALL_NO_CARRIER, /*!< No carrier to make a call */
#endif                         /* LWCELL_CFG_CALL || __DOXYGEN__ */
#if LWCELL_CFG_PHONEBOOK || __DOXYGEN__
    LWCELL_EVT_PB_ENABLE,       /*!< Phonebook enable event */
    LWCELL_EVT_PB_LIST,         /*!< Phonebook list event */
    LWCELL_EVT_PB_SEARCH,       /*!< Phonebook search event */
#endif                         /* LWCELL_CFG_PHONEBOOK || __DOXYGEN__ */
} lwcell_evt_type_t;

/**
 * \ingroup         LWCELL_EVT
 * \brief           Global callback structure to pass as parameter to callback function
 */
typedef struct lwcell_evt {
    lwcell_evt_type_t type; /*!< Callback type */

    union {
        struct {
            lwcellr_t res; /*!< Reset operation result */
        } reset;          /*!< Reset sequence finish. Use with \ref LWCELL_EVT_RESET event */

        struct {
            lwcellr_t res; /*!< Restore operation result */
        } restore;        /*!< Restore sequence finish. Use with \ref LWCELL_EVT_RESTORE event */

        struct {
            lwcell_sim_state_t state; /*!< SIM state */
        } cpin;                      /*!< CPIN event */

        struct {
            const lwcell_operator_curr_t* operator_current; /*!< Current operator info */
        } operator_current; /*!< Current operator event. Use with \ref LWCELL_EVT_NETWORK_OPERATOR_CURRENT event */

        struct {
            lwcell_operator_t* ops; /*!< Pointer to operators */
            size_t opf;            /*!< Number of operators found */
            lwcellr_t res;          /*!< Scan operation result */
        } operator_scan;           /*!< Operator scan event. Use with \ref LWCELL_EVT_OPERATOR_SCAN event */

        struct {
            int16_t rssi; /*!< Strength in units of dBm */
        } rssi;           /*!< Signal strength event. Use with \ref LWCELL_EVT_SIGNAL_STRENGTH event */

#if LWCELL_CFG_CONN || __DOXYGEN__
        struct {
            lwcell_conn_p conn; /*!< Connection where data were received */
            lwcell_pbuf_p buff; /*!< Pointer to received data */
        } conn_data_recv;      /*!< Network data received. Use with \ref LWCELL_EVT_CONN_RECV event */

        struct {
            lwcell_conn_p conn; /*!< Connection where data were sent */
            size_t sent;       /*!< Number of bytes sent on connection */
            lwcellr_t res;      /*!< Send data result */
        } conn_data_send;      /*!< Data successfully sent. Use with \ref LWCELL_EVT_CONN_SEND event */

        struct {
            const char* host;       /*!< Host to use for connection */
            lwcell_port_t port;      /*!< Remote port used for connection */
            lwcell_conn_type_t type; /*!< Connection type */
            void* arg;              /*!< Connection argument used on connection */
            lwcellr_t err;           /*!< Error value */
        } conn_error;               /*!< Client connection start error. Use with \ref LWCELL_EVT_CONN_ERROR event */

        struct {
            lwcell_conn_p conn; /*!< Pointer to connection */
            uint8_t client;    /*!< Set to `1` if connection is/was client mode */
            uint8_t forced;    /*!< Set to `1` if connection action was forced */
            lwcellr_t res;      /*!< Result of close event. Set to \ref lwcellOK on success. */
        } conn_active_close; /*!< Process active and closed statuses at the same time. Use with \ref LWCELL_EVT_CONN_ACTIVE or \ref LWCELL_EVT_CONN_CLOSE events */

        struct {
            lwcell_conn_p conn; /*!< Set connection pointer */
        } conn_poll; /*!< Polling active connection to check for timeouts. Use with \ref LWCELL_EVT_CONN_POLL event */
#endif               /* LWCELL_CFG_CONN || __DOXYGEN__ */

#if LWCELL_CFG_SMS || __DOXYGEN__
        struct {
            lwcellr_t status; /*!< Enable status */
        } sms_enable;        /*!< SMS enable event. Use with \ref LWCELL_EVT_SMS_ENABLE event */

        struct {
            size_t pos;   /*!< Position in memory */
            lwcellr_t res; /*!< SMS send result information */
        } sms_send;       /*!< SMS sent info. Use with \ref LWCELL_EVT_SMS_SEND event */

        struct {
            lwcell_mem_t mem; /*!< Memory of received message */
            size_t pos;      /*!< Received position in memory for sent SMS */
        } sms_recv;          /*!< SMS received info. Use with \ref LWCELL_EVT_SMS_RECV event */

        struct {
            lwcell_sms_entry_t* entry; /*!< SMS entry */
            lwcellr_t res;             /*!< SMS read result information */
        } sms_read;                   /*!< SMS read. Use with \ref LWCELL_EVT_SMS_READ event */

        struct {
            lwcell_mem_t mem; /*!< Memory of deleted message */
            size_t pos;      /*!< Deleted position in memory for sent SMS */
            lwcellr_t res;    /*!< Operation success */
        } sms_delete;        /*!< SMS delete. Use with \ref LWCELL_EVT_SMS_DELETE event */

        struct {
            lwcell_mem_t mem;            /*!< Memory used for scan */
            lwcell_sms_entry_t* entries; /*!< Pointer to entries */
            size_t size;                /*!< Number of valid entries */
            lwcellr_t res;               /*!< Result on command */
        } sms_list;                     /*!< SMS list. Use with \ref LWCELL_EVT_SMS_LIST event */
#endif                                  /* LWCELL_CFG_SMS || __DOXYGEN__ */
#if LWCELL_CFG_CALL || __DOXYGEN__
        struct {
            lwcellr_t res; /*!< Enable status */
        } call_enable;    /*!< Call enable event. Use with \ref LWCELL_EVT_CALL_ENABLE event */

        struct {
            const lwcell_call_t* call; /*!< Call information */
        } call_changed;               /*!< Call changed info. Use with \ref LWCELL_EVT_CALL_CHANGED event */
#endif                                /* LWCELL_CFG_CALL || __DOXYGEN__ */
#if LWCELL_CFG_PHONEBOOK || __DOXYGEN__
        struct {
            lwcellr_t res; /*!< Enable status */
        } pb_enable;      /*!< Phonebook enable event. Use with \ref LWCELL_EVT_PB_ENABLE event */

        struct {
            lwcell_mem_t mem;           /*!< Memory used for scan */
            lwcell_pb_entry_t* entries; /*!< Pointer to entries */
            size_t size;               /*!< Number of valid entries */
            lwcellr_t res;              /*!< Operation success */
        } pb_list;                     /*!< Phonebok list. Use with \ref LWCELL_EVT_PB_LIST event */

        struct {
            const char* search;        /*!< Search string */
            lwcell_mem_t mem;           /*!< Memory used for scan */
            lwcell_pb_entry_t* entries; /*!< Pointer to entries */
            size_t size;               /*!< Number of valid entries */
            lwcellr_t res;              /*!< Operation success */
        } pb_search;                   /*!< Phonebok search list. Use with \ref LWCELL_EVT_PB_SEARCH event */
#endif                                 /* LWCELL_CFG_PHONEBOOK || __DOXYGEN__ */
    } evt;                             /*!< Callback event union */
} lwcell_evt_t;

#define LWCELL_SIZET_MAX ((size_t)(-1)) /*!< Maximal value of size_t variable type */

/**
 * \ingroup         LWCELL_LL
 * \brief           Function prototype for AT output data
 * \param[in]       data: Pointer to data to send. This parameter can be set to `NULL`,
 *                      indicating to the low-level that (if used) DMA could be started
 *                      to transmit data to the device
 * \param[in]       len: Number of bytes to send. This parameter can be set to `0`
 *                      to indicate that internal buffer can be flushed to stream.
 *                      This is implementation defined and feature might be ignored
 * \return          Number of bytes sent
 */
typedef size_t (*lwcell_ll_send_fn)(const void* data, size_t len);

/**
 * \ingroup         LWCELL_LL
 * \brief           Function prototype for hardware reset of GSM device
 * \param[in]       state: State indicating reset. When set to `1`, reset must be active (usually pin active low),
 *                      or set to `0` when reset is cleared
 * \return          `1` on successful action, `0` otherwise
 */
typedef uint8_t (*lwcell_ll_reset_fn)(uint8_t state);

/**
 * \ingroup         LWCELL_LL
 * \brief           Low level user specific functions
 */
typedef struct {
    lwcell_ll_send_fn send_fn;   /*!< Callback function to transmit data */
    lwcell_ll_reset_fn reset_fn; /*!< Reset callback function */

    struct {
        uint32_t baudrate; /*!< UART baudrate value */
    } uart;                /*!< UART communication parameters */
} lwcell_ll_t;

/**
 * \ingroup         LWCELL_TIMEOUT
 * \brief           Timeout callback function prototype
 * \param[in]       arg: Custom user argument
 */
typedef void (*lwcell_timeout_fn)(void* arg);

/**
 * \ingroup         LWCELL_TIMEOUT
 * \brief           Timeout structure
 */
typedef struct lwcell_timeout {
    struct lwcell_timeout* next; /*!< Pointer to next timeout entry */
    uint32_t time;              /*!< Time difference from previous entry */
    void* arg;                  /*!< Argument to pass to callback function */
    lwcell_timeout_fn fn;        /*!< Callback function for timeout */
} lwcell_timeout_t;

/**
 * \ingroup         LWCELL_BUFF
 * \brief           Buffer structure
 */
typedef struct {
    uint8_t* buff; /*!< Pointer to buffer data.
                                                        Buffer is considered initialized when `buff != NULL` */
    size_t size;   /*!< Size of buffer data.
                                                        Size of actual buffer is `1` byte less than this value */
    size_t r;      /*!< Next read pointer.
                                                        Buffer is considered empty when `r == w` and full when `w == r - 1` */
    size_t w;      /*!< Next write pointer.
                                                        Buffer is considered empty when `r == w` and full when `w == r - 1` */
} lwcell_buff_t;

/**
 * \ingroup         LWCELL_TYPES
 * \brief           Linear buffer structure
 */
typedef struct {
    uint8_t* buff; /*!< Pointer to buffer data array */
    size_t len;    /*!< Length of buffer array */
    size_t ptr;    /*!< Current buffer pointer */
} lwcell_linbuff_t;

/**
 * \ingroup         LWCELL_TYPES
 * \brief           Function declaration for API function command event callback function
 * \param[in]       res: Operation result, member of \ref lwcellr_t enumeration
 * \param[in]       arg: Custom user argument
 */
typedef void (*lwcell_api_cmd_evt_fn)(lwcellr_t res, void* arg);

/**
 * \ingroup         LWCELL_UNICODE
 * \brief           Unicode support structure
 */
typedef struct {
    uint8_t ch[4]; /*!< UTF-8 max characters */
    uint8_t t;     /*!< Total expected length in UTF-8 sequence */
    uint8_t r;     /*!< Remaining bytes in UTF-8 sequence */
    lwcellr_t res;  /*!< Current result of processing */
} lwcell_unicode_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWCELL_TYPES_HDR_H */
