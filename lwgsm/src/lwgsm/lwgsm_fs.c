/**
 * \file            lwgsm_fs.c
 * \brief           File System functions
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
 * Authors:         Tilen MAJERLE <tilen@majerle.eu>,
 *                  Ilya Kargapolov <d3vil.st@gmail.com>
 * Version:         v0.1.0
 */
#include "lwgsm/lwgsm_private.h"
#include "lwgsm/lwgsm_mem.h"

#if LWGSM_CFG_FS || __DOXYGEN__

/**
 * \brief           Create file on a file system
 * \param[in]       path: Pointer to string array which contain path
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_fs_create(const char* path, const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_FS_CREATE;
    LWGSM_MSG_VAR_REF(msg).msg.fs_file.path = path;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 10000);
}

/**
 * \brief           Delete file from a file system
 * \param[in]       path: Pointer to string array which contain path
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_fs_delete(const char* path, const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_FS_DELETE;
    LWGSM_MSG_VAR_REF(msg).msg.fs_file.path = path;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 10000);
}

/**
 * \brief           Write content to a file on a file system
 * \param[in]       path: Pointer to string array which contain path
 * \param[in]       mode: mode of file
 * \param[in]       size: Size of file
 * \param[in]       content: Pointer to string array which contain content for a file to be written
 * \param[in]       input_time: timeout for a modem to complete operation in seconds
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_fs_write(const char* path, uint8_t mode, uint16_t size, const char* content, uint8_t input_time,
                const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_FS_WRITE;
    LWGSM_MSG_VAR_REF(msg).msg.fs_file.path = path;
    LWGSM_MSG_VAR_REF(msg).msg.fs_file.mode = mode;
    LWGSM_MSG_VAR_REF(msg).msg.fs_file.size = size;
    LWGSM_MSG_VAR_REF(msg).msg.fs_file.content = content;
    LWGSM_MSG_VAR_REF(msg).msg.fs_file.input_time = input_time;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 10000);
}

#endif /* LWGSM_CFG_FS || __DOXYGEN__ */
