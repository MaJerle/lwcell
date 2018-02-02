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
 * \ingroup         GSM_TYPEDEFS
 * \brief           List of encryptions of access point
 */
typedef enum {
    GSM_ECN_OPEN = 0x00,                        /*!< No encryption on access point */
    GSM_ECN_WEP,                                /*!< WEP (Wired Equivalent Privacy) encryption */
    GSM_ECN_WPA_PSK,                            /*!< WPA (Wifi Protected Access) encryption */
    GSM_ECN_WPA2_PSK,                           /*!< WPA2 (Wifi Protected Access 2) encryption */
    GSM_ECN_WPA_WPA2_PSK,                       /*!< WPA/2 (Wifi Protected Access 1/2) encryption */
    GSM_ECN_WPA2_Enterprise                     /*!< Enterprise encryption. \note GSM is currently not able to connect to access point of this encryption type */
} gsm_ecn_t;

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
 * \ingroup         GSM_AP
 * \brief           Access point data structure
 */
typedef struct {
    gsm_ecn_t ecn;                              /*!< Encryption mode */
    char ssid[21];                              /*!< Access point name */
    int16_t rssi;                               /*!< Received signal strength indicator */
    gsm_mac_t mac;                              /*!< MAC physical address */
    uint8_t ch;                                 /*!< WiFi channel used on access point */
    int8_t offset;                              /*!< Access point offset */
    uint8_t cal;                                /*!< Calibration value */
} gsm_ap_t;

/**
 * \ingroup         GSM_STA
 * \brief           Station data structure
 */
typedef struct {
    gsm_ip_t ip;                                /*!< IP address of connected station */
    gsm_mac_t mac;                              /*!< MAC address of connected station */
} gsm_sta_t;

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
 * \ingroup         GSM_TYPEDEFS
 * \brief           List of possible WiFi modes
 */
typedef enum {
#if GSM_CFG_MODE_STATION || __DOXYGEN__
    GSM_MODE_STA = 1,                           /*!< Set WiFi mode to station only */
#endif /* GSM_CFG_MODE_STATION || __DOXYGEN__ */
#if GSM_CFG_MODE_ACCESS_POINT || __DOXYGEN__
    GSM_MODE_AP = 2,                            /*!< Set WiFi mode to access point only */
#endif /* GSM_CFG_MODE_ACCESS_POINT || __DOXYGEN__ */
#if (GSM_CFG_MODE_STATION_ACCESS_POINT) || __DOXYGEN__
    GSM_MODE_STA_AP = 3,                        /*!< Set WiFi mode to station and access point */
#endif /* (GSM_CFG_MODE_STATION_ACCESS_POINT) || __DOXYGEN__ */
} gsm_mode_t;

/**
 * \ingroup         GSM_CONN
 * \brief           List of possible connection types
 */
typedef enum {
    GSM_CONN_TYPE_TCP,                          /*!< Connection type is TCP */
    GSM_CONN_TYPE_UDP,                          /*!< Connection type is UDP */
    GSM_CONN_TYPE_SSL,                          /*!< Connection type is SSL */
} gsm_conn_type_t;

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
    
#if GSM_CFG_MODE_STATION || __DOXYGEN__
    GSM_CB_WIFI_CONNECTED,                      /*!< Station just connected to AP */
    GSM_CB_WIFI_GOT_IP,                         /*!< Station has valid IP */
    GSM_CB_WIFI_DISCONNECTED,                   /*!< Station just disconnected from AP */
  
    GSM_CB_STA_LIST_AP,                         /*!< Station listed APs event */
#endif /* GSM_CFG_MODE_STATION || __DOXYGEN__ */
#if GSM_CFG_MODE_ACCESS_POINT || __DOXYGEN__
    GSM_CB_AP_CONNECTED_STA,                    /*!< New station just connected to GSM's access point */
    GSM_CB_AP_DISCONNECTED_STA,                 /*!< New station just disconnected from GSM's access point */
    GSM_CB_AP_IP_STA,                           /*!< New station just received IP from GSM's access point */
#endif /* GSM_CFG_MODE_ACCESS_POINT || __DOXYGEN__ */
} gsm_cb_type_t;

/**
 * \ingroup         GSM_EVT
 * \brief           Global callback structure to pass as parameter to callback function
 */
typedef struct gsm_cb_t {
    gsm_cb_type_t type;                         /*!< Callback type */
    union {
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
            /**
             * \todo: Implement error reason:
             *  - No free connection to start
             *  - Remote host is not rgsmonding
             *  - ...
             */
        } conn_error;                           /*!< Client connection start error */
        struct {
            gsm_conn_p conn;                    /*!< Pointer to connection */
            uint8_t client;                     /*!< Set to 1 if connection is/was client mode */
            uint8_t forced;                     /*!< Set to 1 if connection action was forced (when active, 1 = CLIENT, 0 = SERVER; when closed, 1 = CMD, 0 = REMOTE) */
        } conn_active_closed;                   /*!< Process active and closed statuses at the same time. Use with \ref GSM_CB_CONN_ACTIVE or \ref GSM_CB_CONN_CLOSED events */
        struct {
            gsm_conn_p conn;                    /*!< Set connection pointer */
        } conn_poll;                            /*!< Polling active connection to check for timeouts. Use with \ref GSM_CB_CONN_POLL event */
        struct {
            gsmr_t status;                      /*!< Status of command */
            gsm_ap_t* aps;                      /*!< Pointer to access points */
            size_t len;                         /*!< Number of access points found */
        } sta_list_ap;
        
#if GSM_CFG_MODE_ACCESS_POINT
        struct {
            gsm_mac_t* mac;                     /*!< Station MAC address */
        } ap_conn_disconn_sta;                  /*!< A new station connected or disconnected to GSM's access point. Use with \ref GSM_CB_AP_CONNECTED_STA or \ref GSM_CB_AP_DISCONNECTED_STA events */
        struct {
            gsm_mac_t* mac;                     /*!< Station MAC address */
            gsm_ip_t* ip;                       /*!< Station IP address */
        } ap_ip_sta;                            /*!< Station got IP address from GSM's access point. Use with \ref GSM_CB_AP_IP_STA */
#endif /* GSM_CFG_MODE_ACCESS_POINT */

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
typedef struct gsm_timeout_t {
    struct gsm_timeout_t* next;                 /*!< Pointer to next timeout entry */
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
