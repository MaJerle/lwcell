/**
 * \file            gsm_typedefs.h
 * \brief           List of structures and enumerations for public usage
 */

/*
 * Copyright (c) 2018 Tilen Majerle
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
 * This file is part of GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 */
#ifndef __GSM_DEFS_H
#define __GSM_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

/**
 * \ingroup         GSM
 * \defgroup        GSM_TYPEDEFS Structures and enumerations
 * \brief           List of core structures and enumerations
 * \{
 */
    
/**
 * \}
 */

/**
 * \ingroup         GSM_TYPEDEFS
 * \brief           Result enumeration used across application functions
 */
typedef enum {
    gsmOK = 0,                                  /*!< Function returned OK */
    gsmOKIGNOREMORE,                            /*!< Function succedded, should continue as gsmOK but ignore sending more data. This result is possible on connection data receive callback */
    gsmERR,
    gsmPARERR,                                  /*!< Wrong parameters on function call */
    gsmERRMEM,                                  /*!< Memory error occurred */
    gsmTIMEOUT,                                 /*!< Timeout occurred on command */
    gsmCONT,                                    /*!< There is still some command to be processed in current command */
    gsmCLOSED,                                  /*!< Connection just closed */
    gsmINPROG,                                  /*!< Operation is in progress */
    
    gsmERRNOTENABLED,                           /*!< Feature not enabled error */
    gsmERRNOIP,                                 /*!< Station does not have IP address */
    gsmERRNOFREECONN,                           /*!< There is no free connection available to start */
    gsmERRCONNTIMEOUT,                          /*!< Timeout received when connection to access point */
    gsmERRPASS,                                 /*!< Invalid password for access point */
    gsmERRNOAP,                                 /*!< No access point found with specific SSID and MAC address */
    gsmERRCONNFAIL,                             /*!< Connection failed to access point */
    gsmERRWIFINOTCONNECTED,                     /*!< Wifi not connected to access point */
    gsmERRNODEVICE,                             /*!< Device is not present */
} gsmr_t;

/**
 * \brief           SIM state
 */
typedef enum {
    GSM_SIM_STATE_NOT_INSERTED,                 /*!< SIM is not inserted in socket */
    GSM_SIM_STATE_READY,                        /*!< SIM is ready for operations */
    GSM_SIM_STATE_NOT_READY,                    /*!< SIM is not ready for any operation */
    GSM_SIM_STATE_PIN,                          /*!< SIM is waiting for SIM to be given */
    GSM_SIM_STATE_PUK,                          /*!< SIM is waiting for PUT to be given */
    GSM_SIM_STATE_PH_PIN,
    GSM_SIM_STATE_PH_PUK,
} gsm_sim_state_t;

/**
 * \ingroup         GSM_TYPEDEFS
 * \brief           IP structure
 */
typedef struct {
    uint8_t ip[4];                              /*!< IPv4 address */
} gsm_ip_t;

/**
 * \ingroup         GSM_TYPEDEFS
 * \brief           Port variable
 */
typedef uint16_t    gsm_port_t;

/**
 * \ingroup         GSM_TYPEDEFS
 * \brief           MAC address
 */
typedef struct {
    uint8_t mac[6];                             /*!< MAC address */
} gsm_mac_t;

/**
 * \ingroup         GSM_TYPEDEFS
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
} gsm_datetime_t;

/**
 * \ingroup         GSM_CONN
 * \brief           List of possible connection types
 */
typedef enum {
    GSM_CONN_TYPE_TCP,                          /*!< Connection type is TCP */
    GSM_CONN_TYPE_UDP,                          /*!< Connection type is UDP */
} gsm_conn_type_t;

/**
 * \brief           Available device memories
 */
typedef enum {
#define GSM_DEV_MEMORY_ENTRY(name, str_code)    GSM_MEM_ ## name,
#include "gsm/gsm_memories.h"
    GSM_MEM_END,                                /*!< End of memory list */
    GSM_MEM_CURRENT,                            /*!< Use current memory for read/delete operation */
    GSM_MEM_UNKNOWN = 0x1F,                     /*!< Unknown memory */
} gsm_mem_t;

/**
 * \brief           GSM number type
 */
typedef enum {
    GSM_NUMBER_TYPE_NATIONAL = 129,             /*!< Number is national */
    GSM_NUMBER_TYPE_INTERNATIONAL = 145,        /*!< Number is international */
} gsm_number_type_t;

/**
 * \ingroup         GSM_SMS
 * \brief           SMS status in current memory
 */
typedef enum {
    GSM_SMS_STATUS_ALL,                         /*!< Process all SMS, used for mass delete or SMS list */
    GSM_SMS_STATUS_READ,                        /*!< SMS status is read */
    GSM_SMS_STATUS_UNREAD,                      /*!< SMS status is unread */
    GSM_SMS_STATUS_SENT,                        /*!< SMS status is sent */
    GSM_SMS_STATUS_UNSENT,                      /*!< SMS status is unsent */
} gsm_sms_status_t;

/**
 * \ingroup         GSM_SMS
 * \brief           SMS entry structure
 */
typedef struct {
    gsm_mem_t mem;                              /*!< Memory storage */
    size_t pos;                                 /*!< Memory position */
    gsm_datetime_t datetime;                    /*!< Date and time */
    gsm_sms_status_t status;                    /*!< Message status */
    char number[26];                            /*!< Phone number */
    char name[20];                              /*!< Name in phonebook if exists */
    char data[161];                             /*!< Data memory */
    size_t length;                              /*!< Length of SMS data */
} gsm_sms_entry_t;

/**
 * \ingroup         GSM_PB
 * \brief           Phonebook entry structure
 */
typedef struct {
    gsm_mem_t mem;                              /*!< Memory position */
    size_t pos;                                 /*!< Position in memory */
    char name[20];                              /*!< Name of phonebook entry */
    char number[26];                            /*!< Phone number */
    gsm_number_type_t type;                     /*!< Phone number type */
} gsm_pb_entry_t;

/**
 * \ingroup         GSM_OPERATOR
 * \brief           Operator status value
 */
typedef enum {
    GSM_OPERATOR_STATUS_UNKNOWN = 0x00,         /*!< Unknown operator */
    GSM_OPERATOR_STATUS_AVAILABLE,              /*!< Operator is available */
    GSM_OPERATOR_STATUS_CURRENT,                /*!< Operator is currently active */
    GSM_OPERATOR_STATUS_FORBIDDEN               /*!< Operator is forbidden */
} gsm_operator_status_t;

/**
 * \ingroup         GSM_OPERATOR
 * \brief           Operator selection mode
 */
typedef enum {
    GSM_OPERATOR_MODE_AUTO = 0x00,              /*!< Operator automatic mode */
    GSM_OPERATOR_MODE_MANUAL = 0x01,            /*!< Operator manual mode */
    GSM_OPERATOR_MODE_DEREGISTER = 0x02,        /*!< Operator deregistered from network */
    GSM_OPERATOR_MODE_MANUAL_AUTO = 0x04,       /*!< Operator manual mode first. If fails, auto mode enabled */
} gsm_operator_mode_t;

/**
 * \ingroup         GSM_OPERATOR
 * \brief           Operator data format
 */
typedef enum {
    GSM_OPERATOR_FORMAT_LONG_NAME = 0x00,       /*!< COPS command returned long name */
    GSM_OPERATOR_FORMAT_SHORT_NAME,             /*!< COPS command returned short name */
    GSM_OPERATOR_FORMAT_NUMBER,                 /*!< COPS command returned number */
    GSM_OPERATOR_FORMAT_INVALID                 /*!< Unknown format */
} gsm_operator_format_t;

/**
 * \ingroup         GSM_OPERATOR
 * \brief           Operator details for scan
 */
typedef struct {
    gsm_operator_status_t stat;                 /*!< Operator status */
    char long_name[20];                         /*!< Operator long name */
    char short_name[20];                        /*!< Operator short name */
    uint32_t num;                               /*!< Operator numeric value */
} gsm_operator_t;

/**
 * \ingroup         GSM_OPERATOR
 * \brief           Current operator info
 */
typedef struct {
    gsm_operator_mode_t mode;                   /*!< Operator mode */
    gsm_operator_format_t format;               /*!< Data format */
    union {
        char long_name[20];                     /*!< Long name format */
        char short_name[20];                    /*!< Short name format */
        uint32_t num;                           /*!< Number format */
    } data;                                     /*!< Operator data union */
} gsm_operator_curr_t;

/**
 * \brief           Network Registration status
 */
typedef enum {
    GSM_NETWORK_REG_STATUS_SIM_ERR = 0x00,      /*!< SIM card error */
    GSM_NETWORK_REG_STATUS_CONNECTED = 0x01,    /*!< Device is connected to network */
    GSM_NETWORK_REG_STATUS_SEARCHING = 0x02,    /*!< Network search is in progress */
    GSM_NETWORK_REG_STATUS_DENIED = 0x03,       /*!< Registration denied */
    GSM_NETWORK_REG_STATUS_CONNECTED_ROAMING = 0x05 /*!< Device is connected and is roaming */
} gsm_network_reg_status_t;

/**
 * \ingroup         GSM_CALL
 * \brief           List of call directions
 */
typedef enum {
    GSM_CALL_DIR_MO = 0x00,                     /*!< Mobile Originated, outgoing call */
    GSM_CALL_DIR_MT,                            /*!< Mobile Terminated, incoming call */
} gsm_call_dir_t;

/**
 * \ingroup         GSM_CALL
 * \brief           List of call states
 */
typedef enum {
    GSM_CALL_STATE_ACTIVE = 0x00,               /*!< Call is active */
    GSM_CALL_STATE_HELD,                        /*!< Call is held */
    GSM_CALL_STATE_DIALING,                     /*!< Call is dialing */
    GSM_CALL_STATE_ALERGING,                    /*!< Call is alerting */
    GSM_CALL_STATE_INCOMING,                    /*!< Call is incoming */
    GSM_CALL_STATE_WAITING,                     /*!< Call is waiting */
    GSM_CALL_STATE_DISCONNECT,                  /*!< Call disconnected, call finished */
} gsm_call_state_t;

/**
 * \ingroup         GSM_CALL
 * \brief           List of call types
 */
typedef enum {
    GSM_CALL_TYPE_VOICE = 0x00,                 /*!< Voice call */
    GSM_CALL_TYPE_DATA,                         /*!< Data call */
    GSM_CALL_TYPE_FAX,                          /*!< Fax call */
} gsm_call_type_t;

/**
 * \ingroup         GSM_CALL
 * \brief           Call information
 * \note            Data received on `+CLCC` info
 */
typedef struct {
    uint8_t ready;                              /*!< Flag indicating feature ready by device */
    uint8_t enabled;                            /*!< Flag indicating feature enabled */

    uint8_t id;                                 /*!< Call identification number, 0-7 */
    gsm_call_dir_t dir;                         /*!< Call direction */
    gsm_call_state_t state;                     /*!< Call state */
    gsm_call_type_t type;                       /*!< Call type */
    char number[20];                            /*!< Phone number */
    char is_multipart;                          /*!< Multipart status */
    uint8_t addr_type;                          /*!< Address type */
    char name[20];                              /*!< Phone book name if exists for current number */
} gsm_call_t;

/* Forward declarations */
struct gsm_evt;
struct gsm_conn;
struct gsm_pbuf;

/**
 * \ingroup         GSM_CONN
 * \brief           Pointer to \ref gsm_conn_t structure
 */
typedef struct gsm_conn* gsm_conn_p;

/**
 * \ingroup         GSM_PBUF
 * \brief           Pointer to \ref gsm_pbuf_t structure
 */
typedef struct gsm_pbuf* gsm_pbuf_p;

/**
 * \ingroup         GSM_EVT
 * \brief           Event function prototype
 * \param[in]       evt: Callback event data
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
typedef gsmr_t  (*gsm_evt_fn)(struct gsm_evt* evt);

/**
 * \ingroup         GSM_EVT
 * \brief           List of possible callback types received to user
 */
typedef enum gsm_cb_type_t {
    GSM_EVT_RESET,                              /*!< Device reset detected */
    GSM_EVT_RESET_FINISH,                       /*!< Reset operation finished */

    GSM_EVT_DEVICE_PRESENT,                     /*!< Notification when device present status changes */
    GSM_EVT_DEVICE_IDENTIFIED,                  /*!< Device identified event */

    GSM_EVT_INIT_FINISH,                        /*!< Initialization has been finished at this point */

    GSM_EVT_SIGNAL_STRENGTH,                    /*!< Signal strength event */

    GSM_EVT_NETWORK_OPERATOR_CURRENT,           /*!< Current operator event */
    GSM_EVT_NETWORK_REG,                        /*!< Network registration changed. Available even when \ref GSM_CFG_NETWORK is disabled */
#if GSM_CFG_NETWORK || __DOXYGEN__
    GSM_EVT_NETWORK_ATTACHED,                   /*!< Attached to network, PDP context active and ready for TCP/IP application */
    GSM_EVT_NETWORK_DETACHED,                   /*!< Detached from network, PDP context not active anymore */
#endif /* GSM_CFG_NETWORK || __DOXYGEN__ */

#if GSM_CFG_CONN || __DOXYGEN__
    GSM_EVT_CONN_DATA_RECV,                     /*!< Connection data received */
    GSM_EVT_CONN_DATA_SEND,                     /*!< Connection data send */
    GSM_EVT_CONN_ACTIVE,                        /*!< Connection just became active */
    GSM_EVT_CONN_ERROR,                         /*!< Client connection start was not successful */
    GSM_EVT_CONN_CLOSED,                        /*!< Connection was just closed */
    GSM_EVT_CONN_POLL,                          /*!< Poll for connection if there are any changes */
#endif /* GSM_CFG_CONN || __DOXYGEN__ */

    GSM_EVT_CPIN,                               /*!< SIM event */
#if GSM_CFG_SMS || __DOXYGEN__
    GSM_EVT_SMS_ENABLE,                         /*!< SMS enable event */
    GSM_EVT_SMS_READY,                          /*!< SMS ready event */
    GSM_EVT_SMS_SEND,                           /*!< SMS send event */
    GSM_EVT_SMS_RECV,                           /*!< SMS received */
    GSM_EVT_SMS_READ,                           /*!< SMS read */
    GSM_EVT_SMS_LIST,                           /*!< SMS list */
#endif /* GSM_CFG_SMS || __DOXYGEN__ */
#if GSM_CFG_CALL || __DOXYGEN__
    GSM_EVT_CALL_ENABLE,                        /*!< Call enable event */
    GSM_EVT_CALL_READY,                         /*!< Call ready event */
    GSM_EVT_CALL_CHANGED,                       /*!< Call info changed, `+CLCK` statement received */
    GSM_EVT_CALL_RING,                          /*!< Call is ringing event */
    GSM_EVT_CALL_BUSY,                          /*!< Call is busy */
    GSM_EVT_CALL_NO_CARRIER,                    /*!< No carrier to make a call */
#endif /* GSM_CFG_CALL || __DOXYGEN__ */
#if GSM_CFG_PHONEBOOK || __DOXYGEN__
    GSM_EVT_PB_ENABLE,                          /*!< Phonebook enable event */
    GSM_EVT_PB_LIST,                            /*!< Phonebook list event */
    GSM_EVT_PB_SEARCH,                          /*!< Phonebook search event */
#endif /* GSM_CFG_PHONEBOOK || __DOXYGEN__ */
} gsm_evt_type_t;

/**
 * \ingroup         GSM_EVT
 * \brief           Global callback structure to pass as parameter to callback function
 */
typedef struct gsm_evt {
    gsm_evt_type_t type;                        /*!< Callback type */
    union {
        struct {
            uint8_t forced;                     /*!< Set to `1` if reset forced by user */
        } reset;                                /*!< Reset occurred. Use with \ref GSM_EVT_RESET event */

        struct {
            gsm_sim_state_t state;              /*!< SIM state */
        } cpin;                                 /*!< CPIN event */
        struct {
            const gsm_operator_curr_t* operator_current;    /*!< Current operator info */
        } operator_current;                     /*!< Current operator event. Use with \ref GSM_EVT_NETWORK_OPERATOR_CURRENT event */

        struct {
            int16_t rssi;                       /*!< Strength in units of dBm */
        } rssi;                                 /*<! Signal strength event. Use with \ref GSM_EVT_SIGNAL_STRENGTH event */

#if GSM_CFG_CONN || __DOXYGEN__
        struct {
            gsm_conn_p conn;                    /*!< Connection where data were received */
            gsm_pbuf_p buff;                    /*!< Pointer to received data */
        } conn_data_recv;                       /*!< Network data received. Use with \ref GSM_EVT_CONN_DATA_RECV event */
        struct {
            gsm_conn_p conn;                    /*!< Connection where data were sent */
            size_t sent;                        /*!< Number of bytes sent on connection */
            gsmr_t res;                         /*!< Send data result */
        } conn_data_send;                       /*!< Data successfully sent. Use with \ref GSM_EVT_CONN_DATA_SEND event */
        struct {
            const char* host;                   /*!< Host to use for connection */
            gsm_port_t port;                    /*!< Remote port used for connection */
            gsm_conn_type_t type;               /*!< Connection type */
            void* arg;                          /*!< Connection argument used on connection */
            gsmr_t err;                         /*!< Error value */
        } conn_error;                           /*!< Client connection start error. Use with \ref GSM_EVT_CONN_ERROR event */
        struct {
            gsm_conn_p conn;                    /*!< Pointer to connection */
            uint8_t client;                     /*!< Set to 1 if connection is/was client mode */
            uint8_t forced;                     /*!< Set to 1 if connection action was forced (when active: 1 = CLIENT, 0 = SERVER: when closed, 1 = CMD, 0 = REMOTE) */
        } conn_active_closed;                   /*!< Process active and closed statuses at the same time. Use with \ref GSM_EVT_CONN_ACTIVE or \ref GSM_EVT_CONN_CLOSED events */
        struct {
            gsm_conn_p conn;                    /*!< Set connection pointer */
        } conn_poll;                            /*!< Polling active connection to check for timeouts. Use with \ref GSM_EVT_CONN_POLL event */
#endif /* GSM_CFG_CONN || __DOXYGEN__ */

#if GSM_CFG_SMS || __DOXYGEN__
        struct {
            gsmr_t status;                      /*!< Enable status */
        } sms_enable;                           /*!< SMS enable event. Use with \ref GSM_EVT_SMS_ENABLE event */
        struct {
            size_t num;                         /*!< Received number in memory for sent SMS */
            gsmr_t res;                         /*!< SMS send result information */
        } sms_send;                             /*!< SMS sent info. Use with \ref GSM_EVT_SMS_SEND event */
        struct {
            gsm_mem_t mem;                      /*!< Memory of received message */
            size_t pos;                         /*!< Received position in memory for sent SMS */
        } sms_recv;                             /*!< SMS received info. Use with \ref GSM_EVT_SMS_RECV event */
        struct {
            gsm_sms_entry_t* entry;             /*!< SMS entry */
        } sms_read;                             /*!< SMS read. Use with \ref GSM_EVT_SMS_READ event */
        struct {
            gsm_mem_t mem;                      /*!< Memory used for scan */
            gsm_sms_entry_t* entries;           /*!< Pointer to entries */
            size_t size;                        /*!< Number of valid entries */
            gsmr_t err;                         /*!< Error message if exists */
        } sms_list;                             /*!< SMS list. Use with \ref GSM_EVT_SMS_LIST event */
#endif /* GSM_CFG_SMS || __DOXYGEN__ */
#if GSM_CFG_CALL || __DOXYGEN__
        struct {
            gsmr_t status;                      /*!< Enable status */
        } call_enable;                          /*!< Call enable event. Use with \ref GSM_EVT_CALL_ENABLE event */
        struct {
            const gsm_call_t* call;             /*!< Call information */
        } call_changed;                         /*!< Call changed info. Use with \ref GSM_EVT_CALL_CHANGED event */
#endif /* GSM_CFG_CALL || __DOXYGEN__ */
#if GSM_CFG_PHONEBOOK || __DOXYGEN__
        struct {
            gsmr_t status;                      /*!< Enable status */
        } pb_enable;                            /*!< Phonebook enable event. Use with \ref GSM_EVT_PB_ENABLE event */
        struct {
            gsm_mem_t mem;                      /*!< Memory used for scan */
            gsm_pb_entry_t* entries;            /*!< Pointer to entries */
            size_t size;                        /*!< Number of valid entries */
            gsmr_t err;                         /*!< Error message if exists */
        } pb_list;                              /*!< Phonebok list. Use with \ref GSM_EVT_PB_LIST event */
        struct {
            const char* search;                 /*!< Search string */
            gsm_mem_t mem;                      /*!< Memory used for scan */
            gsm_pb_entry_t* entries;            /*!< Pointer to entries */
            size_t size;                        /*!< Number of valid entries */
            gsmr_t err;                         /*!< Error message if exists */
        } pb_search;                            /*!< Phonebok search list. Use with \ref GSM_EVT_PB_SEARCH event */
#endif /* GSM_CFG_PHONEBOOK || __DOXYGEN__ */
    } evt;                                      /*!< Callback event union */
} gsm_evt_t;

#define GSM_SIZET_MAX                           ((size_t)(-1))  /*!< Maximal value of size_t variable type */
 
/**
 * \ingroup         GSM_LL
 * \brief           Function prototype for AT output data
 * \param[in]       data: Pointer to data to send
 * \param[in]       len: Number of bytes to send
 * \return          Number of bytes sent
 */
typedef size_t  (*gsm_ll_send_fn)(const void* data, size_t len);

/**
 * \ingroup         GSM_LL
 * \brief           Low level user specific functions
 */
typedef struct {
    gsm_ll_send_fn send_fn;                     /*!< Callback function to transmit data */
    struct {
        uint32_t baudrate;                      /*!< UART baudrate value */
    } uart;                                     /*!< UART communication parameters */
} gsm_ll_t;

/**
 * \ingroup         GSM_TIMEOUT
 * \brief           Timeout callback function prototype
 * \param[in]       arg: Timeout argument
 */
typedef void (*gsm_timeout_fn)(void * arg);

/**
 * \ingroup         GSM_TIMEOUT
 * \brief           Timeout structure
 */
typedef struct gsm_timeout {
    struct gsm_timeout* next;                   /*!< Pointer to next timeout entry */
    uint32_t time;                              /*!< Time difference from previous entry */
    void* arg;                                  /*!< Argument to pass to callback function */
    gsm_timeout_fn fn;                          /*!< Callback function for timeout */
} gsm_timeout_t;

/**
 * \ingroup         GSM_BUFF
 * \brief           Buffer structure
 */
typedef struct {
    size_t size;                                /*!< Size of buffer in units of bytes */
    size_t in;                                  /*!< Input pointer to save next value */
    size_t out;                                 /*!< Output pointer to read next value */
    uint8_t* buff;                              /*!< Pointer to buffer data array */
    uint8_t flags;                              /*!< Flags for buffer */
} gsm_buff_t;

/**
 * \ingroup         GSM_TYPEDEFS
 * \brief           Linear buffer structure
 */
typedef struct {
    uint8_t* buff;                              /*!< Pointer to buffer data array */
    size_t len;                                 /*!< Length of buffer array */
    size_t ptr;                                 /*!< Current buffer pointer */
} gsm_linbuff_t;

#if !__DOXYGEN__

/**
 * \ingroup         GSM_PRIVATE
 * \brief           Receive character structure to handle full line terminated with `\n` character
 */
typedef struct {
    char data[128];                             /*!< Received characters */
    uint8_t len;                                /*!< Length of line */
} gsm_recv_t;

#endif /* !__DOXYGEN__ */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GSM_DEFS_H */
