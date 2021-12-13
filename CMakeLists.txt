cmake_minimum_required(VERSION 3.0.0)
project(LwGSM VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(${PROJECT_NAME}
    lwgsm/src/api/lwgsm_netconn.c
    lwgsm/src/api/lwgsm_network_api.c
    lwgsm/src/apps/mqtt/lwgsm_mqtt_client.c
    lwgsm/src/apps/mqtt/lwgsm_mqtt_client_api.c
    lwgsm/src/apps/mqtt/lwgsm_mqtt_client_evt.c
    lwgsm/src/lwgsm/lwgsm.c
    lwgsm/src/lwgsm/lwgsm_buff.c
    lwgsm/src/lwgsm/lwgsm_call.c
    lwgsm/src/lwgsm/lwgsm_conn.c
    lwgsm/src/lwgsm/lwgsm_debug.c
    lwgsm/src/lwgsm/lwgsm_device_info.c
    lwgsm/src/lwgsm/lwgsm_evt.c
    lwgsm/src/lwgsm/lwgsm_http.c
    lwgsm/src/lwgsm/lwgsm_input.c
    lwgsm/src/lwgsm/lwgsm_int.c
    lwgsm/src/lwgsm/lwgsm_mem.c
    lwgsm/src/lwgsm/lwgsm_network.c
    lwgsm/src/lwgsm/lwgsm_operator.c
    lwgsm/src/lwgsm/lwgsm_parser.c
    lwgsm/src/lwgsm/lwgsm_pbuf.c
    lwgsm/src/lwgsm/lwgsm_phonebook.c
    lwgsm/src/lwgsm/lwgsm_sim.c
    lwgsm/src/lwgsm/lwgsm_sms.c
    lwgsm/src/lwgsm/lwgsm_threads.c
    lwgsm/src/lwgsm/lwgsm_timeout.c
    lwgsm/src/lwgsm/lwgsm_unicode.c
    lwgsm/src/lwgsm/lwgsm_ussd.c
    lwgsm/src/lwgsm/lwgsm_utils.c
    lwgsm/src/system/lwgsm_ll_win32.c
    lwgsm/src/system/lwgsm_mem_lwmem.c
    lwgsm/src/system/lwgsm_sys_win32.c
    snippets/call.c
    snippets/call_sms.c
    snippets/client.c
    snippets/device_info.c
    snippets/mqtt_client_api.c
    snippets/netconn_client.c
    snippets/network_utils.c
    snippets/sim_manager.c
    snippets/sms_send_receive.c
    snippets/sms_send_receive_thread.c

    ../lwmem/lwmem/src/lwmem/lwmem.c
    ../lwmem/lwmem/src/system/lwmem_sys_win32.c

    dev/VisualStudio/main.c
    )

target_include_directories(${PROJECT_NAME} PRIVATE
    dev/VisualStudio
    lwgsm/src/include
    lwgsm/src/include/system/port/win32
    snippets/include
    ../lwmem/lwmem/src/include
    )

target_compile_definitions(${PROJECT_NAME} PRIVATE
    WIN32
    _DEBUG
    CONSOLE
    LWGSM_DEV
    )

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
