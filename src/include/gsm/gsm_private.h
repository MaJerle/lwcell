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

#include "gsm/gsm.h"
#include "gsm/gsm_typedefs.h"
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
    GSM_CMD_GSLP,                               /*!< Set GSM to sleep mode */
    GSM_CMD_RESTORE,                            /*!< Restore GSM internal settings to default values */
    GSM_CMD_UART,

#if GSM_CFG_NETWORK || __DOXYGEN__
    GSM_CMD_CGACT_SET_0,
    GSM_CMD_CGACT_SET_1,
    GSM_CMD_CGATT_SET_0,
    GSM_CMD_CGATT_SET_1,
    GSM_CMD_NETWORK_ATTACH,                     /*!< Attach to a network */
    GSM_CMD_NETWORK_DETACH,                     /*!< Detach from network */
#endif /* GSM_CFG_NETWORK */

    /*
     * AT commands according to the V.25TER
     */
    GSM_CMD_A,                                  /*!< Re-issues the Last Command Given */
    GSM_CMD_ATA,                                /*!< Answer an Incoming Call */
    GSM_CMD_ATD,                                /*!< Mobile Originated Call to Dial A Number */
    GSM_CMD_ATD_N,                              /*!< Originate Call to Phone Number in Current Memory: ATD<n> */
    GSM_CMD_ATD_STR,                            /*!< Originate Call to Phone Number in Memory Which Corresponds to Field "str": ATD>str */
    GSM_CMD_ATDL,                               /*!< Redial Last Telephone Number Used */
    GSM_CMD_ATE,                                /*!< Set Command Echo Mode */
    GSM_CMD_ATH,                                /*!< Disconnect Existing */
    GSM_CMD_ATI,                                /*!< Display Product Identification Information */
    GSM_CMD_ATL,                                /*!< Set Monitor speaker */
    GSM_CMD_ATM,                                /*!< Set Monitor Speaker Mode */
    GSM_CMD_PPP,                                /*!< Switch from Data Mode or PPP Online Mode to Command Mode, "+++" originally */
    GSM_CMD_ATO,                                /*!< Switch from Command Mode to Data Mode */
    GSM_CMD_ATP,                                /*!< Select Pulse Dialing */
    GSM_CMD_ATQ,                                /*!< Set Result Code Presentation Mode */
    GSM_CMD_ATS0,                               /*!< Set Number of Rings before Automatically Answering the Call */
    GSM_CMD_ATS3,                               /*!< Set Command Line Termination Character */
    GSM_CMD_ATS4,                               /*!< Set Response Formatting Character */
    GSM_CMD_ATS5,                               /*!< Set Command Line Editing Character */
    GSM_CMD_ATS6,                               /*!< Pause Before Blind */
    GSM_CMD_ATS7,                               /*!< Set Number of Seconds to Wait for Connection Completion */
    GSM_CMD_ATS8,                               /*!< Set Number of Seconds to Wait for Comma Dial Modifier Encountered in Dial String of D Command */
    GSM_CMD_ATS10,                              /*!< Set Disconnect Delay after Indicating the Absence of Data Carrier */
    GSM_CMD_ATT,                                /*!< Select Tone Dialing */
    GSM_CMD_ATV,                                /*!< TA Response Format */
    GSM_CMD_ATX,                                /*!< Set CONNECT Result Code Format and Monitor Call Progress */
    GSM_CMD_ATZ,                                /*!< Reset Default Configuration */
    GSM_CMD_AT_C,                               /*!< Set DCD Function Mode, AT&C */
    GSM_CMD_AT_D,                               /*!< Set DTR Function, AT&D */
    GSM_CMD_AT_F,                               /*!< Factory Defined Configuration, AT&F */
    GSM_CMD_AT_V,                               /*!< Display Current Configuration, AT&V */
    GSM_CMD_AT_W,                               /*!< Store Active Profile, AT&W */
    GSM_CMD_GCAP,                               /*!< Request Complete TA Capabilities List */
    GSM_CMD_GMI,                                /*!< Request Manufacturer Identification */
    GSM_CMD_GMM,                                /*!< Request TA Model Identification */
    GSM_CMD_GMR,                                /*!< Request TA Revision Identification of Software Release */
    GSM_CMD_GOI,                                /*!< Request Global Object Identification */
    GSM_CMD_GSN,                                /*!< Request TA Serial Number Identification (IMEI) */
    GSM_CMD_ICF,                                /*!< Set TE-TA Control Character Framing */
    GSM_CMD_IFC,                                /*!< Set TE-TA Local Data Flow Control */
    GSM_CMD_IPR,                                /*!< Set TE-TA Fixed Local Rate */
    GSM_CMD_HVOIC,                              /*!< Disconnect Voice Call Only */

    /*
     * AT commands according to 3GPP TS 27.007
     */
    GSM_CMD_COPS_SET,                           /*!< Set operator */
    GSM_CMD_COPS_GET,                           /*!< Get current operator */
    GSM_CMD_COPS_GET_OPT,                       /*!< Get a list of available operators */
    GSM_CMD_CPAS,                               /*!< Phone Activity Status */

    GSM_CMD_CACM,                               /*!< Accumulated Call Meter (ACM) Reset or Query */
    GSM_CMD_CAMM,                               /*!< Accumulated Call Meter Maximum (ACM max) Set or Query */
    GSM_CMD_CAOC,                               /*!< Advice of Charge */
    GSM_CMD_CBST,                               /*!< Select Bearer Service Type */
    GSM_CMD_CCFC,                               /*!< Call Forwarding Number and Conditions Control */
    GSM_CMD_CCWA,                               /*!< Call Waiting Control */
    GSM_CMD_CEER,                               /*!< Extended Error Report  */
    GSM_CMD_CGMI,                               /*!< Request Manufacturer Identification */
    GSM_CMD_CGMM,                               /*!< Request Model Identification */
    GSM_CMD_CGMR,                               /*!< Request TA Revision Identification of Software Release */
    GSM_CMD_CGSN,                               /*!< Request Product Serial Number Identification (Identical with +GSN) */
    GSM_CMD_CSCS,                               /*!< Select TE Character Set */
    GSM_CMD_CSTA,                               /*!< Select Type of Address */
    GSM_CMD_CHLD,                               /*!< Call Hold and Multiparty */
    GSM_CMD_CIMI,                               /*!< Request International Mobile Subscriber Identity */
    GSM_CMD_CLCC,                               /*!< List Current Calls of ME */
    GSM_CMD_CLCK,                               /*!< Facility Lock */
    GSM_CMD_CLIP,                               /*!< Calling Line Identification Presentation */
    GSM_CMD_CLIR,                               /*!< Calling Line Identification Restriction */
    GSM_CMD_CMEE,                               /*!< Report Mobile Equipment Error */
    GSM_CMD_COLP,                               /*!< Connected Line Identification Presentation */
#if GSM_CFG_PHONEBOOK || __DOXYGEN__
    GSM_CMD_PHONEBOOK_ENABLE,                   /*!< Top command to enable phonebook */
    GSM_CMD_CPBF,                               /*!< Find Phonebook Entries */
    GSM_CMD_CPBR,                               /*!< Read Current Phonebook Entries  */
    GSM_CMD_CPBS_SET,                           /*!< Select Phonebook Memory Storage */
    GSM_CMD_CPBS_GET,                           /*!< Get current Phonebook Memory Storage */
    GSM_CMD_CPBS_GET_OPT,                       /*!< Get available Phonebook Memory Storages */
    GSM_CMD_CPBW_SET,                           /*!< Write Phonebook Entry */
    GSM_CMD_CPBW_GET_OPT,                       /*!< Get options for write Phonebook Entry */
#endif /* GSM_CFG_PHONEBOOK || __DOXYGEN__ */
    GSM_CMD_SIM_PROCESS_BASIC_CMDS,             /*!< Command setup, executed when SIM is in READY state */
    GSM_CMD_CPIN_SET,                           /*!< Enter PIN */
    GSM_CMD_CPIN_GET,                           /*!< Read current SIM status */
    GSM_CMD_CPIN_ADD,                           /*!< Add new PIN to SIM if pin is not set */
    GSM_CMD_CPIN_CHANGE,                        /*!< Change already active SIM */
    GSM_CMD_CPIN_REMOVE,                        /*!< Remove current PIN */
    GMM_CMD_CPUK_SET,                           /*!< Enter PUK and set new PIN */

    GSM_CMD_CSQ_GET,                            /*!< Signal Quality Report */
    GSM_CMD_CFUN_SET,                           /*!< Set Phone Functionality */
    GSM_CMD_CFUN_GET,                           /*!< Get Phone Functionality */
    GSM_CMD_CREG_SET,                           /*!< Network Registration set output */
    GSM_CMD_CREG_GET,                           /*!< Get current network registration status */
    GSM_CMD_CBC,                                /*!< Battery Charge */
    GSM_CMD_CNUM,                               /*!< Subscriber Number */

    GSM_CMD_CPWD,                               /*!< Change Password */
    GSM_CMD_CR,                                 /*!< Service Reporting Control */
    GSM_CMD_CRC,                                /*!< Set Cellular Result Codes for Incoming Call Indication */
    GSM_CMD_CRLP,                               /*!< Select Radio Link Protocol Parameters  */
    GSM_CMD_CRSM,                               /*!< Restricted SIM Access */
    GSM_CMD_VTD,                                /*!< Tone Duration */
    GSM_CMD_VTS,                                /*!< DTMF and Tone Generation */
    GSM_CMD_CMUX,                               /*!< Multiplexer Control */
    GSM_CMD_CPOL,                               /*!< Preferred Operator List */
    GSM_CMD_COPN,                               /*!< Read Operator Names */
    GSM_CMD_CCLK,                               /*!< Clock */
    GSM_CMD_CSIM,                               /*!< Generic SIM Access */
    GSM_CMD_CALM,                               /*!< Alert Sound Mode */
    GSM_CMD_CALS,                               /*!< Alert Sound Select */
    GSM_CMD_CRSL,                               /*!< Ringer Sound Level */
    GSM_CMD_CLVL,                               /*!< Loud Speaker Volume Level */
    GSM_CMD_CMUT,                               /*!< Mute Control */
    GSM_CMD_CPUC,                               /*!< Price Per Unit and Currency Table */
    GSM_CMD_CCWE,                               /*!< Call Meter Maximum Event */
    GSM_CMD_CUSD,                               /*!< Unstructured Supplementary Service Data108 */
    GSM_CMD_CSSN,                               /*!< Supplementary Services Notification 109 */
#if GSM_CFG_CONN || __DOXYGEN__
    GSM_CMD_CIPMUX,                             /*!< Start Up Multi-IP Connection */
    GSM_CMD_CIPSTART,                           /*!< Start Up TCP or UDP Connection */
    GSM_CMD_CIPSEND,                            /*!< Send Data Through TCP or UDP Connection */
    GSM_CMD_CIPQSEND,                           /*!< Select Data Transmitting Mode */
    GSM_CMD_CIPACK,                             /*!< Query Previous Connection Data Transmitting State */
    GSM_CMD_CIPCLOSE,                           /*!< Close TCP or UDP Connection */
    GSM_CMD_CIPSHUT,                            /*!< Deactivate GPRS PDP Context */
    GSM_CMD_CLPORT,                             /*!< Set Local Port */
    GSM_CMD_CSTT,                               /*!< Start Task and Set APN, username, password */
    GSM_CMD_CIICR,                              /*!< Bring Up Wireless Connection with GPRS or CSD */
    GSM_CMD_CIFSR,                              /*!< Get Local IP Address */
    GSM_CMD_CIPSTATUS,                          /*!< Query Current Connection Status */
    GSM_CMD_CDNSCFG,                            /*!< Configure Domain Name Server */
    GSM_CMD_CDNSGIP,                            /*!< Query the IP Address of Given Domain Name */
    GSM_CMD_CIPHEAD,                            /*!< Add an IP Head at the Beginning of a Package Received */
    GSM_CMD_CIPATS,                             /*!< Set Auto Sending Timer */
    GSM_CMD_CIPSPRT,                            /*!< Set Prompt of ‘>’ When Module Sends Data */
    GSM_CMD_CIPSERVER,                          /*!< Configure Module as Server */
    GSM_CMD_CIPCSGP,                            /*!< Set CSD or GPRS for Connection Mode */
    GSM_CMD_CIPSRIP,                            /*!< Show Remote IP Address and Port When Received Data */
    GSM_CMD_CIPDPDP,                            /*!< Set Whether to Check State of GPRS Network Timing */
    GSM_CMD_CIPMODE,                            /*!< Select TCPIP Application Mode */
    GSM_CMD_CIPCCFG,                            /*!< Configure Transparent Transfer Mode */
    GSM_CMD_CIPSHOWTP,                          /*!< Display Transfer Protocol in IP Head When Received Data */
    GSM_CMD_CIPUDPMODE,                         /*!< UDP Extended Mode */
    GSM_CMD_CIPRXGET,                           /*!< Get Data from Network Manually */
    GSM_CMD_CIPSCONT,                           /*!< Save TCPIP Application Context */
    GSM_CMD_CIPRDTIMER,                         /*!< Set Remote Delay Timer */
    GSM_CMD_CIPSGTXT,                           /*!< Select GPRS PDP context */
    GSM_CMD_CIPTKA,                             /*!< Set TCP Keepalive Parameters */
#endif /* GSM_CFG_CONN || __DOXYGEN__ */
#if GSM_CFG_CALL || __DOXYGEN__
    GSM_CMD_CALL_ENABLE,                        /*!< Top command to enable call */
#endif /* GSM_CFG_CALL || __DOXYGEN__ */
#if GSM_CFG_SMS || __DOXYGEN__
    GSM_CMD_SMS_ENABLE,                         /*!< Top command to enable SMS */
    GSM_CMD_CMGD,                               /*!< Delete SMS Message */
    GSM_CMD_CMGF,                               /*!< Select SMS Message Format */
    GSM_CMD_CMGL,                               /*!< List SMS Messages from Preferred Store */
    GSM_CMD_CMGR,                               /*!< Read SMS Message */
    GSM_CMD_CMGS,                               /*!< Send SMS Message */
    GSM_CMD_CMGW,                               /*!< Write SMS Message to Memory */
    GSM_CMD_CMSS,                               /*!< Send SMS Message from Storage */
    GSM_CMD_CNMI,                               /*!< New SMS Message Indications */
    GSM_CMD_CPMS_SET,                           /*!< Set preferred SMS Message Storage */
    GSM_CMD_CPMS_GET,                           /*!< Get preferred SMS Message Storage */
    GSM_CMD_CPMS_GET_OPT,                       /*!< Get optional SMS message storages */
    GSM_CMD_CRES,                               /*!< Restore SMS Settings */
    GSM_CMD_CSAS,                               /*!< Save SMS Settings */
    GSM_CMD_CSCA,                               /*!< SMS Service Center Address */
    GSM_CMD_CSCB,                               /*!< Select Cell Broadcast SMS Messages */
    GSM_CMD_CSDH,                               /*!< Show SMS Text Mode Parameters */
    GSM_CMD_CSMP,                               /*!< Set SMS Text Mode Parameters */
    GSM_CMD_CSMS,                               /*!< Select Message Service */
#endif /* GSM_CFG_SMS || __DOXYGEN__ */

    GSM_CMD_END,                                /*!< Last CMD entry */
} gsm_cmd_t;

#if GSM_CFG_CONN || __DOXYGEN__

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

#endif /* GSM_CFG_CONN || __DOXYGEN__ */

/**
 * \ingroup         GSM_PBUF
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
 * \brief           Message queue structure to share between threads
 */
typedef struct gsm_msg {
    gsm_cmd_t       cmd_def;                    /*!< Default message type received from queue */
    gsm_cmd_t       cmd;                        /*!< Since some commands can have different subcommands, sub command is used here */
    uint8_t         i;                          /*!< Variable to indicate order number of subcommands */
    gsm_sys_sem_t   sem;                        /*!< Semaphore for the message */
    uint8_t         is_blocking;                /*!< Status if command is blocking */
    uint8_t         is_device;                  /*!< Is message device specific? */
    uint32_t        block_time;                 /*!< Maximal blocking time in units of milliseconds. Use 0 to for non-blocking call */
    gsmr_t          res;                        /*!< Result of message operation */
    gsmr_t          (*fn)(struct gsm_msg *);    /*!< Processing callback function to process packet */
    gsmr_t          (*sub_fn)(struct gsm_msg *, uint8_t is_ok, uint16_t is_error);  /*!< Sub command function call */
    union {
        struct {
            uint32_t delay;                     /*!< Delay to use before sending first reset AT command */
        } reset;
        struct {
            uint32_t baudrate;                  /*!< Baudrate for AT port */
        } uart;

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
            int16_t* rssi;                      /*!< Pointer to RSSI variable */
        } csq;                                  /*!< Signal strength */
        struct {
            uint8_t read;                       /*!< Flag indicating we can read the COPS actual data */
            gsm_operator_t* ops;                /*!< Pointer to operators array */
            size_t opsl;                        /*!< Length of operators array */
            size_t opsi;                        /*!< Current operator index array */
            size_t* opf;                        /*!< Pointer to number of operators found */
        } cops_scan;                            /*!< Scan operators */
        struct {
            gsm_operator_curr_t* curr;          /*!< Pointer to output current operator */
        } cops_get;                             /*!< Get current operator info */
        struct {
            gsm_operator_mode_t mode;           /*!< COPS mode */
            gsm_operator_format_t format;       /*!< Operator format to print */
            const char* name;                   /*!< Short or long name, according to format */
            uint32_t num;                       /*!< Number in case format is number */
        } cops_set;

#if GSM_CFG_SMS || __DOXYGEN__
        struct {
            const char* num;                    /*!< Phone number */
            const char* text;                   /*!< SMS content to send */
            uint8_t format;                     /*!< SMS format, `0 = PDU`, `1 = text` */
        } sms_send;                             /*!< Send SMS */
        struct {
            gsm_mem_t mem;                      /*!< Memory to read from */
            size_t pos;                         /*!< SMS position in memory */
            gsm_sms_entry_t* entry;             /*!< Pointer to entry to write info */
            uint8_t update;                     /*!< Update SMS status after read operation */
            uint8_t format;                     /*!< SMS format, `0 = PDU`, `1 = text` */
            uint8_t read;                       /*!< Read the data flag */
        } sms_read;                             /*!< Read single SMS */
        struct {
            gsm_mem_t mem;                      /*!< Memory to delete from */
            size_t pos;                         /*!< SMS position in memory */
        } sms_delete;                           /*!< Delete SMS message */
        struct {
            gsm_mem_t mem;                      /*!< Memory to use for read */
            gsm_sms_status_t status;            /*!< SMS entries status */
            gsm_sms_entry_t* entries;           /*!< Pointer to entries */
            size_t etr;                         /*!< Entries to read (array length) */
            size_t ei;                          /*!< Current entry index in array */
            size_t* er;                         /*!< Final entries read pointer for user */
            uint8_t update;                     /*!< Update SMS status after read operation */
            uint8_t format;                     /*!< SMS format, `0 = PDU`, `1 = text` */
            uint8_t read;                       /*!< Read the data flag */
        } sms_list;                             /*!< List SMS messages */
        struct {
            gsm_mem_t mem[3];                   /*!< Array of memories */
        } sms_memory;                           /*!< Set preferred memories */
#endif /* GSM_CFG_SMS || __DOXYGEN__ */
#if GSM_CFG_CALL || __DOXYGEN__
        struct {
            const char* number;                 /*!< Phone number to dial */
        } call_start;                           /*!< Start a new call */
#endif /* GSM_CFG_CALL || __DOXYGEN__ */
#if GSM_CFG_PHONEBOOK || __DOXYGEN__
        struct {
            gsm_mem_t mem;                      /*!< Memory to use */
            size_t pos;                         /*!< Memory position. Set to 0 to use new one or SIZE_T MAX to delete entry */
            const char* name;                   /*!< Entry name */
            const char* num;                    /*!< Entry number */
            gsm_number_type_t type;             /*!< Entry phone number type */
            uint8_t del;                        /*!< Flag indicates delete */
        } pb_write;                             /*!< Write/Edit/Delete entry */
        struct {
            gsm_mem_t mem;                      /*!< Memory to use */
            size_t start_index;                 /*!< Start index in phonebook to read */
            gsm_pb_entry_t* entries;            /*!< Pointer to entries array */
            size_t etr;                         /*!< NUmber of entries to read */
            size_t ei;                          /*!< Current entry index */
            size_t* er;                         /*!< Final entries read pointer for user */
        } pb_list;                              /*!< List phonebook entries */
        struct {
            gsm_mem_t mem;                      /*!< Memory to use */
            gsm_pb_entry_t* entries;            /*!< Pointer to entries array */
            size_t etr;                         /*!< NUmber of entries to read */
            size_t ei;                          /*!< Current entry index */
            size_t* er;                         /*!< Final entries read pointer for user */
            const char* search;                 /*!< Search string */
        } pb_search;                            /*!< Search phonebook entries */
#endif /* GSM_CFG_PHONEBOOK || __DOXYGEN__ */
#if GSM_CFG_NETWORK || __DOXYGEN__ 
        struct {
            const char* apn;
            const char* user;
            const char* pass;
        } network_attach;
#endif /* GSM_CFG_NETWORK || __DOXYGEN__ */
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
 * \brief           SMS memory information
 */
typedef struct {
    uint32_t mem_available;                     /*!< Bit field of available memories */
    gsm_mem_t current;                          /*!< Current memory choice */
    size_t total;                               /*!< Size of memory in units of entries */
    size_t used;                                /*!< Number of used entries */
} gsm_sms_mem_t;

/**
 * \brief           SMS structure
 */
typedef struct {
    uint8_t enabled;                            /*!< Flag indicating feature enabled */
    gsm_sms_mem_t mem[3];                       /*!< 3 memory info for operation,receive,sent storage */
} gsm_sms_t;

/**
 * \brief           SMS memory information
 */
typedef struct {
    uint32_t mem_available;                     /*!< Bit field of available memories */
    gsm_mem_t current;                          /*!< Current memory choice */
    size_t total;                               /*!< Size of memory in units of entries */
    size_t used;                                /*!< Number of used entries */
} gsm_pb_mem_t;

/**
 * \brief           Phonebook structure
 */
typedef struct {
    uint8_t enabled;                            /*!< Flag indicating feature enabled */
    gsm_pb_mem_t mem;                           /*!< Memory information */
} gsm_pb_t;

/**
 * \brief           Network info
 */
typedef struct {
    gsm_network_reg_status_t status;            /*!< Network registration status */
    gsm_operator_curr_t curr_operator;          /*!< Current operator information */
} gsm_network_t;

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

    gsm_sim_state_t     sim_state;              /*!< SIM current state */
    gsm_network_t       network;                /*!< Network status */
    
    gsm_conn_t          conns[GSM_CFG_MAX_CONNS];   /*!< Array of all connection structures */
    
    gsm_link_conn_t     link_conn;              /*!< Link connection handle */
    //gsm_ipd_t           ipd;                    /*!< Incoming data structure */
    gsm_cb_t            cb;                     /*!< Callback processing structure */
    
    gsm_cb_func_t*      cb_func;                /*!< Callback function linked list */

    int16_t             rssi;                   /*!< RSSI signal strength. `0` = invalid, `-53 % -113` = valid */
#if GSM_CFG_SMS || __DOXYGEN__
    gsm_sms_t           sms;                    /*!< SMS information */
#endif /* GSM_CFG_SMS || __DOXYGEN__ */ 
#if GSM_CFG_PHONEBOOK || __DOXYGEN__
    gsm_pb_t            pb;                     /*!< Phonebook information */
#endif /* GSM_CFG_PHONEBOOK || __DOXYGEN__ */
#if GSM_CFG_CALL || __DOXYGEN__
    gsm_call_t          call;                   /*!< Call information */
#endif /* GSM_CFG_CALL || __DOXYGEN__ */ 
    union {
        struct {
            uint8_t     initialized:1;          /*!< Flag indicating GSM library is initialized */
            uint8_t     dev_present:1;          /*!< Flag indicating GSM device is present */
#if GSM_CFG_SMS || __DOXYGEN__
            uint8_t     sms_ready:1;            /*!< Flag indicating SMS system is ready */
#endif /* GSM_CFG_SMS || __DOXYGEN__ */ 
#if GSM_CFG_CALL || __DOXYGEN__
            uint8_t     call_ready:1;           /*!< Flag indicating CALL system is ready */
#endif /* GSM_CFG_CALL || __DOXYGEN__ */ 
        } f;                                    /*!< Flags structure */
    } status;                                   /*!< Status structure */
    
    uint8_t conn_val_id;                        /*!< Validation ID increased each time device connects to network */
} gsm_t;

/**
 * \brief           Memory mapping structure between string and value in app
 */
typedef struct {
    gsm_mem_t mem;                              /*!< Mem indication */
    const char* mem_str;                        /*!< Memory string */
} gsm_dev_mem_map_t;

/**
 * \brief           Unicode support structure
 */
typedef struct {
    uint8_t ch[4];                              /*!< UTF-8 max characters */
    uint8_t t;                                  /*!< Total expected length in UTF-8 sequence */
    uint8_t r;                                  /*!< Remaining bytes in UTF-8 sequence */
    gsmr_t res;                                 /*!< Current result of processing */
} gsm_unicode_t;

/**
 * \}
 */

/**
 * \ingroup         GSM_DEVICE
 * \name            GSM_DEVICE_FEATURES Device features
 * \anchor          GSM_DEVICE_FEATURES
 * \{
 */

#define GSM_DEVICE_FEATURE_SMS                  GSM_U16(0x0001) /*!< SMS feature */
#define GSM_DEVICE_FEATURE_CALL                 GSM_U16(0x0002) /*!< Phone book feature */
#define GSM_DEVICE_FEATURE_PB                   GSM_U16(0x0004) /*!< Call feature */
#define GSM_DEVICE_FEATURE_TCPIP                GSM_U16(0x0004) /*!< TCP/IP raw connections */

/**
 * \}
 */

#if !__DOXYGEN__
/**
 * \ingroup         GSM_PRIVATE
 * \brief           Receive character structure to handle full line terminated with `\n` character
 */
typedef struct {
    char data[128];                             /*!< Received characters */
    uint8_t len;                                /*!< Length of line */
} gsm_recv_t;
#endif

/**
 * \ingroup         GSM_DEVICE
 * \brief           Device driver structure
 */
typedef struct gsm_device_driver {
    uint16_t features;                          /*!< List of supported features by device driver */

    /**
     * Prototype function to send AT command string to modem
     * 
     * \param[in]       msg: Current active message
     * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
     */
    gsmr_t (*at_start_cmd_fn)   (gsm_msg_t* msg);

    /**
     * New line of data received over AT port
     *
     * \param[in]       recv: Received line of data
     * \param[in,out]   is_ok: Pointer to status if operation is ok. User may modify this status
     * \param[in,out]   is_error: Pointer to status if operation error occurred. User may modify this status
     * \return          `1` if message eaten or `0` if ignored
     */
    uint8_t (*at_line_recv_fn)  (gsm_recv_t* recv, uint8_t* is_ok, uint16_t* is_error);

    /**
     * Process sub command
     *
     * \param[in]       msg: Current message
     * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
     */
    gsmr_t (*at_process_sub_cmd_fn)   (gsm_msg_t* msg, uint8_t is_ok, uint16_t is_error);
} gsm_device_driver_t;

#if !__DOXYGEN__
/**
 * \ingroup         GSM
 * \defgroup        GSM_PRIVATE Internal functions
 * \brief           functions, structures and enumerations
 * \{
 */

extern gsm_t                gsm;
extern gsm_device_driver_t  gsm_device;

extern gsm_dev_mem_map_t    gsm_dev_mem_map[];
extern size_t               gsm_dev_mem_map_size;

#define CMD_IS_CUR(c)       (gsm.msg != NULL && gsm.msg->cmd == (c))
#define CMD_IS_DEF(c)       (gsm.msg != NULL && gsm.msg->cmd_def == (c))
#define CMD_GET_CUR()       ((gsm_cmd_t)(((gsm.msg != NULL) ? gsm.msg->cmd : GSM_CMD_IDLE)))
#define CMD_GET_DEF()       ((gsm_cmd_t)(((gsm.msg != NULL) ? gsm.msg->cmd_def : GSM_CMD_IDLE)))

#define CRLF                "\r\n"
#define CRLF_LEN            2

#define GSM_MSG_VAR_DEFINE(name)                gsm_msg_t* name
#define GSM_MSG_VAR_ALLOC(name)                 do {\
    (name) = gsm_mem_alloc(sizeof(*(name)));          \
    GSM_DEBUGW(GSM_CFG_DBG_VAR | GSM_DBG_TYPE_TRACE, (name) != NULL, "MSG VAR: Allocated %d bytes at %p\r\n", sizeof(*(name)), (name)); \
    GSM_DEBUGW(GSM_CFG_DBG_VAR | GSM_DBG_TYPE_TRACE, (name) == NULL, "MSG VAR: Error allocating %d bytes\r\n", sizeof(*(name))); \
    if (!(name)) {                                  \
        return gsmERRMEM;                           \
    }                                               \
    memset(name, 0x00, sizeof(*(name)));            \
} while (0)
#define GSM_MSG_VAR_REF(name)                   (*(name))
#define GSM_MSG_VAR_FREE(name)                  do {\
    GSM_DEBUGF(GSM_CFG_DBG_VAR | GSM_DBG_TYPE_TRACE, "MSG VAR: Free memory: %p\r\n", (name)); \
    gsm_mem_free(name);                             \
    (name) = NULL;                                  \
} while (0)

#define GSM_CHARISNUM(x)                    ((x) >= '0' && (x) <= '9')
#define GSM_CHARTONUM(x)                    ((x) - '0')
#define GSM_CHARISHEXNUM(x)                 (((x) >= '0' && (x) <= '9') || ((x) >= 'a' && (x) <= 'f') || ((x) >= 'A' && (x) <= 'F'))
#define GSM_CHARHEXTONUM(x)                 (((x) >= '0' && (x) <= '9') ? ((x) - '0') : (((x) >= 'a' && (x) <= 'f') ? ((x) - 'a' + 10) : (((x) >= 'A' && (x) <= 'F') ? ((x) - 'A' + 10) : 0)))
#define GSM_ISVALIDASCII(x)                 (((x) >= 32 && (x) <= 126) || (x) == '\r' || (x) == '\n')

#define RECV_ADD(ch)        do { recv_buff.data[recv_buff.len++] = ch; recv_buff.data[recv_buff.len] = 0; } while (0)
#define RECV_RESET()        do { recv_buff.len = 0; recv_buff.data[0] = 0; } while (0)
#define RECV_LEN()          recv_buff.len
#define RECV_IDX(index)     recv_buff.data[index]

#define GSM_AT_PORT_SEND_BEGIN()        do { GSM_AT_PORT_SEND_STR("AT"); } while (0)
#define GSM_AT_PORT_SEND_END()          do { GSM_AT_PORT_SEND_STR(CRLF); } while (0)

#define GSM_AT_PORT_SEND_STR(str)       gsm.ll.send_fn((const uint8_t *)(str), (uint16_t)strlen(str))
#define GSM_AT_PORT_SEND_CHR(ch)        gsm.ll.send_fn((const uint8_t *)(ch), (uint16_t)1)
#define GSM_AT_PORT_SEND(d, l)          gsm.ll.send_fn((const uint8_t *)(d), (uint16_t)(l))

#define GSM_AT_PORT_SEND_QUOTE_COND(q)  do { if ((q)) { GSM_AT_PORT_SEND_STR("\""); } } while (0)
#define GSM_AT_PORT_SEND_COMMA_COND(c)  do { if ((c)) { GSM_AT_PORT_SEND_STR(","); } } while (0)
#define GSM_AT_PORT_SEND_EQUAL_COND(e)  do { if ((e)) { GSM_AT_PORT_SEND_STR("="); } } while (0)

#define GSM_AT_PORT_SEND_CTRL_Z()       GSM_AT_PORT_SEND_STR("\x1A")
#define GSM_AT_PORT_SEND_ESC()          GSM_AT_PORT_SEND_STR("\x1B")

#define GSM_PORT2NUM(port)              ((uint32_t)(port))

/**
 * \brief           Protect (count up) OS protection (mutex)
 */
#define GSM_CORE_PROTECT()                  gsm_sys_protect()

/**
 * \brief           Unprotect (count down) OS protection (mutex)
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
gsmr_t      gsmi_send_device_msg_to_producer_mbox(gsm_msg_t* msg, uint32_t block, uint32_t max_block_time);
uint32_t    gsmi_get_from_mbox_with_timeout_checks(gsm_sys_mbox_t* b, void** m, uint32_t timeout);

gsmr_t      gsmi_get_sim_info(uint32_t blocking);

/* Send functions */
void        byte_to_str(uint8_t num, char* str);
void        number_to_str(uint32_t num, char* str);
void        signed_number_to_str(int32_t num, char* str);

/* Send functions */
void        send_ip_mac(const void* d, uint8_t is_ip, uint8_t q, uint8_t c);
void        send_string(const char* str, uint8_t e, uint8_t q, uint8_t c);
void        send_number(uint32_t num, uint8_t q, uint8_t c);
void        send_port(gsm_port_t port, uint8_t q, uint8_t c);
void        send_signed_number(int32_t num, uint8_t q, uint8_t c);
void        send_dev_memory(gsm_mem_t mem, uint8_t q, uint8_t c);

/**
 * \}
 */

#endif /* !__DOXYGEN__ */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GSM_PRIV_H */
