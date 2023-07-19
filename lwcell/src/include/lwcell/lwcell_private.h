/**
 * \file            lwcell_private.h
 * \brief           Private structures and enumerations
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
#ifndef LWCELL_PRIV_HDR_H
#define LWCELL_PRIV_HDR_H

#include "lwcell/lwcell.h"
#include "lwcell/lwcell_debug.h"
#include "lwcell/lwcell_includes.h"
#include "lwcell/lwcell_input.h"
#include "lwcell/lwcell_mem.h"
#include "lwcell/lwcell_opt.h"
#include "lwcell/lwcell_parser.h"
#include "lwcell/lwcell_timeout.h"
#include "lwcell/lwcell_types.h"
#include "lwcell/lwcell_unicode.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \addtogroup      LWCELL_TYPES
 * \{
 */

/**
 * \brief           List of possible messages
 */
typedef enum {
    LWCELL_CMD_IDLE = 0, /*!< IDLE mode */

    /* Basic AT commands */
    LWCELL_CMD_RESET,                  /*!< Reset device */
    LWCELL_CMD_RESET_DEVICE_FIRST_CMD, /*!< Reset device first driver specific command */
    LWCELL_CMD_ATE0,                   /*!< Disable ECHO mode on AT commands */
    LWCELL_CMD_ATE1,                   /*!< Enable ECHO mode on AT commands */
    LWCELL_CMD_GSLP,                   /*!< Set GSM to sleep mode */
    LWCELL_CMD_RESTORE,                /*!< Restore GSM internal settings to default values */
    LWCELL_CMD_UART,

    LWCELL_CMD_CGACT_SET_0,
    LWCELL_CMD_CGACT_SET_1,
    LWCELL_CMD_CGATT_SET_0,
    LWCELL_CMD_CGATT_SET_1,
    LWCELL_CMD_NETWORK_ATTACH, /*!< Attach to a network */
    LWCELL_CMD_NETWORK_DETACH, /*!< Detach from network */

    LWCELL_CMD_CIPMUX_SET,
    LWCELL_CMD_CIPRXGET_SET,
    LWCELL_CMD_CSTT_SET,

    /* AT commands according to the V.25TER */
    LWCELL_CMD_CALL_ENABLE,
    LWCELL_CMD_A,       /*!< Re-issues the Last Command Given */
    LWCELL_CMD_ATA,     /*!< Answer an Incoming Call */
    LWCELL_CMD_ATD,     /*!< Mobile Originated Call to Dial A Number */
    LWCELL_CMD_ATD_N,   /*!< Originate Call to Phone Number in Current Memory: ATD<n> */
    LWCELL_CMD_ATD_STR, /*!< Originate Call to Phone Number in Memory Which Corresponds to Field "str": ATD>str */
    LWCELL_CMD_ATDL,    /*!< Redial Last Telephone Number Used */
    LWCELL_CMD_ATE,     /*!< Set Command Echo Mode */
    LWCELL_CMD_ATH,     /*!< Disconnect Existing */
    LWCELL_CMD_ATI,     /*!< Display Product Identification Information */
    LWCELL_CMD_ATL,     /*!< Set Monitor speaker */
    LWCELL_CMD_ATM,     /*!< Set Monitor Speaker Mode */
    LWCELL_CMD_PPP,     /*!< Switch from Data Mode or PPP Online Mode to Command Mode, "+++" originally */
    LWCELL_CMD_ATO,     /*!< Switch from Command Mode to Data Mode */
    LWCELL_CMD_ATP,     /*!< Select Pulse Dialing */
    LWCELL_CMD_ATQ,     /*!< Set Result Code Presentation Mode */
    LWCELL_CMD_ATS0,    /*!< Set Number of Rings before Automatically Answering the Call */
    LWCELL_CMD_ATS3,    /*!< Set Command Line Termination Character */
    LWCELL_CMD_ATS4,    /*!< Set Response Formatting Character */
    LWCELL_CMD_ATS5,    /*!< Set Command Line Editing Character */
    LWCELL_CMD_ATS6,    /*!< Pause Before Blind */
    LWCELL_CMD_ATS7,    /*!< Set Number of Seconds to Wait for Connection Completion */
    LWCELL_CMD_ATS8, /*!< Set Number of Seconds to Wait for Comma Dial Modifier Encountered in Dial String of D Command */
    LWCELL_CMD_ATS10, /*!< Set Disconnect Delay after Indicating the Absence of Data Carrier */
    LWCELL_CMD_ATT,   /*!< Select Tone Dialing */
    LWCELL_CMD_ATV,   /*!< TA Response Format */
    LWCELL_CMD_ATX,   /*!< Set CONNECT Result Code Format and Monitor Call Progress */
    LWCELL_CMD_ATZ,   /*!< Reset Default Configuration */
    LWCELL_CMD_AT_C,  /*!< Set DCD Function Mode, AT&C */
    LWCELL_CMD_AT_D,  /*!< Set DTR Function, AT&D */
    LWCELL_CMD_AT_F,  /*!< Factory Defined Configuration, AT&F */
    LWCELL_CMD_AT_V,  /*!< Display Current Configuration, AT&V */
    LWCELL_CMD_AT_W,  /*!< Store Active Profile, AT&W */
    LWCELL_CMD_GCAP,  /*!< Request Complete TA Capabilities List */
    LWCELL_CMD_GMI,   /*!< Request Manufacturer Identification */
    LWCELL_CMD_GMM,   /*!< Request TA Model Identification */
    LWCELL_CMD_GMR,   /*!< Request TA Revision Identification of Software Release */
    LWCELL_CMD_GOI,   /*!< Request Global Object Identification */
    LWCELL_CMD_GSN,   /*!< Request TA Serial Number Identification (IMEI) */
    LWCELL_CMD_ICF,   /*!< Set TE-TA Control Character Framing */
    LWCELL_CMD_IFC,   /*!< Set TE-TA Local Data Flow Control */
    LWCELL_CMD_IPR,   /*!< Set TE-TA Fixed Local Rate */
    LWCELL_CMD_HVOIC, /*!< Disconnect Voice Call Only */

    /* AT commands according to 3GPP TS 27.007 */
    LWCELL_CMD_COPS_SET,     /*!< Set operator */
    LWCELL_CMD_COPS_GET,     /*!< Get current operator */
    LWCELL_CMD_COPS_GET_OPT, /*!< Get a list of available operators */
    LWCELL_CMD_CPAS,         /*!< Phone Activity Status */
    LWCELL_CMD_CGMI_GET,     /*!< Request Manufacturer Identification */
    LWCELL_CMD_CGMM_GET,     /*!< Request Model Identification */
    LWCELL_CMD_CGMR_GET,     /*!< Request TA Revision Identification of Software Release */
    LWCELL_CMD_CGSN_GET,     /*!< Request Product Serial Number Identification (Identical with +GSN) */

    LWCELL_CMD_CLCC_SET, /*!< List Current Calls of ME */
    LWCELL_CMD_CLCK,     /*!< Facility Lock */

    LWCELL_CMD_CACM,     /*!< Accumulated Call Meter (ACM) Reset or Query */
    LWCELL_CMD_CAMM,     /*!< Accumulated Call Meter Maximum (ACM max) Set or Query */
    LWCELL_CMD_CAOC,     /*!< Advice of Charge */
    LWCELL_CMD_CBST,     /*!< Select Bearer Service Type */
    LWCELL_CMD_CCFC,     /*!< Call Forwarding Number and Conditions Control */
    LWCELL_CMD_CCWA,     /*!< Call Waiting Control */
    LWCELL_CMD_CEER,     /*!< Extended Error Report  */
    LWCELL_CMD_CSCS,     /*!< Select TE Character Set */
    LWCELL_CMD_CSTA,     /*!< Select Type of Address */
    LWCELL_CMD_CHLD,     /*!< Call Hold and Multiparty */
    LWCELL_CMD_CIMI,     /*!< Request International Mobile Subscriber Identity */
    LWCELL_CMD_CLIP,     /*!< Calling Line Identification Presentation */
    LWCELL_CMD_CLIR,     /*!< Calling Line Identification Restriction */
    LWCELL_CMD_CMEE_SET, /*!< Report Mobile Equipment Error */
    LWCELL_CMD_COLP,     /*!< Connected Line Identification Presentation */

    LWCELL_CMD_PHONEBOOK_ENABLE,
    LWCELL_CMD_CPBF,         /*!< Find Phonebook Entries */
    LWCELL_CMD_CPBR,         /*!< Read Current Phonebook Entries  */
    LWCELL_CMD_CPBS_SET,     /*!< Select Phonebook Memory Storage */
    LWCELL_CMD_CPBS_GET,     /*!< Get current Phonebook Memory Storage */
    LWCELL_CMD_CPBS_GET_OPT, /*!< Get available Phonebook Memory Storages */
    LWCELL_CMD_CPBW_SET,     /*!< Write Phonebook Entry */
    LWCELL_CMD_CPBW_GET_OPT, /*!< Get options for write Phonebook Entry */

    LWCELL_CMD_SIM_PROCESS_BASIC_CMDS, /*!< Command setup, executed when SIM is in READY state */
    LWCELL_CMD_CPIN_SET,               /*!< Enter PIN */
    LWCELL_CMD_CPIN_GET,               /*!< Read current SIM status */
    LWCELL_CMD_CPIN_ADD,               /*!< Add new PIN to SIM if pin is not set */
    LWCELL_CMD_CPIN_CHANGE,            /*!< Change already active SIM */
    LWCELL_CMD_CPIN_REMOVE,            /*!< Remove current PIN */
    LWCELL_CMD_CPUK_SET,               /*!< Enter PUK and set new PIN */

    LWCELL_CMD_CSQ_GET,  /*!< Signal Quality Report */
    LWCELL_CMD_CFUN_SET, /*!< Set Phone Functionality */
    LWCELL_CMD_CFUN_GET, /*!< Get Phone Functionality */
    LWCELL_CMD_CREG_SET, /*!< Network Registration set output */
    LWCELL_CMD_CREG_GET, /*!< Get current network registration status */
    LWCELL_CMD_CBC,      /*!< Battery Charge */
    LWCELL_CMD_CNUM,     /*!< Subscriber Number */

    LWCELL_CMD_CPWD,     /*!< Change Password */
    LWCELL_CMD_CR,       /*!< Service Reporting Control */
    LWCELL_CMD_CRC,      /*!< Set Cellular Result Codes for Incoming Call Indication */
    LWCELL_CMD_CRLP,     /*!< Select Radio Link Protocol Parameters  */
    LWCELL_CMD_CRSM,     /*!< Restricted SIM Access */
    LWCELL_CMD_VTD,      /*!< Tone Duration */
    LWCELL_CMD_VTS,      /*!< DTMF and Tone Generation */
    LWCELL_CMD_CMUX,     /*!< Multiplexer Control */
    LWCELL_CMD_CPOL,     /*!< Preferred Operator List */
    LWCELL_CMD_COPN,     /*!< Read Operator Names */
    LWCELL_CMD_CCLK,     /*!< Clock */
    LWCELL_CMD_CSIM,     /*!< Generic SIM Access */
    LWCELL_CMD_CALM,     /*!< Alert Sound Mode */
    LWCELL_CMD_CALS,     /*!< Alert Sound Select */
    LWCELL_CMD_CRSL,     /*!< Ringer Sound Level */
    LWCELL_CMD_CLVL,     /*!< Loud Speaker Volume Level */
    LWCELL_CMD_CMUT,     /*!< Mute Control */
    LWCELL_CMD_CPUC,     /*!< Price Per Unit and Currency Table */
    LWCELL_CMD_CCWE,     /*!< Call Meter Maximum Event */
    LWCELL_CMD_CUSD_SET, /*!< Unstructured Supplementary Service Data, Set command */
    LWCELL_CMD_CUSD_GET, /*!< Unstructured Supplementary Service Data, Get command */
    LWCELL_CMD_CUSD,     /*!< Unstructured Supplementary Service Data, Execute command */
    LWCELL_CMD_CSSN,     /*!< Supplementary Services Notification */

    LWCELL_CMD_CIPMUX,     /*!< Start Up Multi-IP Connection */
    LWCELL_CMD_CIPSTART,   /*!< Start Up TCP or UDP Connection */
    LWCELL_CMD_CIPSEND,    /*!< Send Data Through TCP or UDP Connection */
    LWCELL_CMD_CIPQSEND,   /*!< Select Data Transmitting Mode */
    LWCELL_CMD_CIPACK,     /*!< Query Previous Connection Data Transmitting State */
    LWCELL_CMD_CIPCLOSE,   /*!< Close TCP or UDP Connection */
    LWCELL_CMD_CIPSHUT,    /*!< Deactivate GPRS PDP Context */
    LWCELL_CMD_CLPORT,     /*!< Set Local Port */
    LWCELL_CMD_CSTT,       /*!< Start Task and Set APN, username, password */
    LWCELL_CMD_CIICR,      /*!< Bring Up Wireless Connection with GPRS or CSD */
    LWCELL_CMD_CIFSR,      /*!< Get Local IP Address */
    LWCELL_CMD_CIPSTATUS,  /*!< Query Current Connection Status */
    LWCELL_CMD_CDNSCFG,    /*!< Configure Domain Name Server */
    LWCELL_CMD_CDNSGIP,    /*!< Query the IP Address of Given Domain Name */
    LWCELL_CMD_CIPHEAD,    /*!< Add an IP Head at the Beginning of a Package Received */
    LWCELL_CMD_CIPATS,     /*!< Set Auto Sending Timer */
    LWCELL_CMD_CIPSPRT,    /*!< Set Prompt of greater than sign When Module Sends Data */
    LWCELL_CMD_CIPSERVER,  /*!< Configure Module as Server */
    LWCELL_CMD_CIPCSGP,    /*!< Set CSD or GPRS for Connection Mode */
    LWCELL_CMD_CIPSRIP,    /*!< Show Remote IP Address and Port When Received Data */
    LWCELL_CMD_CIPDPDP,    /*!< Set Whether to Check State of GPRS Network Timing */
    LWCELL_CMD_CIPMODE,    /*!< Select TCPIP Application Mode */
    LWCELL_CMD_CIPCCFG,    /*!< Configure Transparent Transfer Mode */
    LWCELL_CMD_CIPSHOWTP,  /*!< Display Transfer Protocol in IP Head When Received Data */
    LWCELL_CMD_CIPUDPMODE, /*!< UDP Extended Mode */
    LWCELL_CMD_CIPRXGET,   /*!< Get Data from Network Manually */
    LWCELL_CMD_CIPSCONT,   /*!< Save TCPIP Application Context */
    LWCELL_CMD_CIPRDTIMER, /*!< Set Remote Delay Timer */
    LWCELL_CMD_CIPSGTXT,   /*!< Select GPRS PDP context */
    LWCELL_CMD_CIPTKA,     /*!< Set TCP Keepalive Parameters */
    LWCELL_CMD_CIPSSL,     /*!< Connection SSL function */

    LWCELL_CMD_SMS_ENABLE,
    LWCELL_CMD_CMGD,         /*!< Delete SMS Message */
    LWCELL_CMD_CMGF,         /*!< Select SMS Message Format */
    LWCELL_CMD_CMGL,         /*!< List SMS Messages from Preferred Store */
    LWCELL_CMD_CMGR,         /*!< Read SMS Message */
    LWCELL_CMD_CMGS,         /*!< Send SMS Message */
    LWCELL_CMD_CMGW,         /*!< Write SMS Message to Memory */
    LWCELL_CMD_CMSS,         /*!< Send SMS Message from Storage */
    LWCELL_CMD_CMGDA,        /*!< MASS SMS delete */
    LWCELL_CMD_CNMI,         /*!< New SMS Message Indications */
    LWCELL_CMD_CPMS_SET,     /*!< Set preferred SMS Message Storage */
    LWCELL_CMD_CPMS_GET,     /*!< Get preferred SMS Message Storage */
    LWCELL_CMD_CPMS_GET_OPT, /*!< Get optional SMS message storages */
    LWCELL_CMD_CRES,         /*!< Restore SMS Settings */
    LWCELL_CMD_CSAS,         /*!< Save SMS Settings */
    LWCELL_CMD_CSCA,         /*!< SMS Service Center Address */
    LWCELL_CMD_CSCB,         /*!< Select Cell Broadcast SMS Messages */
    LWCELL_CMD_CSDH,         /*!< Show SMS Text Mode Parameters */
    LWCELL_CMD_CSMP,         /*!< Set SMS Text Mode Parameters */
    LWCELL_CMD_CSMS,         /*!< Select Message Service */

    LWCELL_CMD_END, /*!< Last CMD entry */
} lwcell_cmd_t;

/**
 * \brief           Connection structure
 */
typedef struct lwcell_conn {
    lwcell_conn_type_t type;   /*!< Connection type */
    uint8_t num;              /*!< Connection number */
    lwcell_ip_t remote_ip;     /*!< Remote IP address */
    lwcell_port_t remote_port; /*!< Remote port number */
    lwcell_port_t local_port;  /*!< Local IP address */
    lwcell_evt_fn evt_func;    /*!< Callback function for connection */
    void* arg;                /*!< User custom argument */

    uint8_t val_id; /*!< Validation ID number. It is increased each time a new connection is established.
                                                     It protects sending data to wrong connection in case we have data in send queue,
                                                     and connection was closed and active again in between. */

    lwcell_linbuff_t buff; /*!< Linear buffer structure */

    size_t total_recved; /*!< Total number of bytes received */

    union {
        struct {
            uint8_t active        : 1; /*!< Status whether connection is active */
            uint8_t client        : 1; /*!< Status whether connection is in client mode */
            uint8_t data_received : 1; /*!< Status whether first data were received on connection */
            uint8_t in_closing    : 1; /*!< Status if connection is in closing mode.
                                                    When in closing mode, ignore any possible received data from function */
            uint8_t bearer        : 1; /*!< Bearer used. Can be `1` or `0` */
        } f;                           /*!< Connection flags */
    } status;                          /*!< Connection status union with flag bits */
} lwcell_conn_t;

/**
 * \ingroup         LWCELL_PBUF
 * \brief           Packet buffer structure
 */
typedef struct lwcell_pbuf {
    struct lwcell_pbuf* next; /*!< Next pbuf in chain list */
    size_t tot_len;          /*!< Total length of pbuf chain */
    size_t len;              /*!< Length of payload */
    size_t ref;              /*!< Number of references to this structure */
    uint8_t* payload;        /*!< Pointer to payload memory */
    lwcell_ip_t ip;           /*!< Remote address for received IPD data */
    lwcell_port_t port;       /*!< Remote port for received IPD data */
} lwcell_pbuf_t;

/**
 * \brief           Incoming network data read structure
 */
typedef struct {
    uint8_t read;      /*!< Set to 1 when we should process input data as connection data */
    size_t tot_len;    /*!< Total length of packet */
    size_t rem_len;    /*!< Remaining bytes to read in current +IPD statement */
    lwcell_conn_p conn; /*!< Pointer to connection for network data */

    size_t buff_ptr;   /*!< Buffer pointer to save data to.
                                                     When set to `NULL` while `read = 1`, reading should ignore incoming data */
    lwcell_pbuf_p buff; /*!< Pointer to data buffer used for receiving data */
} lwcell_ipd_t;

/**
 * \brief           Connection result on connect command
 */
typedef enum {
    LWCELL_CONN_CONNECT_UNKNOWN, /*!< No valid result */
    LWCELL_CONN_CONNECT_OK,      /*!< Connected OK */
    LWCELL_CONN_CONNECT_ERROR,   /*!< Error on connection */
    LWCELL_CONN_CONNECT_ALREADY, /*!< Already connected */
} lwcell_conn_connect_res_t;

/**
 * \brief           Message queue structure to share between threads
 */
typedef struct lwcell_msg {
    lwcell_cmd_t cmd_def; /*!< Default message type received from queue */
    lwcell_cmd_t cmd;     /*!< Since some commands can have different subcommands, sub command is used here */
    uint8_t i;           /*!< Variable to indicate order number of subcommands */
    lwcell_sys_sem_t sem; /*!< Semaphore for the message */
    uint8_t is_blocking; /*!< Status if command is blocking */
    uint32_t block_time; /*!< Maximal blocking time in units of milliseconds. Use 0 to for non-blocking call */
    lwcellr_t res;        /*!< Result of message operation */
    lwcellr_t (*fn)(struct lwcell_msg*); /*!< Processing callback function to process packet */

#if LWCELL_CFG_USE_API_FUNC_EVT
    lwcell_api_cmd_evt_fn evt_fn; /*!< Command callback API function */
    void* evt_arg;               /*!< Command callback API callback parameter */
#endif                           /* LWCELL_CFG_USE_API_FUNC_EVT */

    union {
        struct {
            uint32_t delay; /*!< Delay to use before sending first reset AT command */
        } reset;            /*!< Reset device */

        struct {
            uint32_t baudrate; /*!< Baudrate for AT port */
        } uart;                /*!< UART configuration */

        struct {
            uint8_t mode; /*!< Functionality mode */
        } cfun;           /*!< Set phone functionality */

        struct {
            const char* pin; /*!< Pin code to write */
        } cpin_enter;        /*!< Enter pin code */

        struct {
            const char* pin; /*!< New pin code */
        } cpin_add;          /*!< Add pin code if previously wasn't set */

        struct {
            const char* current_pin; /*!< Current pin code */
            const char* new_pin;     /*!< New pin code */
        } cpin_change;               /*!< Change current pin code */

        struct {
            const char* pin; /*!< Current pin code */
        } cpin_remove;       /*!< Remove PIN code */

        struct {
            const char* puk; /*!< PUK code */
            const char* pin; /*!< New PIN code */
        } cpuk_enter;        /*!< Enter PUK and new PIN */

        struct {
            size_t cnum_tries; /*!< Number of tries */
        } sim_info;            /*!< Get information for SIM card */

        struct {
            char* str;  /*!< Pointer to output string array */
            size_t len; /*!< Length of output string array including trailing zero memory */
        } device_info;  /*!< All kind of device info, serial number, model, manufacturer, revision */

        struct {
            int16_t* rssi; /*!< Pointer to RSSI variable */
        } csq;             /*!< Signal strength */

        struct {
            uint8_t read;          /*!< Flag indicating we can read the COPS actual data */
            lwcell_operator_t* ops; /*!< Pointer to operators array */
            size_t opsl;           /*!< Length of operators array */
            size_t opsi;           /*!< Current operator index array */
            size_t* opf;           /*!< Pointer to number of operators found */
        } cops_scan;               /*!< Scan operators */

        struct {
            lwcell_operator_curr_t* curr; /*!< Pointer to output current operator */
        } cops_get;                      /*!< Get current operator info */

        struct {
            lwcell_operator_mode_t mode;     /*!< COPS mode */
            lwcell_operator_format_t format; /*!< Operator format to print */
            const char* name;               /*!< Short or long name, according to format */
            uint32_t num;                   /*!< Number in case format is number */
        } cops_set;                         /*!< Set operator settings */

#if LWCELL_CFG_CONN || __DOXYGEN__
        /* Connection based commands */
        struct {
            lwcell_conn_t** conn;               /*!< Pointer to pointer to save connection used */
            const char* host;                  /*!< Host to use for connection */
            lwcell_port_t port;                 /*!< Remote port used for connection */
            lwcell_conn_type_t type;            /*!< Connection type */
            void* arg;                         /*!< Connection custom argument */
            lwcell_evt_fn evt_func;             /*!< Callback function to use on connection */
            uint8_t num;                       /*!< Connection number used for start */
            lwcell_conn_connect_res_t conn_res; /*!< Connection result status */
        } conn_start;                          /*!< Structure for starting new connection */

        struct {
            lwcell_conn_t* conn; /*!< Pointer to connection to close */
            uint8_t val_id;     /*!< Connection current validation ID when command was sent to queue */
        } conn_close;           /*!< Close connection */

        struct {
            lwcell_conn_t* conn;          /*!< Pointer to connection to send data */
            size_t btw;                  /*!< Number of remaining bytes to write */
            size_t ptr;                  /*!< Current write pointer for data */
            const uint8_t* data;         /*!< Data to send */
            size_t sent;                 /*!< Number of bytes sent in last packet */
            size_t sent_all;             /*!< Number of bytes sent all together */
            uint8_t tries;               /*!< Number of tries used for last packet */
            uint8_t wait_send_ok_err;    /*!< Set to 1 when we wait for SEND OK or SEND ERROR */
            const lwcell_ip_t* remote_ip; /*!< Remote IP address for UDP connection */
            lwcell_port_t remote_port;    /*!< Remote port address for UDP connection */
            uint8_t fau;                 /*!< Free after use flag to free memory after data are sent (or not) */
            size_t* bw;                  /*!< Number of bytes written so far */
            uint8_t val_id;              /*!< Connection current validation ID when command was sent to queue */
        } conn_send;                     /*!< Structure to send data on connection */
#endif                                   /* LWCELL_CFG_CONN || __DOXYGEN__ */
#if LWCELL_CFG_SMS || __DOXYGEN__
        struct {
            const char* num;  /*!< Phone number */
            const char* text; /*!< SMS content to send */
            uint8_t format;   /*!< SMS format, `0 = PDU`, `1 = text` */
            size_t pos;       /*!< Set on +CMGS response if command is OK */
        } sms_send;           /*!< Send SMS */

        struct {
            lwcell_mem_t mem;          /*!< Memory to read from */
            size_t pos;               /*!< SMS position in memory */
            lwcell_sms_entry_t* entry; /*!< Pointer to entry to write info */
            uint8_t update;           /*!< Update SMS status after read operation */
            uint8_t format;           /*!< SMS format, `0 = PDU`, `1 = text` */
            uint8_t read;             /*!< Read the data flag */
        } sms_read;                   /*!< Read single SMS */

        struct {
            lwcell_mem_t mem; /*!< Memory to delete from */
            size_t pos;      /*!< SMS position in memory */
        } sms_delete;        /*!< Delete SMS message */

        struct {
            lwcell_sms_status_t status; /*!< SMS status to delete */
        } sms_delete_all;              /*!< Mass delete SMS messages */

        struct {
            lwcell_mem_t mem;            /*!< Memory to use for read */
            lwcell_sms_status_t status;  /*!< SMS entries status */
            lwcell_sms_entry_t* entries; /*!< Pointer to entries */
            size_t etr;                 /*!< Entries to read (array length) */
            size_t ei;                  /*!< Current entry index in array */
            size_t* er;                 /*!< Final entries read pointer for user */
            uint8_t update;             /*!< Update SMS status after read operation */
            uint8_t format;             /*!< SMS format, `0 = PDU`, `1 = text` */
            uint8_t read;               /*!< Read the data flag */
        } sms_list;                     /*!< List SMS messages */

        struct {
            lwcell_mem_t mem[3]; /*!< Array of memories */
        } sms_memory;           /*!< Set preferred memories */
#endif                          /* LWCELL_CFG_SMS || __DOXYGEN__ */
#if LWCELL_CFG_CALL || __DOXYGEN__
        struct {
            const char* number; /*!< Phone number to dial */
        } call_start;           /*!< Start a new call */
#endif                          /* LWCELL_CFG_CALL || __DOXYGEN__ */
#if LWCELL_CFG_PHONEBOOK || __DOXYGEN__
        struct {
            lwcell_mem_t mem;          /*!< Memory to use */
            size_t pos;               /*!< Memory position. Set to 0 to use new one or SIZE_T MAX to delete entry */
            const char* name;         /*!< Entry name */
            const char* num;          /*!< Entry number */
            lwcell_number_type_t type; /*!< Entry phone number type */
            uint8_t del;              /*!< Flag indicates delete */
        } pb_write;                   /*!< Write/Edit/Delete entry */

        struct {
            lwcell_mem_t mem;           /*!< Memory to use */
            size_t start_index;        /*!< Start index in phonebook to read */
            lwcell_pb_entry_t* entries; /*!< Pointer to entries array */
            size_t etr;                /*!< NUmber of entries to read */
            size_t ei;                 /*!< Current entry index */
            size_t* er;                /*!< Final entries read pointer for user */
        } pb_list;                     /*!< List phonebook entries */

        struct {
            lwcell_mem_t mem;           /*!< Memory to use */
            lwcell_pb_entry_t* entries; /*!< Pointer to entries array */
            size_t etr;                /*!< NUmber of entries to read */
            size_t ei;                 /*!< Current entry index */
            size_t* er;                /*!< Final entries read pointer for user */
            const char* search;        /*!< Search string */
        } pb_search;                   /*!< Search phonebook entries */
#endif                                 /* LWCELL_CFG_PHONEBOOK || __DOXYGEN__ */
        struct {
            const char* code;      /*!< Code to send */
            char* resp;            /*!< Response array */
            size_t resp_len;       /*!< Length of response array */
            uint8_t read;          /*!< Flag indicating we can read the COPS actual data */
            size_t resp_write_ptr; /*!< Write pointer for response */
            uint8_t quote_det;     /*!< Information if quote has been detected */
        } ussd;                    /*!< Execute USSD command */
#if LWCELL_CFG_NETWORK || __DOXYGEN__
        struct {
            const char* apn;  /*!< APN address */
            const char* user; /*!< APN username */
            const char* pass; /*!< APN password */
        } network_attach;     /*!< Settings for network attach */
#endif                        /* LWCELL_CFG_NETWORK || __DOXYGEN__ */
    } msg;                    /*!< Group of different possible message contents */
} lwcell_msg_t;

/**
 * \brief           IP and MAC structure with netmask and gateway addresses
 */
typedef struct {
    lwcell_ip_t ip;   /*!< IP address */
    lwcell_ip_t gw;   /*!< Gateway address */
    lwcell_ip_t nm;   /*!< Netmask address */
    lwcell_mac_t mac; /*!< MAC address */
} lwcell_ip_mac_t;

/**
 * \brief           Link connection active info
 */
typedef struct {
    uint8_t failed;           /*!< Status if connection successful */
    uint8_t num;              /*!< Connection number */
    uint8_t is_server;        /*!< Status if connection is client or server */
    lwcell_conn_type_t type;   /*!< Connection type */
    lwcell_ip_t remote_ip;     /*!< Remote IP address */
    lwcell_port_t remote_port; /*!< Remote port */
    lwcell_port_t local_port;  /*!< Local port number */
} lwcell_link_conn_t;

/**
 * \brief           Callback function linked list prototype
 */
typedef struct lwcell_evt_func {
    struct lwcell_evt_func* next; /*!< Next function in the list */
    lwcell_evt_fn fn;             /*!< Function pointer itself */
} lwcell_evt_func_t;

/**
 * \ingroup         LWCELL_SMS
 * \brief           SMS memory information
 */
typedef struct {
    uint32_t mem_available; /*!< Bit field of available memories */
    lwcell_mem_t current;    /*!< Current memory choice */
    size_t total;           /*!< Size of memory in units of entries */
    size_t used;            /*!< Number of used entries */
} lwcell_sms_mem_t;

/**
 * \ingroup         LWCELL_SMS
 * \brief           SMS structure
 */
typedef struct {
    uint8_t ready;   /*!< Flag indicating feature ready by device */
    uint8_t enabled; /*!< Flag indicating feature enabled */

    lwcell_sms_mem_t mem[3]; /*!< 3 memory info for operation,receive,sent storage */
} lwcell_sms_t;

/**
 * \ingroup         LWCELL_SMS
 * \brief           SMS memory information
 */
typedef struct {
    uint32_t mem_available; /*!< Bit field of available memories */
    lwcell_mem_t current;    /*!< Current memory choice */
    size_t total;           /*!< Size of memory in units of entries */
    size_t used;            /*!< Number of used entries */
} lwcell_pb_mem_t;

/**
 * \ingroup         LWCELL_PB
 * \brief           Phonebook structure
 */
typedef struct {
    uint8_t ready;   /*!< Flag indicating feature ready by device */
    uint8_t enabled; /*!< Flag indicating feature enabled */

    lwcell_pb_mem_t mem; /*!< Memory information */
} lwcell_pb_t;

/**
 * \brief           SIM structure
 */
typedef struct {
    lwcell_sim_state_t state; /*!< Current SIM status */
} lwcell_sim_t;

/**
 * \brief           Network info
 */
typedef struct {
    lwcell_network_reg_status_t status;   /*!< Network registration status */
    lwcell_operator_curr_t curr_operator; /*!< Current operator information */

    uint8_t is_attached; /*!< Flag indicating device is attached and PDP context is active */
    lwcell_ip_t ip_addr;  /*!< Device IP address when network PDP context is enabled */
} lwcell_network_t;

/**
 * \brief           GSM modules structure
 */
typedef struct {
    /* Device identification */
    char model_manufacturer[20];  /*!< Device manufacturer */
    char model_number[20];        /*!< Device model number */
    char model_serial_number[20]; /*!< Device serial number */
    char model_revision[20];      /*!< Device revision */
    lwcell_device_model_t model;   /*!< Device model */

    /* Network&operator specific */
    lwcell_sim_t sim;         /*!< SIM data */
    lwcell_network_t network; /*!< Network status */
    int16_t rssi;            /*!< RSSI signal strength. `0` = invalid, `-53 % -113` = valid */

    /* Device specific */
#if LWCELL_CFG_CONN || __DOXYGEN__
    uint8_t active_conns_cur_parse_num; /*!< Current connection number used for parsing */

    lwcell_conn_t conns[LWCELL_CFG_MAX_CONNS]; /*!< Array of all connection structures */
    lwcell_ipd_t ipd;                         /*!< Connection incoming data structure */
    uint8_t conn_val_id;                     /*!< Validation ID increased each time device connects to network */
#endif                                       /* LWCELL_CFG_CONNS || __DOXYGEN__ */
#if LWCELL_CFG_SMS || __DOXYGEN__
    lwcell_sms_t sms; /*!< SMS information */
#endif               /* LWCELL_CFG_SMS || __DOXYGEN__ */
#if LWCELL_CFG_PHONEBOOK || __DOXYGEN__
    lwcell_pb_t pb; /*!< Phonebook information */
#endif             /* LWCELL_CFG_PHONEBOOK || __DOXYGEN__ */
#if LWCELL_CFG_CALL || __DOXYGEN__
    lwcell_call_t call; /*!< Call information */
#endif                 /* LWCELL_CFG_CALL || __DOXYGEN__ */
} lwcell_modules_t;

/**
 * \brief           GSM global structure
 */
typedef struct {
    size_t locked_cnt; /*!< Counter how many times (recursive) stack is currently locked */

    lwcell_sys_sem_t sem_sync;          /*!< Synchronization semaphore between threads */
    lwcell_sys_mbox_t mbox_producer;    /*!< Producer message queue handle */
    lwcell_sys_mbox_t mbox_process;     /*!< Consumer message queue handle */
    lwcell_sys_thread_t thread_produce; /*!< Producer thread handle */
    lwcell_sys_thread_t thread_process; /*!< Processing thread handle */
#if !LWCELL_CFG_INPUT_USE_PROCESS || __DOXYGEN__
    lwcell_buff_t buff; /*!< Input processing buffer */
#endif                 /* !LWCELL_CFG_INPUT_USE_PROCESS || __DOXYGEN__ */
    lwcell_ll_t ll;     /*!< Low level functions */

    lwcell_msg_t* msg; /*!< Pointer to current user message being executed */

    lwcell_evt_t evt;            /*!< Callback processing structure */
    lwcell_evt_func_t* evt_func; /*!< Callback function linked list */

    lwcell_modules_t m; /*!< All modules. When resetting, reset structure */

    union {
        struct {
            uint8_t initialized : 1; /*!< Flag indicating GSM library is initialized */
            uint8_t dev_present : 1; /*!< Flag indicating GSM device is present */
        } f;                         /*!< Flags structure */
    } status;                        /*!< Status structure */
} lwcell_t;

/**
 * \brief           Memory mapping structure between string and value in app
 */
typedef struct {
    lwcell_mem_t mem;     /*!< Mem indication */
    const char* mem_str; /*!< Memory string */
} lwcell_dev_mem_map_t;

/**
 * \brief           Device models map between model and other information
 */
typedef struct {
    lwcell_device_model_t model; /*!< Device model */
    const char* id_str;         /*!< Model string identification */
    uint8_t is_2g;              /*!< Status if modem is 2G */
    uint8_t is_lte;             /*!< Status if modem is LTE */
} lwcell_dev_model_map_t;

/**
 * \}
 */

#if !__DOXYGEN__

/**
 * \ingroup         LWCELL
 * \defgroup        LWCELL_PRIVATE Internal functions
 * \brief           Functions, structures and enumerations
 * \{
 */

extern lwcell_t lwcell;

extern const lwcell_dev_mem_map_t lwcell_dev_mem_map[];
extern const size_t lwcell_dev_mem_map_size;

extern const lwcell_dev_model_map_t lwcell_dev_model_map[];
extern const size_t lwcell_dev_model_map_size;

#define CMD_IS_CUR(c)              (lwcell.msg != NULL && lwcell.msg->cmd == (c))
#define CMD_IS_DEF(c)              (lwcell.msg != NULL && lwcell.msg->cmd_def == (c))
#define CMD_GET_CUR()              ((lwcell_cmd_t)(((lwcell.msg != NULL) ? lwcell.msg->cmd : LWCELL_CMD_IDLE)))
#define CMD_GET_DEF()              ((lwcell_cmd_t)(((lwcell.msg != NULL) ? lwcell.msg->cmd_def : LWCELL_CMD_IDLE)))

#define CRLF                       "\r\n"
#define CRLF_LEN                   2

#define LWCELL_MSG_VAR_DEFINE(name) lwcell_msg_t* name
#define LWCELL_MSG_VAR_ALLOC(name, blocking)                                                                            \
    do {                                                                                                               \
        (name) = lwcell_mem_malloc(sizeof(*(name)));                                                                    \
        LWCELL_DEBUGW(LWCELL_CFG_DBG_VAR | LWCELL_DBG_TYPE_TRACE, (name) != NULL,                                         \
                     "[MSG VAR] Allocated %d bytes at %p\r\n", (int)sizeof(*(name)), (void*)(name));                   \
        LWCELL_DEBUGW(LWCELL_CFG_DBG_VAR | LWCELL_DBG_TYPE_TRACE, (name) == NULL,                                         \
                     "[MSG VAR] Error allocating %d bytes\r\n", (int)sizeof(*(name)));                                 \
        if ((name) == NULL) {                                                                                          \
            return lwcellERRMEM;                                                                                        \
        }                                                                                                              \
        LWCELL_MEMSET((name), 0x00, sizeof(*(name)));                                                                   \
        (name)->is_blocking = LWCELL_U8((blocking) > 0);                                                                \
    } while (0)
#define LWCELL_MSG_VAR_REF(name) (*(name))
#define LWCELL_MSG_VAR_FREE(name)                                                                                       \
    do {                                                                                                               \
        LWCELL_DEBUGF(LWCELL_CFG_DBG_VAR | LWCELL_DBG_TYPE_TRACE, "[MSG VAR] Free memory: %p\r\n", (void*)(name));        \
        if (lwcell_sys_sem_isvalid(&((name)->sem))) {                                                                   \
            lwcell_sys_sem_delete(&((name)->sem));                                                                      \
            lwcell_sys_sem_invalid(&((name)->sem));                                                                     \
        }                                                                                                              \
        lwcell_mem_free_s((void**)&(name));                                                                             \
    } while (0)
#if LWCELL_CFG_USE_API_FUNC_EVT
#define LWCELL_MSG_VAR_SET_EVT(name, e_fn, e_arg)                                                                       \
    do {                                                                                                               \
        (name)->evt_fn = (e_fn);                                                                                       \
        (name)->evt_arg = (e_arg);                                                                                     \
    } while (0)
#else /* LWCELL_CFG_USE_API_FUNC_EVT */
#define LWCELL_MSG_VAR_SET_EVT(name, e_fn, e_arg)                                                                       \
    do {                                                                                                               \
        LWCELL_UNUSED(e_fn);                                                                                            \
        LWCELL_UNUSED(e_arg);                                                                                           \
    } while (0)
#endif /* !LWCELL_CFG_USE_API_FUNC_EVT */

#define LWCELL_CHARISNUM(x)    ((x) >= '0' && (x) <= '9')
#define LWCELL_CHARTONUM(x)    ((x) - '0')
#define LWCELL_CHARISHEXNUM(x) (((x) >= '0' && (x) <= '9') || ((x) >= 'a' && (x) <= 'f') || ((x) >= 'A' && (x) <= 'F'))
#define LWCELL_CHARHEXTONUM(x)                                                                                          \
    (((x) >= '0' && (x) <= '9')                                                                                        \
         ? ((x) - '0')                                                                                                 \
         : (((x) >= 'a' && (x) <= 'f') ? ((x) - 'a' + 10) : (((x) >= 'A' && (x) <= 'F') ? ((x) - 'A' + 10) : 0)))
#define LWCELL_ISVALIDASCII(x) (((x) >= 32 && (x) <= 126) || (x) == '\r' || (x) == '\n')

#define LWCELL_PORT2NUM(port)  ((uint32_t)(port))

const char* lwcelli_dbg_msg_to_string(lwcell_cmd_t cmd);
lwcellr_t lwcelli_process(const void* data, size_t len);
lwcellr_t lwcelli_process_buffer(void);
lwcellr_t lwcelli_initiate_cmd(lwcell_msg_t* msg);
uint8_t lwcelli_is_valid_conn_ptr(lwcell_conn_p conn);
lwcellr_t lwcelli_send_cb(lwcell_evt_type_t type);
lwcellr_t lwcelli_send_conn_cb(lwcell_conn_t* conn, lwcell_evt_fn cb);
void lwcelli_conn_init(void);
lwcellr_t lwcelli_send_msg_to_producer_mbox(lwcell_msg_t* msg, lwcellr_t (*process_fn)(lwcell_msg_t*),
                                          uint32_t max_block_time);
uint32_t lwcelli_get_from_mbox_with_timeout_checks(lwcell_sys_mbox_t* b, void** m, uint32_t timeout);
uint8_t lwcelli_conn_closed_process(uint8_t conn_num, uint8_t forced);
void lwcelli_conn_start_timeout(lwcell_conn_p conn);

lwcellr_t lwcelli_get_sim_info(const uint32_t blocking);

void lwcelli_reset_everything(uint8_t forced);
void lwcelli_process_events_for_timeout_or_error(lwcell_msg_t* msg, lwcellr_t err);

/**
 * \}
 */

#endif /* !__DOXYGEN__ */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWCELL_PRIV_HDR_H */
