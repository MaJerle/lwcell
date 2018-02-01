/**
 * \file            gsm_private.h
 * \brief           Private structures and enumerations
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
#ifndef __GSM_PRIV_H
#define __GSM_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "stdint.h"
#include "stdlib.h"
#include "string.h"

#if defined(GSM_INTERNAL) || __DOXYGEN__

#include "gsm/gsm.h"
#include "gsm/gsm_debug.h"

/**
 * \addtogroup      GSM_TYPEDEFS
 * \{
 */
 
/**
 * \brief           List of possible messages
 */
typedef enum {
    GSM_CMD_IDLE = 0,                           /*!< IDLE mode */
    
    /*
     * Basic AT commands
     */
    GSM_CMD_RESET,                              /*!< Reset device */
    GSM_CMD_ATE0,                               /*!< Disable ECHO mode on AT commands */
    GSM_CMD_ATE1,                               /*!< Enable ECHO mode on AT commands */
    GSM_CMD_GMR,                                /*!< Get AT commands version */
    GSM_CMD_GSLP,                               /*!< Set GSM to sleep mode */
    GSM_CMD_RESTORE,                            /*!< Restore GSM internal settings to default values */
    GSM_CMD_UART,
    GSM_CMD_SLEEP,
    GSM_CMD_WAKEUPGPIO,
    GSM_CMD_RFPOWER,
    GSM_CMD_RFVDD,
    GSM_CMD_RFAUTOTRACE,
    GSM_CMD_SYSRAM,
    GSM_CMD_SYSADC,
    GSM_CMD_SYSIOSETCFG,
    GSM_CMD_SYSIOGETCFG,
    GSM_CMD_SYSGPIODIR,
    GSM_CMD_SYSGPIOWRITE,
    GSM_CMD_SYSGPIOREAD,
    GSM_CMD_SYSMSG,                             /*!< Configure system messages */
    
    /*
     * WiFi based commands
     */
    GSM_CMD_WIFI_CWMODE,                        /*!< Set/Get wifi mode */
#if GSM_CFG_MODE_STATION || __DOXYGEN__
    GSM_CMD_WIFI_CWJAP,                         /*!< Connect to access point */
    GSM_CMD_WIFI_CWQAP,                         /*!< Disconnect from access point */
    GSM_CMD_WIFI_CWLAP,                         /*!< List available access points */
    GSM_CMD_WIFI_CIPSTAMAC_GET,                 /*!< Get MAC address of GSM station */
    GSM_CMD_WIFI_CIPSTAMAC_SET,                 /*!< Set MAC address of GSM station */
    GSM_CMD_WIFI_CIPSTA_GET,                    /*!< Get IP address of GSM station */
    GSM_CMD_WIFI_CIPSTA_SET,                    /*!< Set IP address of GSM station */
    GSM_CMD_WIFI_CWAUTOCONN,                    /*!< Configure auto connection to access point */
#endif /* GSM_CFG_MODE_STATION || __DOXYGEN__ */
#if GSM_CFG_MODE_ACCESS_POINT || __DOXYGEN__
    GSM_CMD_WIFI_CWSAP_GET,                     /*!< Get software access point configuration */
    GSM_CMD_WIFI_CWSAP_SET,                     /*!< Set software access point configuration */
    GSM_CMD_WIFI_CIPAPMAC_GET,                  /*!< Get MAC address of GSM access point */
    GSM_CMD_WIFI_CIPAPMAC_SET,                  /*!< Set MAC address of GSM access point */
    GSM_CMD_WIFI_CIPAP_GET,                     /*!< Get IP address of GSM access point */
    GSM_CMD_WIFI_CIPAP_SET,                     /*!< Set IP address of GSM access point */
    GSM_CMD_WIFI_CWLIF,                         /*!< Get connected stations on access point */
#endif /* GSM_CFG_MODE_STATION || __DOXYGEN__ */
    GSM_CMD_WIFI_WPS,                           /*!< Set WPS option */
    GSM_CMD_WIFI_MDNS,                          /*!< Configure MDNS function */
#if GSM_CFG_HOSTNAME || __DOXYGEN__
    GSM_CMD_WIFI_CWHOSTNAME_SET,                /*!< Set device hostname */
    GSM_CMD_WIFI_CWHOSTNAME_GET,                /*!< Get device hostname */
#endif /* GSM_CFG_HOSTNAME || __DOXYGEN__ */
    
    /*
     * TCP/IP related commands
     */
    GSM_CMD_TCPIP_CIPSTATUS,                    /*!< Get status of connections */
#if GSM_CFG_DNS || __DOXYGEN__
    GSM_CMD_TCPIP_CIPDOMAIN,                    /*!< Get IP address from domain name = DNS function */
#endif /* GSM_CFG_DNS || __DOXYGEN__ */
    GSM_CMD_TCPIP_CIPSTART,                     /*!< Start client connection */
    GSM_CMD_TCPIP_CIPSSLSIZE,                   /*!< Set SSL buffer size for SSL connection */
    GSM_CMD_TCPIP_CIPSEND,                      /*!< Send network data */
    GSM_CMD_TCPIP_CIPCLOSE,                     /*!< Close active connection */
    GSM_CMD_TCPIP_CIFSR,                        /*!< Get local IP */
    GSM_CMD_TCPIP_CIPMUX,                       /*!< Set single or multiple connections */
    GSM_CMD_TCPIP_CIPSERVER,                    /*!< Enables/Disables server mode */
    GSM_CMD_TCPIP_CIPSERVERMAXCONN,             /*!< Sets maximal number of connections allowed for server population */
    GSM_CMD_TCPIP_CIPMODE,                      /*!< Transmission mode, either transparent or normal one */
    GSM_CMD_TCPIP_CIPSTO,                       /*!< Sets connection timeout */
#if GSM_CFG_PING || __DOXYGEN__
    GSM_CMD_TCPIP_PING,                         /*!< Ping domain */
#endif /* GSM_CFG_PING || __DOXYGEN__ */
    GSM_CMD_TCPIP_CIUPDATE,                     /*!< Perform self-update */
#if GSM_CFG_SNTP || __DOXYGEN__
    GSM_CMD_TCPIP_CIPSNTPCFG,                   /*!< Configure SNTP servers */
    GSM_CMD_TCPIP_CIPSNTPTIME,                  /*!< Get current time using SNTP */
#endif /* GSM_SNT || __DOXYGEN__ */
    GSM_CMD_TCPIP_CIPDNS,                       /*!< Configure user specific DNS servers */
    GSM_CMD_TCPIP_CIPDINFO,                     /*!< Configure what data are received on +IPD statement */
} gsm_cmd_t;

/**
 * \brief           Connection structure
 */
typedef struct gsm_conn_t {
    gsm_conn_type_t type;                       /*!< Connection type */
    uint8_t         num;                        /*!< Connection number */
    gsm_ip_t        remote_ip;                  /*!< Remote IP address */
    gsm_port_t      remote_port;                /*!< Remote port number */
    gsm_port_t      local_port;                 /*!< Local IP address */
    gsm_cb_fn       cb_func;                    /*!< Callback function for connection */
    void*           arg;                        /*!< User custom argument */
    
    uint8_t         val_id;                     /*!< Validation ID number. It is increased each time a new connection is established.
                                                     It protects sending data to wrong connection in case we have data in send queue,
                                                     and connection was closed and active again in between. */
    
    uint8_t*        buff;                       /*!< Pointer to buffer when using \ref gsm_conn_write function */
    size_t          buff_len;                   /*!< Total length of buffer */
    size_t          buff_ptr;                   /*!< Current write pointer of buffer */
    
    union {
        struct {
            uint8_t active:1;                   /*!< Status whether connection is active */
            uint8_t client:1;                   /*!< Status whether connection is in client mode */
            uint8_t data_received:1;            /*!< Status whether first data were received on connection */
            uint8_t in_closing:1;               /*!< Status if connection is in closing mode.
                                                    When in closing mode, ignore any possible received data from function */
        } f;
    } status;                                   /*!< Connection status union with flag bits */
} gsm_conn_t;

/**
 * \brief           Packet buffer structure
 */
typedef struct gsm_pbuf_t {
    struct gsm_pbuf_t* next;                    /*!< Next pbuf in chain list */
    size_t tot_len;                             /*!< Total length of pbuf chain */
    size_t len;                                 /*!< Length of payload */
    size_t ref;                                 /*!< Number of references to this structure */
    uint8_t* payload;                           /*!< Pointer to payload memory */
    gsm_ip_t ip;                                /*!< Remote address for received IPD data */
    gsm_port_t port;                            /*!< Remote port for received IPD data */
} gsm_pbuf_t;

/**
 * \brief           Incoming network data read structure
 */
typedef struct {
    uint8_t             read;                   /*!< Set to 1 when we should process input data as connection data */
    size_t              tot_len;                /*!< Total length of packet */
    size_t              rem_len;                /*!< Remaining bytes to read in current +IPD statement */
    gsm_conn_p          conn;                   /*!< Pointer to connection for network data */
    gsm_ip_t            ip;                     /*!< Remote IP address on from IPD data */
    gsm_port_t          port;                   /*!< Remote port on IPD data */
    
    size_t              buff_ptr;               /*!< Buffer pointer to save data to */
    gsm_pbuf_p          buff;                   /*!< Pointer to data buffer used for receiving data */
} gsm_ipd_t;

/**
 * \brief           Message queue structure to share between threads
 */
typedef struct gsm_msg {
    gsm_cmd_t       cmd_def;                    /*!< Default message type received from queue */
    gsm_cmd_t       cmd;                        /*!< Since some commands can have different subcommands, sub command is used here */
    uint8_t         i;                          /*!< Variable to indicate order number of subcommands */
    gsm_sys_sem_t   sem;                        /*!< Semaphore for the message */
    uint8_t         is_blocking;                /*!< Status if command is blocking */
    uint32_t        block_time;                 /*!< Maximal blocking time in units of milliseconds. Use 0 to for non-blocking call */
    gsmr_t          res;                        /*!< Result of message operation */
    gsmr_t          (*fn)(struct gsm_msg *);    /*!< Processing callback function to process packet */
    union {
        struct {
            uint32_t baudrate;                  /*!< Baudrate for AT port */
        } uart;
        struct {
            gsm_mode_t mode;                    /*!< Mode of operation */                    
        } wifi_mode;                            /*!< When message type \ref GSM_CMD_WIFI_CWMODE is used */
        struct {
            const char* name;                   /*!< AP name */
            const char* pass;                   /*!< AP password */
            const gsm_mac_t* mac;               /*!< Specific MAC address to use when connecting to AP */
            uint8_t def;                        /*!< Value indicates to connect as current only or as default */
            uint8_t error_num;                  /*!< Error number on connecting */
        } sta_join;                             /*!< Message for joining to access point */
        struct {
            uint8_t en;                         /*!< Status to enable/disable auto join feature */
        } sta_autojoin;                         /*!< Message for auto join procedure */
        struct {
            gsm_ip_t* ip;                       /*!< Pointer to IP variable */
            gsm_ip_t* gw;                       /*!< Pointer to gateway variable */
            gsm_ip_t* nm;                       /*!< Pointer to netmask variable */
            uint8_t def;                        /*!< Value for receiving default or current settings */
        } sta_ap_getip;                         /*!< Message for reading station or access point IP */
        struct {
            gsm_mac_t* mac;                     /*!< Pointer to MAC variable */
            uint8_t def;                        /*!< Value for receiving default or current settings */
        } sta_ap_getmac;                        /*!< Message for reading station or access point MAC address */
        struct {
            const gsm_ip_t* ip;                 /*!< Pointer to IP variable */
            const gsm_ip_t* gw;                 /*!< Pointer to gateway variable */
            const gsm_ip_t* nm;                 /*!< Pointer to netmask variable */
            uint8_t def;                        /*!< Value for receiving default or current settings */
        } sta_ap_setip;                         /*!< Message for setting station or access point IP */
        struct {
            const gsm_mac_t* mac;               /*!< Pointer to MAC variable */
            uint8_t def;                        /*!< Value for receiving default or current settings */
        } sta_ap_setmac;                        /*!< Message for setting station or access point MAC address */
        struct {
            const char* ssid;                   /*!< Pointer to optional filter SSID name to search */
            gsm_ap_t* aps;                      /*!< Pointer to array to save access points */
            size_t apsl;                        /*!< Length of input array of access points */
            size_t apsi;                        /*!< Current access point array */
            size_t* apf;                        /*!< Pointer to output variable holding number of access points found */
        } ap_list;                              /*!< List for access points */
        struct {
            gsm_sta_t* stas;                    /*!< Pointer to array to save access points */
            size_t stal;                        /*!< Length of input array of access points */
            size_t stai;                        /*!< Current access point array */
            size_t* staf;                       /*!< Pointer to output variable holding number of access points found */
        } sta_list;                             /*!< List for stations */
        struct {
            const char* ssid;                   /*!< Name of access point */
            const char* pwd;                    /*!< Password of access point */
            gsm_ecn_t ecn;                      /*!< Ecryption used */
            uint8_t ch;                         /*!< RF Channel used */
            uint8_t max_sta;                    /*!< Max allowed connected stations */
            uint8_t hid;                        /*!< Configuration if network is hidden or visible */
            uint8_t def;                        /*!< Save as default configuration */
        } ap_conf;                              /*!< Parameters to configura access point */
        struct {
            char* hostname;                     /*!< Hostname set/get value */
            size_t length;                      /*!< Length of buffer when reading hostname */
        } wifi_hostname;                        /*!< Set or get hostname structure */
        
        /**
         * Connection based commands
         */
        struct {
            gsm_conn_t** conn;                  /*!< Pointer to pointer to save connection used */
            const char* host;                   /*!< Host to use for connection */
            gsm_port_t port;                    /*!< Remote port used for connection */
            gsm_conn_type_t type;               /*!< Connection type */
            void* arg;                          /*!< Connection custom argument */
            gsm_cb_fn cb_func;                  /*!< Callback function to use on connection */
            uint8_t num;                        /*!< Connection number used for start */
        } conn_start;                           /*!< Structure for starting new connection */
        struct {
            gsm_conn_t* conn;                   /*!< Pointer to connection to close */
            uint8_t val_id;                     /*!< Connection current validation ID when command was sent to queue */
        } conn_close;
        struct {
            gsm_conn_t* conn;                   /*!< Pointer to connection to send data */
            size_t btw;                         /*!< Number of remaining bytes to write */
            size_t ptr;                         /*!< Current write pointer for data */
            const uint8_t* data;                /*!< Data to send */
            size_t sent;                        /*!< Number of bytes sent in last packet */
            size_t sent_all;                    /*!< Number of bytes sent all together */
            uint8_t tries;                      /*!< Number of tries used for last packet */
            uint8_t wait_send_ok_err;           /*!< Set to 1 when we wait for SEND OK or SEND ERROR */
            const gsm_ip_t* remote_ip;           /*!< Remote IP address for UDP connection */
            gsm_port_t remote_port;               /*!< Remote port address for UDP connection */
            uint8_t fau;                        /*!< Free after use flag to free memory after data are sent (or not) */
            size_t* bw;                         /*!< Number of bytes written so far */
            uint8_t val_id;                     /*!< Connection current validation ID when command was sent to queue */
        } conn_send;                            /*!< Structure to send data on connection */
        
        /*
         * TCP/IP based commands
         */
        struct {
            uint8_t mux;                        /*!< Mux status, either enabled or disabled */
        } tcpip_mux;                            /*!< Used for setting up multiple connections */
        struct {
            gsm_port_t port;                    /*!< Server port number */
            uint16_t max_conn;                  /*!< Maximal number of connections available for server */
            uint16_t timeout;                   /*!< Connection timeout */
            gsm_cb_fn cb;                       /*!< Server default callback function */
        } tcpip_server;
        struct {
            uint8_t info;                       /*!< New info status */
        } tcpip_dinfo;                          /*!< Structure to enable more info on +IPD command */
        struct {
            const char* host;                   /*!< Hostname to ping */
            uint32_t* time;                     /*!< Pointer to time variable */
        } tcpip_ping;                           /*!< Pinging structure */
        struct {
            size_t size;                        /*!< Size for SSL in uints of bytes */
        } tcpip_sslsize;                        /*!< TCP SSL size for SSL connections */
        struct {
            const char* host;                   /*!< Hostname to resolve IP address for */
            gsm_ip_t* ip;                       /*!< Pointer to IP address to save result */
        } dns_getbyhostname;                    /*!< DNS function */
        struct {
            uint8_t en;                         /*!< Status if SNTP is enabled or not */
            int8_t tz;                          /*!< Timezone setup */
            const char* h1;                     /*!< Optional server 1 */
            const char* h2;                     /*!< Optional server 2 */
            const char* h3;                     /*!< Optional server 3 */
        } tcpip_sntp_cfg;                       /*!< SNTP configuration */
        struct {
            gsm_datetime_t* dt;                 /*!< Pointer to datetime structure */
        } tcpip_sntp_time;                      /*!< SNTP get time */
        struct {
            uint8_t en;                         /*!< Status if WPS is enabled or not */
        } wps_cfg;                              /*!< WPS configuration */
    } msg;                                      /*!< Group of different possible message contents */
} gsm_msg_t;

/**
 * \brief           IP and MAC structure with netmask and gateway addresses
 */
typedef struct {
    gsm_ip_t ip;                                /*!< IP address */
    gsm_ip_t gw;                                /*!< Gateway address */
    gsm_ip_t nm;                                /*!< Netmask address */
    gsm_mac_t mac;                              /*!< MAC address */
} gsm_ip_mac_t;

/**
 * \brief           Link connection active info
 */
typedef struct {
    uint8_t failed;                             /*!< Status if connection successful */
    uint8_t num;                                /*!< Connection number */
    uint8_t is_server;                          /*!< Status if connection is client or server */
    gsm_conn_type_t type;                       /*!< Connection type */
    gsm_ip_t remote_ip;                         /*!< Remote IP address */
    gsm_port_t remote_port;                     /*!< Remote port */
    gsm_port_t local_port;                      /*!< Local port number */
} gsm_link_conn_t;

/**
 * \brief           Callback function linked list prototype
 */
typedef struct gsm_cb_func {
    struct gsm_cb_func* next;                   /*!< Next function in the list */
    gsm_cb_fn fn;                               /*!< Function pointer itself */
} gsm_cb_func_t;

/**
 * \brief           GSM global structure
 */
typedef struct {    
    uint32_t version_at;                        /*!< Version of AT command software on GSM device */
    uint32_t version_sdk;                       /*!< Version of SDK used to build AT software */

    gsm_sys_sem_t       sem_sync;               /*!< Synchronization semaphore between threads */
    gsm_sys_mbox_t      mbox_producer;          /*!< Producer message queue handle */
    gsm_sys_mbox_t      mbox_process;           /*!< Consumer message queue handle */
    gsm_sys_thread_t    thread_producer;        /*!< Producer thread handle */
    gsm_sys_thread_t    thread_process;         /*!< Processing thread handle */
#if !GSM_CFG_INPUT_USE_PROCESS || __DOXYGEN__
    gsm_buff_t          buff;                   /*!< Input processing buffer */
#endif /* !GSM_CFG_INPUT_USE_PROCESS || __DOXYGEN__ */
    gsm_ll_t            ll;                     /*!< Low level functions */
    
    gsm_msg_t*          msg;                    /*!< Pointer to current user message being executed */
    
    uint32_t            active_conns;           /*!< Bit field of currently active connections, @todo: In case user has more than 32 connections, single variable is not enough */
    uint32_t            active_conns_last;      /*!< The same as previous but status before last check */
    
    gsm_conn_t          conns[GSM_CFG_MAX_CONNS];   /*!< Array of all connection structures */
    
    gsm_link_conn_t     link_conn;              /*!< Link connection handle */
    gsm_ipd_t           ipd;                    /*!< Incoming data structure */
    gsm_cb_t            cb;                     /*!< Callback processing structure */
    
    gsm_cb_func_t*      cb_func;                /*!< Callback function linked list */
    gsm_cb_fn           cb_server;              /*!< Default callback function for server connections */
    
#if GSM_CFG_MODE_STATION || __DOXYGEN__
    gsm_ip_mac_t        sta;                    /*!< Station IP and MAC addressed */
#endif /* GSM_CFG_MODE_STATION || __DOXYGEN__ */
#if GSM_CFG_MODE_ACCESS_POINT || __DOXYGEN__
    gsm_ip_mac_t        ap;                     /*!< Access point IP and MAC addressed */
#endif /* GSM_CFG_MODE_ACCESS_POINT || __DOXYGEN__ */
    
    union {
        struct {
            uint8_t     initialized:1;          /*!< Flag indicating GSM library is initialized */
            uint8_t     r_got_ip:1;             /*!< Flag indicating GSM has IP */
            uint8_t     r_w_conn:1;             /*!< Flag indicating GSM is connected to wifi */
        } f;                                    /*!< Flags structure */
    } status;                                   /*!< Status structure */
    
    uint8_t conn_val_id;                        /*!< Validation ID increased each time device connects to wifi network or on reset.
                                                    it is used for connections */
} gsm_t;

/**
 * \brief           Unicode support structure
 */
typedef struct gsm_unicode_t {
    uint8_t ch[4];                              /*!< UTF-8 max characters */
    uint8_t t;                                  /*!< Total expected length in UTF-8 sequence */
    uint8_t r;                                  /*!< Remaining bytes in UTF-8 sequence */
    gsmr_t res;                                 /*!< Current result of processing */
} gsm_unicode_t;

/**
 * \}
 */
 
/**
 * \addtogroup      GSM
 * \{
 */

/**
 * \defgroup        GSM_PRIVATE Private region
 * \brief           functions, structures and enumerations
 * \{
 */

extern gsm_t gsm;

#if !__DOXYGEN__

#define GSM_CHARISNUM(x)                    ((x) >= '0' && (x) <= '9')
#define GSM_CHARISHEXNUM(x)                 (((x) >= '0' && (x) <= '9') || ((x) >= 'a' && (x) <= 'f') || ((x) >= 'A' && (x) <= 'F'))
#define GSM_CHARTONUM(x)                    ((x) - '0')
#define GSM_CHARHEXTONUM(x)                 (((x) >= '0' && (x) <= '9') ? ((x) - '0') : (((x) >= 'a' && (x) <= 'f') ? ((x) - 'a' + 10) : (((x) >= 'A' && (x) <= 'F') ? ((x) - 'A' + 10) : 0)))
#define GSM_ISVALIDASCII(x)                 (((x) >= 32 && (x) <= 126) || (x) == '\r' || (x) == '\n')

/**
 * \brief           Protects (counts up) core from multiple accesses
 */
#define GSM_CORE_PROTECT()                  gsm_sys_protect()

/**
 * \brief           Unprotects (counts down) OS protection (mutex)
 */
#define GSM_CORE_UNPROTECT()                gsm_sys_unprotect()

const char * gsmi_dbg_msg_to_string(gsm_cmd_t cmd);

gsmr_t      gsmi_process(const void* data, size_t len);
gsmr_t      gsmi_process_buffer(void);

gsmr_t      gsmi_initiate_cmd(gsm_msg_t* msg);
uint8_t     gsmi_is_valid_conn_ptr(gsm_conn_p conn);
gsmr_t      gsmi_send_cb(gsm_cb_type_t type);
gsmr_t      gsmi_send_conn_cb(gsm_conn_t* conn, gsm_cb_fn cb);

void        gsmi_conn_init(void);

gsmr_t      gsmi_send_msg_to_producer_mbox(gsm_msg_t* msg, gsmr_t (*process_fn)(gsm_msg_t *), uint32_t block, uint32_t max_block_time);

#endif /* !__DOXYGEN__ */

/**
 * \}
 */
 
/**
 * \}
 */

#endif /* GSM_INTERNAL || __DOXYGEN__ */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GSM_PRIV_H */
