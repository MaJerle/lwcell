cmake_minimum_required(VERSION 3.13)

# Debug message
message("LWGSM: Entering lib source CMakeLists.txt")

# Set library name
set(LWGSM_LIB_NAME "lwgsm")

# Check definitions of some key things
if(NOT DEFINED LWGSM_SYS_ARCH)
    message("LWGSM: LWGSM_SYS_ARCH not defined. Using default one: \"win32\"")
    set(LWGSM_SYS_ARCH "win32")
endif()
if(NOT DEFINED LWGSM_LL_ARCH)
    message("LWGSM: LWGSM_LL_ARCH not defined. Using default one: \"win32\"")
    set(LWGSM_LL_ARCH "win32")
endif()

# Register library to the system
add_library(${LWGSM_LIB_NAME} INTERFACE)

# Setup generic source files
target_sources(${LWGSM_LIB_NAME} PUBLIC
    # Core libraries
    ${CMAKE_CURRENT_LIST_DIR}/src/lwgsm/lwgsm.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwgsm/lwgsm_buff.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwgsm/lwgsm_call.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwgsm/lwgsm_conn.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwgsm/lwgsm_debug.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwgsm/lwgsm_device_info.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwgsm/lwgsm_evt.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwgsm/lwgsm_http.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwgsm/lwgsm_input.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwgsm/lwgsm_int.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwgsm/lwgsm_mem.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwgsm/lwgsm_network.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwgsm/lwgsm_operator.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwgsm/lwgsm_parser.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwgsm/lwgsm_pbuf.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwgsm/lwgsm_phonebook.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwgsm/lwgsm_sim.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwgsm/lwgsm_sms.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwgsm/lwgsm_threads.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwgsm/lwgsm_timeout.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwgsm/lwgsm_unicode.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwgsm/lwgsm_ussd.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwgsm/lwgsm_utils.c
    
    # API part
    ${CMAKE_CURRENT_LIST_DIR}/src/api/lwgsm_netconn.c
    ${CMAKE_CURRENT_LIST_DIR}/src/api/lwgsm_network_api.c

    # Applications
    ${CMAKE_CURRENT_LIST_DIR}/src/apps/mqtt/lwgsm_mqtt_client.c
    ${CMAKE_CURRENT_LIST_DIR}/src/apps/mqtt/lwgsm_mqtt_client_api.c
    ${CMAKE_CURRENT_LIST_DIR}/src/apps/mqtt/lwgsm_mqtt_client_evt.c

    # Port files
    ${CMAKE_CURRENT_LIST_DIR}/src/system/lwgsm_sys_${LWGSM_SYS_ARCH}.c
    ${CMAKE_CURRENT_LIST_DIR}/src/system/lwgsm_ll_${LWGSM_LL_ARCH}.c
    )

# Setup include directories
target_include_directories(${LWGSM_LIB_NAME} INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/include
    ${CMAKE_CURRENT_LIST_DIR}/src/include/system/port/${LWGSM_SYS_ARCH}
    )
