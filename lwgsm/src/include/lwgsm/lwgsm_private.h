/**
 * \file            lwgsm_private.h
 * \brief           Private structures and enumerations
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
#ifndef LWGSM_HDR_PRIV_H
#define LWGSM_HDR_PRIV_H

#include "lwgsm/lwgsm.h"
#include "lwgsm/lwgsm_typedefs.h"
#include "lwgsm/lwgsm_debug.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \addtogroup      LWGSM_TYPEDEFS
 * \{
 */

/**
 * \brief           List of possible messages
 */
typedef enum {
    LWGSM_CMD_IDLE = 0,                         /*!< IDLE mode */

    /* Basic AT commands */
    LWGSM_CMD_RESET,                            /*!< Reset device */
    LWGSM_CMD_RESET_DEVICE_FIRST_CMD,           /*!< Reset device first driver specific command */
    LWGSM_CMD_ATE0,                             /*!< Disable ECHO mode on AT commands */
    LWGSM_CMD_ATE1,                             /*!< Enable ECHO mode on AT commands */
    LWGSM_CMD_GSLP,                             /*!< Set GSM to sleep mode */
    LWGSM_CMD_RESTORE,                          /*!< Restore GSM internal settings to default values */
    LWGSM_CMD_UART,

    LWGSM_CMD_CGACT_SET_0,
    LWGSM_CMD_CGACT_SET_1,
    LWGSM_CMD_CGATT_SET_0,
    LWGSM_CMD_CGATT_SET_1,
    LWGSM_CMD_NETWORK_ATTACH,                   /*!< Attach to a network */
    LWGSM_CMD_NETWORK_DETACH,                   /*!< Detach from network */

    LWGSM_CMD_CIPMUX_SET,
    LWGSM_CMD_CIPRXGET_SET,
    LWGSM_CMD_CSTT_SET,



    /* AT commands according to the V.25TER */
    LWGSM_CMD_CALL_ENABLE,
    LWGSM_CMD_A,                                /*!< Re-issues the Last Command Given */
    LWGSM_CMD_ATA,                              /*!< Answer an Incoming Call */
    LWGSM_CMD_ATD,                              /*!< Mobile Originated Call to Dial A Number */
    LWGSM_CMD_ATD_N,                            /*!< Originate Call to Phone Number in Current Memory: ATD<n> */
    LWGSM_CMD_ATD_STR,                          /*!< Originate Call to Phone Number in Memory Which Corresponds to Field "str": ATD>str */
    LWGSM_CMD_ATDL,                             /*!< Redial Last Telephone Number Used */
    LWGSM_CMD_ATE,                              /*!< Set Command Echo Mode */
    LWGSM_CMD_ATH,                              /*!< Disconnect Existing */
    LWGSM_CMD_ATI,                              /*!< Display Product Identification Information */
    LWGSM_CMD_ATL,                              /*!< Set Monitor speaker */
    LWGSM_CMD_ATM,                              /*!< Set Monitor Speaker Mode */
    LWGSM_CMD_PPP,                              /*!< Switch from Data Mode or PPP Online Mode to Command Mode, "+++" originally */
    LWGSM_CMD_ATO,                              /*!< Switch from Command Mode to Data Mode */
    LWGSM_CMD_ATP,                              /*!< Select Pulse Dialing */
    LWGSM_CMD_ATQ,                              /*!< Set Result Code Presentation Mode */
    LWGSM_CMD_ATS0,                             /*!< Set Number of Rings before Automatically Answering the Call */
    LWGSM_CMD_ATS3,                             /*!< Set Command Line Termination Character */
    LWGSM_CMD_ATS4,                             /*!< Set Response Formatting Character */
    LWGSM_CMD_ATS5,                             /*!< Set Command Line Editing Character */
    LWGSM_CMD_ATS6,                             /*!< Pause Before Blind */
    LWGSM_CMD_ATS7,                             /*!< Set Number of Seconds to Wait for Connection Completion */
    LWGSM_CMD_ATS8,                             /*!< Set Number of Seconds to Wait for Comma Dial Modifier Encountered in Dial String of D Command */
    LWGSM_CMD_ATS10,                            /*!< Set Disconnect Delay after Indicating the Absence of Data Carrier */
    LWGSM_CMD_ATT,                              /*!< Select Tone Dialing */
    LWGSM_CMD_ATV,                              /*!< TA Response Format */
    LWGSM_CMD_ATX,                              /*!< Set CONNECT Result Code Format and Monitor Call Progress */
    LWGSM_CMD_ATZ,                              /*!< Reset Default Configuration */
    LWGSM_CMD_AT_C,                             /*!< Set DCD Function Mode, AT&C */
    LWGSM_CMD_AT_D,                             /*!< Set DTR Function, AT&D */
    LWGSM_CMD_AT_F,                             /*!< Factory Defined Configuration, AT&F */
    LWGSM_CMD_AT_V,                             /*!< Display Current Configuration, AT&V */
    LWGSM_CMD_AT_W,                             /*!< Store Active Profile, AT&W */
    LWGSM_CMD_GCAP,                             /*!< Request Complete TA Capabilities List */
    LWGSM_CMD_GMI,                              /*!< Request Manufacturer Identification */
    LWGSM_CMD_GMM,                              /*!< Request TA Model Identification */
    LWGSM_CMD_GMR,                              /*!< Request TA Revision Identification of Software Release */
    LWGSM_CMD_GOI,                              /*!< Request Global Object Identification */
    LWGSM_CMD_GSN,                              /*!< Request TA Serial Number Identification (IMEI) */
    LWGSM_CMD_ICF,                              /*!< Set TE-TA Control Character Framing */
    LWGSM_CMD_IFC,                              /*!< Set TE-TA Local Data Flow Control */
    LWGSM_CMD_IPR,                              /*!< Set TE-TA Fixed Local Rate */
    LWGSM_CMD_HVOIC,                            /*!< Disconnect Voice Call Only */

    /* AT commands according to 3GPP TS 27.007 */
    LWGSM_CMD_COPS_SET,                         /*!< Set operator */
    LWGSM_CMD_COPS_GET,                         /*!< Get current operator */
    LWGSM_CMD_COPS_GET_OPT,                     /*!< Get a list of available operators */
    LWGSM_CMD_CPAS,                             /*!< Phone Activity Status */
    LWGSM_CMD_CGMI_GET,                         /*!< Request Manufacturer Identification */
    LWGSM_CMD_CGMM_GET,                         /*!< Request Model Identification */
    LWGSM_CMD_CGMR_GET,                         /*!< Request TA Revision Identification of Software Release */
    LWGSM_CMD_CGSN_GET,                         /*!< Request Product Serial Number Identification (Identical with +GSN) */

    LWGSM_CMD_CLCC_SET,                         /*!< List Current Calls of ME */
    LWGSM_CMD_CLCK,                             /*!< Facility Lock */

    LWGSM_CMD_CACM,                             /*!< Accumulated Call Meter (ACM) Reset or Query */
    LWGSM_CMD_CAMM,                             /*!< Accumulated Call Meter Maximum (ACM max) Set or Query */
    LWGSM_CMD_CAOC,                             /*!< Advice of Charge */
    LWGSM_CMD_CBST,                             /*!< Select Bearer Service Type */
    LWGSM_CMD_CCFC,                             /*!< Call Forwarding Number and Conditions Control */
    LWGSM_CMD_CCWA,                             /*!< Call Waiting Control */
    LWGSM_CMD_CEER,                             /*!< Extended Error Report  */
    LWGSM_CMD_CSCS,                             /*!< Select TE Character Set */
    LWGSM_CMD_CSTA,                             /*!< Select Type of Address */
    LWGSM_CMD_CHLD,                             /*!< Call Hold and Multiparty */
    LWGSM_CMD_CIMI,                             /*!< Request International Mobile Subscriber Identity */
    LWGSM_CMD_CLIP,                             /*!< Calling Line Identification Presentation */
    LWGSM_CMD_CLIR,                             /*!< Calling Line Identification Restriction */
    LWGSM_CMD_CMEE_SET,                         /*!< Report Mobile Equipment Error */
    LWGSM_CMD_COLP,                             /*!< Connected Line Identification Presentation */

    LWGSM_CMD_PHONEBOOK_ENABLE,
    LWGSM_CMD_CPBF,                             /*!< Find Phonebook Entries */
    LWGSM_CMD_CPBR,                             /*!< Read Current Phonebook Entries  */
    LWGSM_CMD_CPBS_SET,                         /*!< Select Phonebook Memory Storage */
    LWGSM_CMD_CPBS_GET,                         /*!< Get current Phonebook Memory Storage */
    LWGSM_CMD_CPBS_GET_OPT,                     /*!< Get available Phonebook Memory Storages */
    LWGSM_CMD_CPBW_SET,                         /*!< Write Phonebook Entry */
    LWGSM_CMD_CPBW_GET_OPT,                     /*!< Get options for write Phonebook Entry */

    LWGSM_CMD_SIM_PROCESS_BASIC_CMDS,           /*!< Command setup, executed when SIM is in READY state */
    LWGSM_CMD_CPIN_SET,                         /*!< Enter PIN */
    LWGSM_CMD_CPIN_GET,                         /*!< Read current SIM status */
    LWGSM_CMD_CPIN_ADD,                         /*!< Add new PIN to SIM if pin is not set */
    LWGSM_CMD_CPIN_CHANGE,                      /*!< Change already active SIM */
    LWGSM_CMD_CPIN_REMOVE,                      /*!< Remove current PIN */
    LWGSM_CMD_CPUK_SET,                         /*!< Enter PUK and set new PIN */

    LWGSM_CMD_CSQ_GET,                          /*!< Signal Quality Report */
    LWGSM_CMD_CFUN_SET,                         /*!< Set Phone Functionality */
    LWGSM_CMD_CFUN_GET,                         /*!< Get Phone Functionality */
    LWGSM_CMD_CREG_SET,                         /*!< Network Registration set output */
    LWGSM_CMD_CREG_GET,                         /*!< Get current network registration status */
    LWGSM_CMD_CBC,                              /*!< Battery Charge */
    LWGSM_CMD_CNUM,                             /*!< Subscriber Number */

    LWGSM_CMD_CPWD,                             /*!< Change Password */
    LWGSM_CMD_CR,                               /*!< Service Reporting Control */
    LWGSM_CMD_CRC,                              /*!< Set Cellular Result Codes for Incoming Call Indication */
    LWGSM_CMD_CRLP,                             /*!< Select Radio Link Protocol Parameters  */
    LWGSM_CMD_CRSM,                             /*!< Restricted SIM Access */
    LWGSM_CMD_VTD,                              /*!< Tone Duration */
    LWGSM_CMD_VTS,                              /*!< DTMF and Tone Generation */
    LWGSM_CMD_CMUX,                             /*!< Multiplexer Control */
    LWGSM_CMD_CPOL,                             /*!< Preferred Operator List */
    LWGSM_CMD_COPN,                             /*!< Read Operator Names */
    LWGSM_CMD_CCLK,                             /*!< Clock */
    LWGSM_CMD_CSIM,                             /*!< Generic SIM Access */
    LWGSM_CMD_CALM,                             /*!< Alert Sound Mode */
    LWGSM_CMD_CALS,                             /*!< Alert Sound Select */
    LWGSM_CMD_CRSL,                             /*!< Ringer Sound Level */
    LWGSM_CMD_CLVL,                             /*!< Loud Speaker Volume Level */
    LWGSM_CMD_CMUT,                             /*!< Mute Control */
    LWGSM_CMD_CPUC,                             /*!< Price Per Unit and Currency Table */
    LWGSM_CMD_CCWE,                             /*!< Call Meter Maximum Event */
    LWGSM_CMD_CUSD_SET,                         /*!< Unstructured Supplementary Service Data, Set command */
    LWGSM_CMD_CUSD_GET,                         /*!< Unstructured Supplementary Service Data, Get command */
    LWGSM_CMD_CUSD,                             /*!< Unstructured Supplementary Service Data, Execute command */
    LWGSM_CMD_CSSN,                             /*!< Supplementary Services Notification */

    LWGSM_CMD_CIPMUX,                           /*!< Start Up Multi-IP Connection */
    LWGSM_CMD_CIPSTART,                         /*!< Start Up TCP or UDP Connection */
    LWGSM_CMD_CIPSEND,                          /*!< Send Data Through TCP or UDP Connection */
    LWGSM_CMD_CIPQSEND,                         /*!< Select Data Transmitting Mode */
    LWGSM_CMD_CIPACK,                           /*!< Query Previous Connection Data Transmitting State */
    LWGSM_CMD_CIPCLOSE,                         /*!< Close TCP or UDP Connection */
    LWGSM_CMD_CIPSHUT,                          /*!< Deactivate GPRS PDP Context */
    LWGSM_CMD_CLPORT,                           /*!< Set Local Port */
    LWGSM_CMD_CSTT,                             /*!< Start Task and Set APN, username, password */
    LWGSM_CMD_CIICR,                            /*!< Bring Up Wireless Connection with GPRS or CSD */
    LWGSM_CMD_CIFSR,                            /*!< Get Local IP Address */
    LWGSM_CMD_CIPSTATUS,                        /*!< Query Current Connection Status */
    LWGSM_CMD_CDNSCFG,                          /*!< Configure Domain Name Server */
    LWGSM_CMD_CDNSGIP,                          /*!< Query the IP Address of Given Domain Name */
    LWGSM_CMD_CIPHEAD,                          /*!< Add an IP Head at the Beginning of a Package Received */
    LWGSM_CMD_CIPATS,                           /*!< Set Auto Sending Timer */
    LWGSM_CMD_CIPSPRT,                          /*!< Set Prompt of greater than sign When Module Sends Data */
    LWGSM_CMD_CIPSERVER,                        /*!< Configure Module as Server */
    LWGSM_CMD_CIPCSGP,                          /*!< Set CSD or GPRS for Connection Mode */
    LWGSM_CMD_CIPSRIP,                          /*!< Show Remote IP Address and Port When Received Data */
    LWGSM_CMD_CIPDPDP,                          /*!< Set Whether to Check State of GPRS Network Timing */
    LWGSM_CMD_CIPMODE,                          /*!< Select TCPIP Application Mode */
    LWGSM_CMD_CIPCCFG,                          /*!< Configure Transparent Transfer Mode */
    LWGSM_CMD_CIPSHOWTP,                        /*!< Display Transfer Protocol in IP Head When Received Data */
    LWGSM_CMD_CIPUDPMODE,                       /*!< UDP Extended Mode */
    LWGSM_CMD_CIPRXGET,                         /*!< Get Data from Network Manually */
    LWGSM_CMD_CIPSCONT,                         /*!< Save TCPIP Application Context */
    LWGSM_CMD_CIPRDTIMER,                       /*!< Set Remote Delay Timer */
    LWGSM_CMD_CIPSGTXT,                         /*!< Select GPRS PDP context */
    LWGSM_CMD_CIPTKA,                           /*!< Set TCP Keepalive Parameters */
    LWGSM_CMD_CIPSSL,                           /*!< Connection SSL function */

    LWGSM_CMD_SMS_ENABLE,
    LWGSM_CMD_CMGD,                             /*!< Delete SMS Message */
    LWGSM_CMD_CMGF,                             /*!< Select SMS Message Format */
    LWGSM_CMD_CMGL,                             /*!< List SMS Messages from Preferred Store */
    LWGSM_CMD_CMGR,                             /*!< Read SMS Message */
    LWGSM_CMD_CMGS,                             /*!< Send SMS Message */
    LWGSM_CMD_CMGW,                             /*!< Write SMS Message to Memory */
    LWGSM_CMD_CMSS,                             /*!< Send SMS Message from Storage */
    LWGSM_CMD_CMGDA,                            /*!< MASS SMS delete */
    LWGSM_CMD_CNMI,                             /*!< New SMS Message Indications */
    LWGSM_CMD_CPMS_SET,                         /*!< Set preferred SMS Message Storage */
    LWGSM_CMD_CPMS_GET,                         /*!< Get preferred SMS Message Storage */
    LWGSM_CMD_CPMS_GET_OPT,                     /*!< Get optional SMS message storages */
    LWGSM_CMD_CRES,                             /*!< Restore SMS Settings */
    LWGSM_CMD_CSAS,                             /*!< Save SMS Settings */
    LWGSM_CMD_CSCA,                             /*!< SMS Service Center Address */
    LWGSM_CMD_CSCB,                             /*!< Select Cell Broadcast SMS Messages */
    LWGSM_CMD_CSDH,                             /*!< Show SMS Text Mode Parameters */
    LWGSM_CMD_CSMP,                             /*!< Set SMS Text Mode Parameters */
    LWGSM_CMD_CSMS,                             /*!< Select Message Service */

    LWGSM_CMD_END,                              /*!< Last CMD entry */
} lwgsm_cmd_t;

/**
 * \brief           Connection structure
 */
typedef struct lwgsm_conn {
    lwgsm_conn_type_t type;                     /*!< Connection type */
    uint8_t         num;                        /*!< Connection number */
    lwgsm_ip_t        remote_ip;                /*!< Remote IP address */
    lwgsm_port_t      remote_port;              /*!< Remote port number */
    lwgsm_port_t      local_port;               /*!< Local IP address */
    lwgsm_evt_fn      evt_func;                 /*!< Callback function for connection */
    void*           arg;                        /*!< User custom argument */

    uint8_t         val_id;                     /*!< Validation ID number. It is increased each time a new connection is established.
                                                     It protects sending data to wrong connection in case we have data in send queue,
                                                     and connection was closed and active again in between. */

    lwgsm_linbuff_t   buff;                     /*!< Linear buffer structure */

    size_t          total_recved;               /*!< Total number of bytes received */

    union {
        struct {
            uint8_t active: 1;                  /*!< Status whether connection is active */
            uint8_t client: 1;                  /*!< Status whether connection is in client mode */
            uint8_t data_received: 1;           /*!< Status whether first data were received on connection */
            uint8_t in_closing: 1;               /*!< Status if connection is in closing mode.
                                                    When in closing mode, ignore any possible received data from function */
            uint8_t bearer: 1;                  /*!< Bearer used. Can be `1` or `0` */
        } f;                                    /*!< Connection flags */
    } status;                                   /*!< Connection status union with flag bits */
} lwgsm_conn_t;

/**
 * \ingroup         LWGSM_PBUF
 * \brief           Packet buffer structure
 */
typedef struct lwgsm_pbuf {
    struct lwgsm_pbuf* next;                    /*!< Next pbuf in chain list */
    size_t tot_len;                             /*!< Total length of pbuf chain */
    size_t len;                                 /*!< Length of payload */
    size_t ref;                                 /*!< Number of references to this structure */
    uint8_t* payload;                           /*!< Pointer to payload memory */
    lwgsm_ip_t ip;                              /*!< Remote address for received IPD data */
    lwgsm_port_t port;                          /*!< Remote port for received IPD data */
} lwgsm_pbuf_t;

/**
 * \brief           Incoming network data read structure
 */
typedef struct {
    uint8_t             read;                   /*!< Set to 1 when we should process input data as connection data */
    size_t              tot_len;                /*!< Total length of packet */
    size_t              rem_len;                /*!< Remaining bytes to read in current +IPD statement */
    lwgsm_conn_p          conn;                 /*!< Pointer to connection for network data */

    size_t              buff_ptr;               /*!< Buffer pointer to save data to.
                                                     When set to `NULL` while `read = 1`, reading should ignore incoming data */
    lwgsm_pbuf_p          buff;                 /*!< Pointer to data buffer used for receiving data */
} lwgsm_ipd_t;

/**
 * \brief           Connection result on connect command
 */
typedef enum {
    LWGSM_CONN_CONNECT_UNKNOWN,                 /*!< No valid result */
    LWGSM_CONN_CONNECT_OK,                      /*!< Connected OK */
    LWGSM_CONN_CONNECT_ERROR,                   /*!< Error on connection */
    LWGSM_CONN_CONNECT_ALREADY,                 /*!< Already connected */
} lwgsm_conn_connect_res_t;

/**
 * \brief           Message queue structure to share between threads
 */
typedef struct lwgsm_msg {
    lwgsm_cmd_t       cmd_def;                  /*!< Default message type received from queue */
    lwgsm_cmd_t       cmd;                      /*!< Since some commands can have different subcommands, sub command is used here */
    uint8_t         i;                          /*!< Variable to indicate order number of subcommands */
    lwgsm_sys_sem_t   sem;                      /*!< Semaphore for the message */
    uint8_t         is_blocking;                /*!< Status if command is blocking */
    uint32_t        block_time;                 /*!< Maximal blocking time in units of milliseconds. Use 0 to for non-blocking call */
    lwgsmr_t          res;                      /*!< Result of message operation */
    lwgsmr_t          (*fn)(struct lwgsm_msg*); /*!< Processing callback function to process packet */

#if LWGSM_CFG_USE_API_FUNC_EVT
    lwgsm_api_cmd_evt_fn evt_fn;                /*!< Command callback API function */
    void*           evt_arg;                    /*!< Command callback API callback parameter */
#endif /* LWGSM_CFG_USE_API_FUNC_EVT */

    union {
        struct {
            uint32_t delay;                     /*!< Delay to use before sending first reset AT command */
        } reset;                                /*!< Reset device */
        struct {
            uint32_t baudrate;                  /*!< Baudrate for AT port */
        } uart;                                 /*!< UART configuration */

        struct {
            uint8_t mode;                       /*!< Functionality mode */
        } cfun;                                 /*!< Set phone functionality */

        struct {
            const char* pin;                    /*!< Pin code to write */
        } cpin_enter;                           /*!< Enter pin code */
        struct {
            const char* pin;                    /*!< New pin code */
        } cpin_add;                             /*!< Add pin code if previously wasn't set */
        struct {
            const char* current_pin;            /*!< Current pin code */
            const char* new_pin;                /*!< New pin code */
        } cpin_change;                          /*!< Change current pin code */
        struct {
            const char* pin;                    /*!< Current pin code */
        } cpin_remove;                          /*!< Remove PIN code */
        struct {
            const char* puk;                    /*!< PUK code */
            const char* pin;                    /*!< New PIN code */
        } cpuk_enter;                           /*!< Enter PUK and new PIN */
        struct {
            size_t cnum_tries;                  /*!< Number of tries */
        } sim_info;                             /*!< Get information for SIM card */

        struct {
            char* str;                          /*!< Pointer to output string array */
            size_t len;                         /*!< Length of output string array including trailing zero memory */
        } device_info;                          /*!< All kind of device info, serial number, model, manufacturer, revision */

        struct {
            int16_t* rssi;                      /*!< Pointer to RSSI variable */
        } csq;                                  /*!< Signal strength */
        struct {
            uint8_t read;                       /*!< Flag indicating we can read the COPS actual data */
            lwgsm_operator_t* ops;              /*!< Pointer to operators array */
            size_t opsl;                        /*!< Length of operators array */
            size_t opsi;                        /*!< Current operator index array */
            size_t* opf;                        /*!< Pointer to number of operators found */
        } cops_scan;                            /*!< Scan operators */
        struct {
            lwgsm_operator_curr_t* curr;        /*!< Pointer to output current operator */
        } cops_get;                             /*!< Get current operator info */
        struct {
            lwgsm_operator_mode_t mode;         /*!< COPS mode */
            lwgsm_operator_format_t format;     /*!< Operator format to print */
            const char* name;                   /*!< Short or long name, according to format */
            uint32_t num;                       /*!< Number in case format is number */
        } cops_set;                             /*!< Set operator settings */

#if LWGSM_CFG_CONN || __DOXYGEN__
        /* Connection based commands */
        struct {
            lwgsm_conn_t** conn;                /*!< Pointer to pointer to save connection used */
            const char* host;                   /*!< Host to use for connection */
            lwgsm_port_t port;                  /*!< Remote port used for connection */
            lwgsm_conn_type_t type;             /*!< Connection type */
            void* arg;                          /*!< Connection custom argument */
            lwgsm_evt_fn evt_func;              /*!< Callback function to use on connection */
            uint8_t num;                        /*!< Connection number used for start */
            lwgsm_conn_connect_res_t conn_res;  /*!< Connection result status */
        } conn_start;                           /*!< Structure for starting new connection */
        struct {
            lwgsm_conn_t* conn;                 /*!< Pointer to connection to close */
            uint8_t val_id;                     /*!< Connection current validation ID when command was sent to queue */
        } conn_close;                           /*!< Close connection */
        struct {
            lwgsm_conn_t* conn;                 /*!< Pointer to connection to send data */
            size_t btw;                         /*!< Number of remaining bytes to write */
            size_t ptr;                         /*!< Current write pointer for data */
            const uint8_t* data;                /*!< Data to send */
            size_t sent;                        /*!< Number of bytes sent in last packet */
            size_t sent_all;                    /*!< Number of bytes sent all together */
            uint8_t tries;                      /*!< Number of tries used for last packet */
            uint8_t wait_send_ok_err;           /*!< Set to 1 when we wait for SEND OK or SEND ERROR */
            const lwgsm_ip_t* remote_ip;        /*!< Remote IP address for UDP connection */
            lwgsm_port_t remote_port;           /*!< Remote port address for UDP connection */
            uint8_t fau;                        /*!< Free after use flag to free memory after data are sent (or not) */
            size_t* bw;                         /*!< Number of bytes written so far */
            uint8_t val_id;                     /*!< Connection current validation ID when command was sent to queue */
        } conn_send;                            /*!< Structure to send data on connection */
#endif /* LWGSM_CFG_CONN || __DOXYGEN__ */
#if LWGSM_CFG_SMS || __DOXYGEN__
        struct {
            const char* num;                    /*!< Phone number */
            const char* text;                   /*!< SMS content to send */
            uint8_t format;                     /*!< SMS format, `0 = PDU`, `1 = text` */
            size_t pos;                         /*!< Set on +CMGS response if command is OK */
        } sms_send;                             /*!< Send SMS */
        struct {
            lwgsm_mem_t mem;                    /*!< Memory to read from */
            size_t pos;                         /*!< SMS position in memory */
            lwgsm_sms_entry_t* entry;           /*!< Pointer to entry to write info */
            uint8_t update;                     /*!< Update SMS status after read operation */
            uint8_t format;                     /*!< SMS format, `0 = PDU`, `1 = text` */
            uint8_t read;                       /*!< Read the data flag */
        } sms_read;                             /*!< Read single SMS */
        struct {
            lwgsm_mem_t mem;                    /*!< Memory to delete from */
            size_t pos;                         /*!< SMS position in memory */
        } sms_delete;                           /*!< Delete SMS message */
        struct {
            lwgsm_sms_status_t status;          /*!< SMS status to delete */
        } sms_delete_all;                       /*!< Mass delete SMS messages */
        struct {
            lwgsm_mem_t mem;                    /*!< Memory to use for read */
            lwgsm_sms_status_t status;          /*!< SMS entries status */
            lwgsm_sms_entry_t* entries;         /*!< Pointer to entries */
            size_t etr;                         /*!< Entries to read (array length) */
            size_t ei;                          /*!< Current entry index in array */
            size_t* er;                         /*!< Final entries read pointer for user */
            uint8_t update;                     /*!< Update SMS status after read operation */
            uint8_t format;                     /*!< SMS format, `0 = PDU`, `1 = text` */
            uint8_t read;                       /*!< Read the data flag */
        } sms_list;                             /*!< List SMS messages */
        struct {
            lwgsm_mem_t mem[3];                 /*!< Array of memories */
        } sms_memory;                           /*!< Set preferred memories */
#endif /* LWGSM_CFG_SMS || __DOXYGEN__ */
#if LWGSM_CFG_CALL || __DOXYGEN__
        struct {
            const char* number;                 /*!< Phone number to dial */
        } call_start;                           /*!< Start a new call */
#endif /* LWGSM_CFG_CALL || __DOXYGEN__ */
#if LWGSM_CFG_PHONEBOOK || __DOXYGEN__
        struct {
            lwgsm_mem_t mem;                    /*!< Memory to use */
            size_t pos;                         /*!< Memory position. Set to 0 to use new one or SIZE_T MAX to delete entry */
            const char* name;                   /*!< Entry name */
            const char* num;                    /*!< Entry number */
            lwgsm_number_type_t type;           /*!< Entry phone number type */
            uint8_t del;                        /*!< Flag indicates delete */
        } pb_write;                             /*!< Write/Edit/Delete entry */
        struct {
            lwgsm_mem_t mem;                    /*!< Memory to use */
            size_t start_index;                 /*!< Start index in phonebook to read */
            lwgsm_pb_entry_t* entries;          /*!< Pointer to entries array */
            size_t etr;                         /*!< NUmber of entries to read */
            size_t ei;                          /*!< Current entry index */
            size_t* er;                         /*!< Final entries read pointer for user */
        } pb_list;                              /*!< List phonebook entries */
        struct {
            lwgsm_mem_t mem;                    /*!< Memory to use */
            lwgsm_pb_entry_t* entries;          /*!< Pointer to entries array */
            size_t etr;                         /*!< NUmber of entries to read */
            size_t ei;                          /*!< Current entry index */
            size_t* er;                         /*!< Final entries read pointer for user */
            const char* search;                 /*!< Search string */
        } pb_search;                            /*!< Search phonebook entries */
#endif /* LWGSM_CFG_PHONEBOOK || __DOXYGEN__ */
        struct {
            const char* code;                   /*!< Code to send */
            char* resp;                         /*!< Response array */
            size_t resp_len;                    /*!< Length of response array */
            uint8_t read;                       /*!< Flag indicating we can read the COPS actual data */
            size_t resp_write_ptr;              /*!< Write pointer for response */
            uint8_t quote_det;                  /*!< Information if quote has been detected */
        } ussd;                                 /*!< Execute USSD command */
#if LWGSM_CFG_NETWORK || __DOXYGEN__
        struct {
            const char* apn;                    /*!< APN address */
            const char* user;                   /*!< APN username */
            const char* pass;                   /*!< APN password */
        } network_attach;                       /*!< Settings for network attach */
#endif /* LWGSM_CFG_NETWORK || __DOXYGEN__ */
    } msg;                                      /*!< Group of different possible message contents */
} lwgsm_msg_t;

/**
 * \brief           IP and MAC structure with netmask and gateway addresses
 */
typedef struct {
    lwgsm_ip_t ip;                              /*!< IP address */
    lwgsm_ip_t gw;                              /*!< Gateway address */
    lwgsm_ip_t nm;                              /*!< Netmask address */
    lwgsm_mac_t mac;                            /*!< MAC address */
} lwgsm_ip_mac_t;

/**
 * \brief           Link connection active info
 */
typedef struct {
    uint8_t failed;                             /*!< Status if connection successful */
    uint8_t num;                                /*!< Connection number */
    uint8_t is_server;                          /*!< Status if connection is client or server */
    lwgsm_conn_type_t type;                     /*!< Connection type */
    lwgsm_ip_t remote_ip;                       /*!< Remote IP address */
    lwgsm_port_t remote_port;                   /*!< Remote port */
    lwgsm_port_t local_port;                    /*!< Local port number */
} lwgsm_link_conn_t;

/**
 * \brief           Callback function linked list prototype
 */
typedef struct lwgsm_evt_func {
    struct lwgsm_evt_func* next;                /*!< Next function in the list */
    lwgsm_evt_fn fn;                            /*!< Function pointer itself */
} lwgsm_evt_func_t;

/**
 * \ingroup         LWGSM_SMS
 * \brief           SMS memory information
 */
typedef struct {
    uint32_t mem_available;                     /*!< Bit field of available memories */
    lwgsm_mem_t current;                        /*!< Current memory choice */
    size_t total;                               /*!< Size of memory in units of entries */
    size_t used;                                /*!< Number of used entries */
} lwgsm_sms_mem_t;

/**
 * \ingroup         LWGSM_SMS
 * \brief           SMS structure
 */
typedef struct {
    uint8_t ready;                              /*!< Flag indicating feature ready by device */
    uint8_t enabled;                            /*!< Flag indicating feature enabled */

    lwgsm_sms_mem_t mem[3];                     /*!< 3 memory info for operation,receive,sent storage */
} lwgsm_sms_t;

/**
 * \ingroup         LWGSM_SMS
 * \brief           SMS memory information
 */
typedef struct {
    uint32_t mem_available;                     /*!< Bit field of available memories */
    lwgsm_mem_t current;                        /*!< Current memory choice */
    size_t total;                               /*!< Size of memory in units of entries */
    size_t used;                                /*!< Number of used entries */
} lwgsm_pb_mem_t;

/**
 * \ingroup         LWGSM_PB
 * \brief           Phonebook structure
 */
typedef struct {
    uint8_t ready;                              /*!< Flag indicating feature ready by device */
    uint8_t enabled;                            /*!< Flag indicating feature enabled */

    lwgsm_pb_mem_t mem;                         /*!< Memory information */
} lwgsm_pb_t;

/**
 * \brief           SIM structure
 */
typedef struct {
    lwgsm_sim_state_t state;                    /*!< Current SIM status */
} lwgsm_sim_t;

/**
 * \brief           Network info
 */
typedef struct {
    lwgsm_network_reg_status_t status;          /*!< Network registration status */
    lwgsm_operator_curr_t curr_operator;        /*!< Current operator information */

    uint8_t is_attached;                        /*!< Flag indicating device is attached and PDP context is active */
    lwgsm_ip_t ip_addr;                         /*!< Device IP address when network PDP context is enabled */
} lwgsm_network_t;

/**
 * \brief           GSM modules structure
 */
typedef struct {
    /* Device identification */
    char                model_manufacturer[20]; /*!< Device manufacturer */
    char                model_number[20];       /*!< Device model number */
    char                model_serial_number[20];/*!< Device serial number */
    char                model_revision[20];     /*!< Device revision */
    lwgsm_device_model_t  model;                /*!< Device model */

    /* Network&operator specific */
    lwgsm_sim_t           sim;                  /*!< SIM data */
    lwgsm_network_t       network;              /*!< Network status */
    int16_t             rssi;                   /*!< RSSI signal strength. `0` = invalid, `-53 % -113` = valid */

    /* Device specific */
#if LWGSM_CFG_CONN || __DOXYGEN__
    uint8_t             active_conns_cur_parse_num; /*!< Current connection number used for parsing */

    lwgsm_conn_t          conns[LWGSM_CFG_MAX_CONNS];   /*!< Array of all connection structures */
    lwgsm_ipd_t           ipd;                  /*!< Connection incoming data structure */
    uint8_t             conn_val_id;            /*!< Validation ID increased each time device connects to network */
#endif /* LWGSM_CFG_CONNS || __DOXYGEN__ */
#if LWGSM_CFG_SMS || __DOXYGEN__
    lwgsm_sms_t           sms;                  /*!< SMS information */
#endif /* LWGSM_CFG_SMS || __DOXYGEN__ */
#if LWGSM_CFG_PHONEBOOK || __DOXYGEN__
    lwgsm_pb_t            pb;                   /*!< Phonebook information */
#endif /* LWGSM_CFG_PHONEBOOK || __DOXYGEN__ */
#if LWGSM_CFG_CALL || __DOXYGEN__
    lwgsm_call_t          call;                 /*!< Call information */
#endif /* LWGSM_CFG_CALL || __DOXYGEN__ */
} lwgsm_modules_t;

/**
 * \brief           GSM global structure
 */
typedef struct {
    size_t              locked_cnt;             /*!< Counter how many times (recursive) stack is currently locked */

    lwgsm_sys_sem_t       sem_sync;             /*!< Synchronization semaphore between threads */
    lwgsm_sys_mbox_t      mbox_producer;        /*!< Producer message queue handle */
    lwgsm_sys_mbox_t      mbox_process;         /*!< Consumer message queue handle */
    lwgsm_sys_thread_t    thread_produce;       /*!< Producer thread handle */
    lwgsm_sys_thread_t    thread_process;       /*!< Processing thread handle */
#if !LWGSM_CFG_INPUT_USE_PROCESS || __DOXYGEN__
    lwgsm_buff_t          buff;                 /*!< Input processing buffer */
#endif /* !LWGSM_CFG_INPUT_USE_PROCESS || __DOXYGEN__ */
    lwgsm_ll_t            ll;                   /*!< Low level functions */

    lwgsm_msg_t*          msg;                  /*!< Pointer to current user message being executed */

    lwgsm_evt_t           evt;                  /*!< Callback processing structure */
    lwgsm_evt_func_t*     evt_func;             /*!< Callback function linked list */

    lwgsm_modules_t       m;                    /*!< All modules. When resetting, reset structure */

    union {
        struct {
            uint8_t     initialized: 1;         /*!< Flag indicating GSM library is initialized */
            uint8_t     dev_present: 1;         /*!< Flag indicating GSM device is present */
        } f;                                    /*!< Flags structure */
    } status;                                   /*!< Status structure */
} lwgsm_t;

/**
 * \brief           Memory mapping structure between string and value in app
 */
typedef struct {
    lwgsm_mem_t mem;                            /*!< Mem indication */
    const char* mem_str;                        /*!< Memory string */
} lwgsm_dev_mem_map_t;

/**
 * \brief           Device models map between model and other information
 */
typedef struct {
    lwgsm_device_model_t model;                 /*!< Device model */
    const char* id_str;                         /*!< Model string identification */
    uint8_t is_2g;                              /*!< Status if modem is 2G */
    uint8_t is_lte;                             /*!< Status if modem is LTE */
} lwgsm_dev_model_map_t;

/**
 * \ingroup         LWGSM_UNICODE
 * \brief           Unicode support structure
 */
typedef struct {
    uint8_t ch[4];                              /*!< UTF-8 max characters */
    uint8_t t;                                  /*!< Total expected length in UTF-8 sequence */
    uint8_t r;                                  /*!< Remaining bytes in UTF-8 sequence */
    lwgsmr_t res;                               /*!< Current result of processing */
} lwgsm_unicode_t;

/**
 * \}
 */

#if !__DOXYGEN__

/**
 * \ingroup         LWGSM
 * \defgroup        LWGSM_PRIVATE Internal functions
 * \brief           Functions, structures and enumerations
 * \{
 */

extern lwgsm_t                lwgsm;

extern const lwgsm_dev_mem_map_t  lwgsm_dev_mem_map[];
extern const size_t         lwgsm_dev_mem_map_size;

extern const lwgsm_dev_model_map_t    lwgsm_dev_model_map[];
extern const size_t         lwgsm_dev_model_map_size;

#define CMD_IS_CUR(c)       (lwgsm.msg != NULL && lwgsm.msg->cmd == (c))
#define CMD_IS_DEF(c)       (lwgsm.msg != NULL && lwgsm.msg->cmd_def == (c))
#define CMD_GET_CUR()       ((lwgsm_cmd_t)(((lwgsm.msg != NULL) ? lwgsm.msg->cmd : LWGSM_CMD_IDLE)))
#define CMD_GET_DEF()       ((lwgsm_cmd_t)(((lwgsm.msg != NULL) ? lwgsm.msg->cmd_def : LWGSM_CMD_IDLE)))

#define CRLF                "\r\n"
#define CRLF_LEN            2

#define LWGSM_MSG_VAR_DEFINE(name)                lwgsm_msg_t* name
#define LWGSM_MSG_VAR_ALLOC(name, blocking)           do {\
        (name) = lwgsm_mem_malloc(sizeof(*(name)));       \
        LWGSM_DEBUGW(LWGSM_CFG_DBG_VAR | LWGSM_DBG_TYPE_TRACE, (name) != NULL, "[MSG VAR] Allocated %d bytes at %p\r\n", sizeof(*(name)), (name)); \
        LWGSM_DEBUGW(LWGSM_CFG_DBG_VAR | LWGSM_DBG_TYPE_TRACE, (name) == NULL, "[MSG VAR] Error allocating %d bytes\r\n", sizeof(*(name))); \
        if ((name) == NULL) {                           \
            return lwgsmERRMEM;                           \
        }                                               \
        LWGSM_MEMSET((name), 0x00, sizeof(*(name)));      \
        (name)->is_blocking = LWGSM_U8((blocking) > 0);   \
    } while (0)
#define LWGSM_MSG_VAR_REF(name)                   (*(name))
#define LWGSM_MSG_VAR_FREE(name)                  do {\
        LWGSM_DEBUGF(LWGSM_CFG_DBG_VAR | LWGSM_DBG_TYPE_TRACE, "[MSG VAR] Free memory: %p\r\n", (name)); \
        if (lwgsm_sys_sem_isvalid(&((name)->sem))) {      \
            lwgsm_sys_sem_delete(&((name)->sem));         \
            lwgsm_sys_sem_invalid(&((name)->sem));        \
        }                                               \
        lwgsm_mem_free_s((void **)&(name));               \
    } while (0)
#if LWGSM_CFG_USE_API_FUNC_EVT
#define LWGSM_MSG_VAR_SET_EVT(name, e_fn, e_arg)  do {\
        (name)->evt_fn = (e_fn);                        \
        (name)->evt_arg = (e_arg);                      \
    } while (0)
#else /* LWGSM_CFG_USE_API_FUNC_EVT */
#define LWGSM_MSG_VAR_SET_EVT(name, e_fn, e_arg) do { LWGSM_UNUSED(e_fn); LWGSM_UNUSED(e_arg); } while (0)
#endif /* !LWGSM_CFG_USE_API_FUNC_EVT */

#define LWGSM_CHARISNUM(x)                    ((x) >= '0' && (x) <= '9')
#define LWGSM_CHARTONUM(x)                    ((x) - '0')
#define LWGSM_CHARISHEXNUM(x)                 (((x) >= '0' && (x) <= '9') || ((x) >= 'a' && (x) <= 'f') || ((x) >= 'A' && (x) <= 'F'))
#define LWGSM_CHARHEXTONUM(x)                 (((x) >= '0' && (x) <= '9') ? ((x) - '0') : (((x) >= 'a' && (x) <= 'f') ? ((x) - 'a' + 10) : (((x) >= 'A' && (x) <= 'F') ? ((x) - 'A' + 10) : 0)))
#define LWGSM_ISVALIDASCII(x)                 (((x) >= 32 && (x) <= 126) || (x) == '\r' || (x) == '\n')

#define LWGSM_PORT2NUM(port)                  ((uint32_t)(port))

const char* lwgsmi_dbg_msg_to_string(lwgsm_cmd_t cmd);
lwgsmr_t      lwgsmi_process(const void* data, size_t len);
lwgsmr_t      lwgsmi_process_buffer(void);
lwgsmr_t      lwgsmi_initiate_cmd(lwgsm_msg_t* msg);
uint8_t     lwgsmi_is_valid_conn_ptr(lwgsm_conn_p conn);
lwgsmr_t      lwgsmi_send_cb(lwgsm_evt_type_t type);
lwgsmr_t      lwgsmi_send_conn_cb(lwgsm_conn_t* conn, lwgsm_evt_fn cb);
void        lwgsmi_conn_init(void);
lwgsmr_t      lwgsmi_send_msg_to_producer_mbox(lwgsm_msg_t* msg, lwgsmr_t (*process_fn)(lwgsm_msg_t*), uint32_t max_block_time);
uint32_t    lwgsmi_get_from_mbox_with_timeout_checks(lwgsm_sys_mbox_t* b, void** m, uint32_t timeout);
uint8_t     lwgsmi_conn_closed_process(uint8_t conn_num, uint8_t forced);
void        lwgsmi_conn_start_timeout(lwgsm_conn_p conn);

lwgsmr_t      lwgsmi_get_sim_info(const uint32_t blocking);

void        lwgsmi_reset_everything(uint8_t forced);
void        lwgsmi_process_events_for_timeout_or_error(lwgsm_msg_t* msg, lwgsmr_t err);

/**
 * \}
 */

#endif /* !__DOXYGEN__ */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWGSM_HDR_PRIV_H */
