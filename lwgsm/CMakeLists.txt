cmake_minimum_required(VERSION 3.13)

#Debug message
message("Entering ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt")

# Library core sources
set(lwgsm_core_SRCS
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
    )
    
# API sources
set(lwgsm_api_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/src/api/lwgsm_netconn.c
    ${CMAKE_CURRENT_LIST_DIR}/src/api/lwgsm_network_api.c
    )

# MQTT
set(lwgsm_mqtt_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/src/apps/mqtt/lwgsm_mqtt_client.c
    ${CMAKE_CURRENT_LIST_DIR}/src/apps/mqtt/lwgsm_mqtt_client_api.c
    ${CMAKE_CURRENT_LIST_DIR}/src/apps/mqtt/lwgsm_mqtt_client_evt.c
    )

# All apps source files
set(lwgsm_allapps_SRCS
    ${lwgsm_mqtt_SRCS}
    )

# Setup include directories
set(lwgsm_include_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/src/include
    )

# Register core library to the system
add_library(lwgsm INTERFACE)
target_sources(lwgsm PUBLIC ${lwgsm_core_SRCS})
target_include_directories(lwgsm INTERFACE ${lwgsm_include_DIRS})

# Register API to the system
add_library(lwgsm_api INTERFACE)
target_sources(lwgsm_api PUBLIC ${lwgsm_api_SRCS})
target_include_directories(lwgsm_api INTERFACE ${lwgsm_include_DIRS})

# Register apps to the system
add_library(lwgsm_apps INTERFACE)
target_sources(lwgsm_apps PUBLIC ${lwgsm_allapps_SRCS})
target_include_directories(lwgsm_apps INTERFACE ${lwgsm_include_DIRS})

#Debug message
message("Exiting ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt")
