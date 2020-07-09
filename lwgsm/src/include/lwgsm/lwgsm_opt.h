/**
 * \file            lwgsm_opt.h
 * \brief           GSM-AT options
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
 * This file is part of GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         $_version_$
 */
#ifndef GSM_HDR_OPT_H
#define GSM_HDR_OPT_H

/* Include application options */
#include "lwgsm_opts.h"

/**
 * \defgroup        GSM_OPT Configuration
 * \brief           GSM-AT options
 * \{
 *
 */

/**
 * \brief           Enables `1` or disables `0` operating system support for GSM library
 *
 * \note            Value must be set to 1 in the current revision
 *
 * \note            Check \ref GSM_OPT_OS group for more configuration related to operating system
 *
 */
#ifndef GSM_CFG_OS
#define GSM_CFG_OS                          1
#endif

/**
 * \brief           Enables `1` or disables `0` custom memory management functions
 *
 * When set to `1`, \ref GSM_MEM block must be provided manually.
 * This includes implementation of functions \ref lwgsm_mem_malloc,
 * \ref lwgsm_mem_calloc, \ref lwgsm_mem_realloc and \ref lwgsm_mem_free
 *
 * \note            Function declaration follows standard C functions `malloc, calloc, realloc, free`.
 *                  Declaration is available in `gsm/lwgsm_mem.h` file. Include this file to final
 *                  implementation file
 *
 * \note            When implementing custom memory allocation, it is necessary
 *                  to take care of multiple threads accessing same resource for custom allocator
 */
#ifndef GSM_CFG_MEM_CUSTOM
#define GSM_CFG_MEM_CUSTOM                  0
#endif

/**
 * \brief           Memory alignment for dynamic memory allocations
 *
 * \note            Some CPUs can work faster if memory is aligned, usually to 4 or 8 bytes.
 *                  To speed up this possibilities, you can set memory alignment and library
 *                  will try to allocate memory on aligned boundaries.
 *
 * \note            Some CPUs such ARM Cortex-M0 don't support unaligned memory access.
 *                  This CPUs must have set correct memory alignment value.
 *
 * \note            This value must be power of `2`
 */
#ifndef GSM_CFG_MEM_ALIGNMENT
#define GSM_CFG_MEM_ALIGNMENT               4
#endif

/**
 * \brief           Enables `1` or disables `0` callback function and custom parameter for API functions
 *
 * When enabled, `2` additional parameters are available in API functions.
 * When command is executed, callback function with its parameter could be called when not set to `NULL`.
 */
#ifndef GSM_CFG_USE_API_FUNC_EVT
#define GSM_CFG_USE_API_FUNC_EVT            1
#endif

/**
 * \brief           Maximal number of connections AT software can support on GSM device
 *
 */
#ifndef GSM_CFG_MAX_CONNS
#define GSM_CFG_MAX_CONNS                   6
#endif

/**
 * \brief           Maximal number of bytes we can send at single command to GSM
 * \note            Value can not exceed `1460` bytes or no data will be ever send
 *
 * \note            This is limitation of GSM AT commands and on systems where RAM
 *                  is not an issue, it should be set to maximal value (`1460`)
 *                  to optimize data transfer speed performance
 */
#ifndef GSM_CFG_CONN_MAX_DATA_LEN
#define GSM_CFG_CONN_MAX_DATA_LEN           1460
#endif

/**
 * \brief           Set number of retries for send data command.
 *
 * Sometimes it may happen that `AT+SEND` command fails due to different problems.
 * Trying to send the same data multiple times can raise chances we are successful.
 */
#ifndef GSM_CFG_MAX_SEND_RETRIES
#define GSM_CFG_MAX_SEND_RETRIES            3
#endif

/**
 * \brief           Maximum single buffer size for network receive data (TCP/UDP connections)
 *
 * \note            When GSM sends buffer buffer than maximal, multiple buffers are created
 */
#ifndef GSM_CFG_IPD_MAX_BUFF_SIZE
#define GSM_CFG_IPD_MAX_BUFF_SIZE           1460
#endif

/**
 * \brief           Default baudrate used for AT port
 *
 * \note            Later, user may call API function to change to desired baudrate if necessary
 */
#ifndef GSM_CFG_AT_PORT_BAUDRATE
#define GSM_CFG_AT_PORT_BAUDRATE            115200
#endif

/**
 * \brief           Buffer size for received data waiting to be processed
 * \note            When server mode is active and a lot of connections are in queue
 *                  this should be set high otherwise your buffer may overflow
 *
 * \note            Buffer size also depends on TX user driver if it uses DMA or blocking mode
 *                  In case of DMA (CPU can work other tasks), buffer may be smaller as CPU
 *                  will have more time to process all the incoming bytes
 *
 * \note            This parameter has no meaning when \ref GSM_CFG_INPUT_USE_PROCESS is enabled
 */
#ifndef GSM_CFG_RCV_BUFF_SIZE
#define GSM_CFG_RCV_BUFF_SIZE               0x400
#endif

/**
 * \brief           Enables `1` or disables `0` reset sequence after \ref lwgsm_init call
 *
 * \note            When this functionality is disabled, user must manually call \ref lwgsm_reset to send
 *                  reset sequence to GSM device.
 */
#ifndef GSM_CFG_RESET_ON_INIT
#define GSM_CFG_RESET_ON_INIT               1
#endif

/**
 * \brief           Enables `1` or disables `0` reset sequence after \ref lwgsm_device_set_present call
 *
 * \note            When this functionality is disabled, user must manually call \ref lwgsm_reset to send
 *                  reset sequence to GSM device.
 */
#ifndef GSM_CFG_RESET_ON_DEVICE_PRESENT
#define GSM_CFG_RESET_ON_DEVICE_PRESENT     1
#endif


/**
 * \brief           Default delay (milliseconds unit) before sending first AT command on reset sequence
 */
#ifndef GSM_CFG_RESET_DELAY_DEFAULT
#define GSM_CFG_RESET_DELAY_DEFAULT         1000
#endif

/**
 * \defgroup        GSM_OPT_DBG Debugging
 * \brief           Debugging configurations
 * \{
 */

/**
 * \brief           Set global debug support.
 *
 * Possible values are \ref GSM_DBG_ON or \ref GSM_DBG_OFF
 *
 * \note            Set to \ref GSM_DBG_OFF to globally disable all debugs
 */
#ifndef GSM_CFG_DBG
#define GSM_CFG_DBG                         GSM_DBG_OFF
#endif

/**
 * \brief           Debugging output function
 *
 *                  Called with format and optional parameters for printf style debug
 */
#ifndef GSM_CFG_DBG_OUT
#define GSM_CFG_DBG_OUT(fmt, ...)           do { extern int printf( const char * format, ... ); printf(fmt, ## __VA_ARGS__); } while (0)
#endif

/**
 * \brief           Minimal debug level
 *
 *                  Check \ref GSM_DBG_LVL for possible values
 */
#ifndef GSM_CFG_DBG_LVL_MIN
#define GSM_CFG_DBG_LVL_MIN                 GSM_DBG_LVL_ALL
#endif

/**
 * \brief           Enabled debug types
 *
 * When debug is globally enabled with \ref GSM_CFG_DBG parameter,
 * user must enable debug types such as TRACE or STATE messages.
 *
 * Check \ref GSM_DBG_TYPE for possible options. Separate values with `bitwise OR` operator
 */
#ifndef GSM_CFG_DBG_TYPES_ON
#define GSM_CFG_DBG_TYPES_ON                0
#endif

/**
 * \brief           Set debug level for init function
 *
 * Possible values are \ref GSM_DBG_ON or \ref GSM_DBG_OFF
 */
#ifndef GSM_CFG_DBG_INIT
#define GSM_CFG_DBG_INIT                    GSM_DBG_OFF
#endif

/**
 * \brief           Set debug level for memory manager
 *
 * Possible values are \ref GSM_DBG_ON or \ref GSM_DBG_OFF
 */
#ifndef GSM_CFG_DBG_MEM
#define GSM_CFG_DBG_MEM                     GSM_DBG_OFF
#endif

/**
 * \brief           Set debug level for input module
 *
 * Possible values are \ref GSM_DBG_ON or \ref GSM_DBG_OFF
 */
#ifndef GSM_CFG_DBG_INPUT
#define GSM_CFG_DBG_INPUT                   GSM_DBG_OFF
#endif

/**
 * \brief           Set debug level for GSM threads
 *
 * Possible values are \ref GSM_DBG_ON or \ref GSM_DBG_OFF
 */
#ifndef GSM_CFG_DBG_THREAD
#define GSM_CFG_DBG_THREAD                  GSM_DBG_OFF
#endif

/**
 * \brief           Set debug level for asserting of input variables
 *
 * Possible values are \ref GSM_DBG_ON or \ref GSM_DBG_OFF
 */
#ifndef GSM_CFG_DBG_ASSERT
#define GSM_CFG_DBG_ASSERT                  GSM_DBG_OFF
#endif

/**
 * \brief           Set debug level for incoming data received from device
 *
 * Possible values are \ref GSM_DBG_ON or \ref GSM_DBG_OFF
 */
#ifndef GSM_CFG_DBG_IPD
#define GSM_CFG_DBG_IPD                     GSM_DBG_OFF
#endif

/**
 * \brief           Set debug level for packet buffer manager
 *
 * Possible values are \ref GSM_DBG_ON or \ref GSM_DBG_OFF
 */
#ifndef GSM_CFG_DBG_PBUF
#define GSM_CFG_DBG_PBUF                    GSM_DBG_OFF
#endif

/**
 * \brief           Set debug level for connections
 *
 * Possible values are \ref GSM_DBG_ON or \ref GSM_DBG_OFF
 */
#ifndef GSM_CFG_DBG_CONN
#define GSM_CFG_DBG_CONN                    GSM_DBG_OFF
#endif

/**
 * \brief           Set debug level for dynamic variable allocations
 *
 * Possible values are \ref GSM_DBG_ON or \ref GSM_DBG_OFF
 */
#ifndef GSM_CFG_DBG_VAR
#define GSM_CFG_DBG_VAR                     GSM_DBG_OFF
#endif

/**
 * \brief           Set debug level for netconn sequential API
 *
 * Possible values are \ref GSM_DBG_ON or \ref GSM_DBG_OFF
 */
#ifndef GSM_CFG_DBG_NETCONN
#define GSM_CFG_DBG_NETCONN                 GSM_DBG_OFF
#endif

/**
 * \brief           Enables `1` or disables `0` echo mode on AT commands
 *                  sent to GSM device.
 *
 * \note            This mode is useful when debugging GSM communication
 */
#ifndef GSM_CFG_AT_ECHO
#define GSM_CFG_AT_ECHO                     0
#endif

/**
 * \}
 */

/**
 * \defgroup        GSM_OPT_OS OS configuration
 * \brief           Operating system dependant configuration
 * \{
 */

/**
 * \brief           Set number of message queue entries for procuder thread
 *
 * Message queue is used for storing memory address to command data
 */
#ifndef GSM_CFG_THREAD_PRODUCER_MBOX_SIZE
#define GSM_CFG_THREAD_PRODUCER_MBOX_SIZE   16
#endif

/**
 * \brief           Set number of message queue entries for processing thread
 *
 * Message queue is used to notify processing thread about new received data on AT port
 */
#ifndef GSM_CFG_THREAD_PROCESS_MBOX_SIZE
#define GSM_CFG_THREAD_PROCESS_MBOX_SIZE    16
#endif

/**
 * \brief           Enables `1` or disables `0` direct support for processing input data
 *
 * When this mode is enabled, no overhead is included for copying data
 * to receive buffer because bytes are processed directly.
 *
 * \note            This mode can only be used when \ref GSM_CFG_OS is enabled
 *
 * \note            When using this mode, separate thread must be dedicated only
 *                  for reading data on AT port
 *
 * \note            Best case for using this mode is if DMA receive is supported by host device
 */
#ifndef GSM_CFG_INPUT_USE_PROCESS
#define GSM_CFG_INPUT_USE_PROCESS           0
#endif

/**
 * \brief           Producer thread hook, called each time thread wakes-up and does the processing.
 *
 * It can be used to check if thread is alive.
 */
#ifndef GSM_THREAD_PRODUCER_HOOK
#define GSM_THREAD_PRODUCER_HOOK()
#endif

/**
 * \brief           Process thread hook, called each time thread wakes-up and does the processing.
 *
 * It can be used to check if thread is alive.
 */
#ifndef GSM_THREAD_PROCESS_HOOK
#define GSM_THREAD_PROCESS_HOOK()
#endif

/**
 * \}
 */

/**
 * \defgroup        GSM_OPT_MODULES Modules
 * \brief           Configuration of specific modules
 * \{
 */

/**
 * \defgroup        GSM_OPT_MODULES_NETCONN Netconn module
 * \brief           Configuration of netconn API module
 * \{
 */

/**
 * \brief           Enables `1` or disables `0` NETCONN sequential API support for OS systems
 *
 * \note            To use this feature, OS support is mandatory.
 * \sa              GSM_CFG_OS
 */
#ifndef GSM_CFG_NETCONN
#define GSM_CFG_NETCONN                     0
#endif

/**
 * \brief           Enables `1` or disables `0` receive timeout feature
 *
 * When this option is enabled, user will get an option
 * to set timeout value for receive data on netconn,
 * before function returns timeout error.
 *
 * \note            Even if this option is enabled, user must still manually set timeout,
 *                  by default time will be set to 0 which means no timeout.
 */
#ifndef GSM_CFG_NETCONN_RECEIVE_TIMEOUT
#define GSM_CFG_NETCONN_RECEIVE_TIMEOUT     1
#endif

/**
 * \brief           Accept queue length for new client when netconn server is used
 *
 * Defines number of maximal clients waiting in accept queue of server connection
 */
#ifndef GSM_CFG_NETCONN_ACCEPT_QUEUE_LEN
#define GSM_CFG_NETCONN_ACCEPT_QUEUE_LEN    5
#endif

/**
 * \brief           Receive queue length for pbuf entries
 *
 * Defines maximal number of pbuf data packet references for receive
 */
#ifndef GSM_CFG_NETCONN_RECEIVE_QUEUE_LEN
#define GSM_CFG_NETCONN_RECEIVE_QUEUE_LEN   8
#endif

/**
 * \}
 */

/**
 * \defgroup        GSM_OPT_MODULES_MQTT MQTT client module
 * \brief           Configuration of MQTT and MQTT API client modules
 * \{
 */

/**
 * \brief           Maximal number of open MQTT requests at a time
 *
 */
#ifndef GSM_CFG_MQTT_MAX_REQUESTS
#define GSM_CFG_MQTT_MAX_REQUESTS           8
#endif

/**
 * \brief           Set debug level for MQTT client module
 *
 * Possible values are \ref GSM_DBG_ON or \ref GSM_DBG_OFF
 */
#ifndef GSM_CFG_DBG_MQTT
#define GSM_CFG_DBG_MQTT                    GSM_DBG_OFF
#endif

/**
 * \brief           Set debug level for MQTT API client module
 *
 * Possible values are \ref GSM_DBG_ON or \ref GSM_DBG_OFF
 */
#ifndef GSM_CFG_DBG_MQTT_API
#define GSM_CFG_DBG_MQTT_API                GSM_DBG_OFF
#endif

/**
 * \}
 */

/**
 * \brief           Enables `1` or disables `0` network functionality
 *                      used for TCP/IP communication
 *
 * Network must be enabled to use all GPRS/LTE functions such
 * as connection API, FTP, HTTP, etc.
 */
#ifndef GSM_CFG_NETWORK
#define GSM_CFG_NETWORK                     0
#endif

/**
 * \brief           Ignores `1` or not `0` result from `AT+CGACT` command.
 *
 * \note            This may be used for data-only SIM cards where command might fail
 *                  when trying to attach to network for data transfer
 */
#ifndef GSM_CFG_NETWORK_IGNORE_CGACT_RESULT
#define GSM_CFG_NETWORK_IGNORE_CGACT_RESULT 0
#endif

/**
 * \brief           Enables `1` or disables `0` connection API.
 *
 * \note            \ref GSM_CFG_NETWORK must be enabled to use connection feature
 */
#ifndef GSM_CFG_CONN
#define GSM_CFG_CONN                        0
#endif

/**
 * \brief           Enables `1` or disables `0` SMS API.
 *
 */
#ifndef GSM_CFG_SMS
#define GSM_CFG_SMS                         0
#endif

/**
 * \brief           Enables `1` or disables `0` call API.
 *
 */
#ifndef GSM_CFG_CALL
#define GSM_CFG_CALL                        0
#endif

/**
 * \brief           Enables `1` or disables `0` phonebook API.
 *
 */
#ifndef GSM_CFG_PHONEBOOK
#define GSM_CFG_PHONEBOOK                   0
#endif

/**
 * \brief           Enables `1` or disables `0` HTTP API.
 *
 * \note            \ref GSM_CFG_NETWORK must be enabled to use connection feature
 */
#ifndef GSM_CFG_HTTP
#define GSM_CFG_HTTP                        0
#endif

/**
 * \brief           Enables `1` or disables `0` FTP API.
 *
 * \note            \ref GSM_CFG_NETWORK must be enabled to use connection feature
 */
#ifndef GSM_CFG_FTP
#define GSM_CFG_FTP                         0
#endif

/**
 * \brief           Enables `1` or disables `0` PING API.
 *
 * \note            \ref GSM_CFG_NETWORK must be enabled to use connection feature
 */
#ifndef GSM_CFG_PING
#define GSM_CFG_PING                        0
#endif

/**
 * \brief           Enables `1` or disables `0` USSD API.
 *
 */
#ifndef GSM_CFG_USSD
#define GSM_CFG_USSD                        0
#endif

/**
 * \}
 */

/**
 * \brief           Poll interval for connections in units of milliseconds
 *
 * Value indicates interval time to call poll event on active connections.
 *
 * \note            Single poll interval applies for all connections
 */
#ifndef GSM_CFG_CONN_POLL_INTERVAL
#define GSM_CFG_CONN_POLL_INTERVAL          500
#endif

/**
 * \defgroup        GSM_OPT_STD_LIB Standard library
 * \brief           Standard C library configuration
 * \{
 *
 * Configuration allows you to overwrite default C language function
 * in case of better implementation with hardware (for example DMA for data copy).
 */

/**
 * \brief           Memory copy function declaration
 *
 * User is able to change the memory function, in case
 * hardware supports copy operation, it may implement its own
 *
 * Function prototype must be similar to:
 *
 * \code{c}
void *  my_memcpy(void* dst, const void* src, size_t len);
\endcode
 *
 * \param[in]       dst: Destination memory start address
 * \param[in]       src: Source memory start address
 * \param[in]       len: Number of bytes to copy
 * \return          Destination memory start address
 */
#ifndef GSM_MEMCPY
#define GSM_MEMCPY(dst, src, len)           memcpy(dst, src, len)
#endif

/**
 * \brief           Memory set function declaration
 *
 * Function prototype must be similar to:
 *
 * \code{c}
void *  my_memset(void* dst, int b, size_t len);
\endcode
 *
 * \param[in]       dst: Destination memory start address
 * \param[in]       b: Value (byte) to set in memory
 * \param[in]       len: Number of bytes to set
 * \return          Destination memory start address
 */
#ifndef GSM_MEMSET
#define GSM_MEMSET(dst, b, len)             memset(dst, b, len)
#endif

/**
 * \}
 */

/**
 * \}
 */

#if !__DOXYGEN__

#if !GSM_CFG_OS
#if GSM_CFG_INPUT_USE_PROCESS
#error "GSM_CFG_INPUT_USE_PROCESS may only be enabled when OS is used!"
#endif /* GSM_CFG_INPUT_USE_PROCESS */
#endif /* !GSM_CFG_OS */

#endif /* !__DOXYGEN__ */

#include "lwgsm/lwgsm_debug.h"

#endif /* GSM_HDR_OPT_H */
