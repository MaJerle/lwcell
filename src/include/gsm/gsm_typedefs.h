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
 * This file is part of GSM-AT.
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
    gsmNOFREECONN,                              /*!< There is no free connection available to start */
    gsmCONT,                                    /*!< There is still some command to be processed in current command */
    gsmCLOSED,                                  /*!< Connection just closed */
    gsmINPROG,                                  /*!< Operation is in progress */
    
    gsmERRCONNTIMEOUT,                          /*!< Timeout received when connection to access point */
    gsmERRPASS,                                 /*!< Invalid password for access point */
    gsmERRNOAP,                                 /*!< No access point found with specific SSID and MAC address */
    gsmERRCONNFAIL,                             /*!< Connection failed to access point */
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
    uint8_t date;                               /*!< Day in a month, from 1 to up to 31 */
    uint8_t month;                              /*!< Month in a year, from 1 to 12 */
    uint16_t year;                              /*!< Year */
    uint8_t day;                                /*!< Day in a week, from 1 to 7 */
    uint8_t hours;                              /*!< Hours in a day, from 0 to 23 */
    uint8_t minutes;                            /*!< Minutes in a hour, from 0 to 59 */
    uint8_t seconds;                            /*!< Seconds in a minute, from 0 to 59 */
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
* \brief           Available memories
*/
typedef enum {
    GSM_MEM_UNKNOWN,
    GSM_MEM_SM,
    GSM_MEM_ME,
    GSM_MEM_MT,
    GSM_MEM_BM,
    GSM_MEM_SR,
} gsm_mem_t;

/**
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
 * \brief           Operator status value
 */
typedef enum {
    GSM_OPERATOR_STATUS_UNKNOWN = 0x00,         /*!< Unknown operator */
    GSM_OPERATOR_STATUS_AVAILABLE,              /*!< Operator is available */
    GSM_OPERATOR_STATUS_CURRENT,                /*!< Operator is currently active */
    GSM_OPERATOR_STATUS_FORBIDDEN               /*!< Operator is forbidden */
} gsm_operator_status_t;

/**
* \brief           SMS entry structure
*/
typedef struct {
    gsm_datetime_t datetime;                    /*!< Date and time */
    size_t pos;                                 /*!< Memory position */
    gsm_sms_status_t status;                    /*!< Message status */
    char number[26];                            /*!< Phone number */
    char data[161];                             /*!< Data memory */
    size_t length;                              /*!< Length of SMS data */
} gsm_sms_entry_t;

/**
 * \brief           Operator details for scan
 */
typedef struct {
    gsm_operator_status_t stat;                 /*!< Operator status */
    char long_name[20];                         /*!< Operator long name */
    char short_name[20];                        /*!< Operator short name */
    uint32_t num;                               /*!< Operator numeric value */
} gsm_operator_t;

/* Forward declarations */
struct gsm_cb_t;
struct gsm_conn_t;
struct gsm_pbuf_t;

/**
 * \ingroup         GSM_CONN
 * \brief           Pointer to \ref gsm_conn_t structure
 */
typedef struct gsm_conn_t* gsm_conn_p;

/**
 * \ingroup         GSM_PBUF
 * \brief           Pointer to \ref gsm_pbuf_t structure
 */
typedef struct gsm_pbuf_t* gsm_pbuf_p;

/**
 * \ingroup         GSM_EVT
 * \brief           Event function prototype
 * \param[in]       cb: Callback event data
 * \return          gsmOK on success, member of \ref gsmr_t otherwise
 */
typedef gsmr_t  (*gsm_cb_fn)(struct gsm_cb_t* cb);

/**
 * \ingroup         GSM_EVT
 * \brief           List of possible callback types received to user
 */
typedef enum gsm_cb_type_t {
    GSM_CB_RESET,                               /*!< Device reset detected */
    
    GSM_CB_INIT_FINISH,                         /*!< Initialization has been finished at this point */
    
    GSM_CB_CONN_DATA_RECV,                      /*!< Connection data received */
    GSM_CB_CONN_DATA_SENT,                      /*!< Data were successfully sent */
    GSM_CB_CONN_DATA_SEND_ERR,                  /*!< Error trying to send data */
    GSM_CB_CONN_ACTIVE,                         /*!< Connection just became active */
    GSM_CB_CONN_ERROR,                          /*!< Client connection start was not successful */
    GSM_CB_CONN_CLOSED,                         /*!< Connection was just closed */
    GSM_CB_CONN_POLL,                           /*!< Poll for connection if there are any changes */

    GSM_CB_CPIN,                                /*!< SIM event */
#if GSM_CFG_SMS || __DOXYGEN__
    GSM_CB_SMS_READY,                           /*!< SMS ready event */
    GSM_CB_SMS_SENT,                            /*!< SMS sent successfully */
    GSM_CB_SMS_SEND_ERROR,                      /*!< SMS sent successfully */
    GSM_CB_SMS_RECV,                            /*!< SMS received */
#endif /* GSM_CFG_SMS || __DOXYGEN__ */
#if GSM_CFG_CALL || __DOXYGEN__
    GSM_CB_CALL_READY,                          /*!< Call ready event */
#endif /* GSM_CFG_CALL || __DOXYGEN__ */
} gsm_cb_type_t;

/**
 * \ingroup         GSM_EVT
 * \brief           Global callback structure to pass as parameter to callback function
 */
typedef struct gsm_cb_t {
    gsm_cb_type_t type;                         /*!< Callback type */
    union {
        struct {
            gsm_sim_state_t state;              /*!< SIM state */
        } cpin;                                 /*!< CPIN event */
#if GSM_CFG_SMS
        struct {
            uint16_t num;                       /*!< Received number in memory for sent SMS*/
        } sms_sent;                             /*!< SMS sent info. Use with \ref GSM_CB_SMS_SENT event */
        struct {
            gsm_mem_t mem;                      /*!< SMS memory */
            uint16_t num;                       /*!< Received number in memory for sent SMS*/
        } sms_recv;                             /*!< SMS received info. Use with \ref GSM_CB_SMS_RECV event */
#endif
        struct {
            gsm_conn_p conn;                    /*!< Connection where data were received */
            gsm_pbuf_p buff;                    /*!< Pointer to received data */
        } conn_data_recv;                       /*!< Network data received. Use with \ref GSM_CB_CONN_DATA_RECV event */
        struct {
            gsm_conn_p conn;                    /*!< Connection where data were sent */
            size_t sent;                        /*!< Number of bytes sent on connection */
        } conn_data_sent;                       /*!< Data successfully sent. Use with \ref GSM_CB_CONN_DATA_SENT event */
        struct {
            gsm_conn_p conn;                    /*!< Connection where data were sent */
            size_t sent;                        /*!< Number of bytes sent on connection before error occurred */
        } conn_data_send_err;                   /*!< Data were not sent. Use with \ref GSM_CB_CONN_DATA_SEND_ERR event */
        struct {
            const char* host;                   /*!< Host to use for connection */
            gsm_port_t port;                    /*!< Remote port used for connection */
            gsm_conn_type_t type;               /*!< Connection type */
            void* arg;                          /*!< Connection argument used on connection */
        } conn_error;                           /*!< Client connection start error */

        struct {
            uint8_t forced;                     /*!< Set to 1 if reset forced by user */
        } reset;                                /*!< Reset occurred */
    } cb;                                       /*!< Callback event union */
} gsm_cb_t;

#define GSM_SIZET_MAX                           ((size_t)(-1))  /*!< Maximal value of size_t variable type */
 
/**
 * \ingroup         GSM_LL
 * \brief           Function prototype for AT output data
 * \param[in]       data: Pointer to data to send
 * \param[in]       len: Number of bytes to send
 * \return          Number of bytes sent
 */
typedef uint16_t (*gsm_ll_send_fn)(const void* data, uint16_t len);

/**
 * \ingroup         GSM_LL
 * \brief           Low level user specific functions
 */
typedef struct {
    gsm_ll_send_fn send_fn;                     /*!< Callback function to transmit data */
} gsm_ll_t;

/**
 * \ingroup         GSM_TIMEOUT
 * \brief           Timeout callback function prototype
 */
typedef void (*gsm_timeout_fn_t)(void *);

/**
 * \ingroup         GSM_TIMEOUT
 * \brief           Timeout structure
 */
typedef struct gsm_timeout {
    struct gsm_timeout* next;                   /*!< Pointer to next timeout entry */
    uint32_t time;                              /*!< Time difference from previous entry */
    void* arg;                                  /*!< Argument to pass to callback function */
    gsm_timeout_fn_t fn;                        /*!< Callback function for timeout */
} gsm_timeout_t;

/**
 * \ingroup         GSM_BUFF
 * \brief           Buffer structure
 */
typedef struct gsm_buff {
    size_t size;                                /*!< Size of buffer in units of bytes */
    size_t in;                                  /*!< Input pointer to save next value */
    size_t out;                                 /*!< Output pointer to read next value */
    uint8_t* buff;                              /*!< Pointer to buffer data array */
    uint8_t flags;                              /*!< Flags for buffer */
} gsm_buff_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GSM_DEFS_H */
