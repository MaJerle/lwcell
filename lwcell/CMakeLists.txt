cmake_minimum_required(VERSION 3.22)

# Library core sources
set(lwcell_core_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/src/lwcell/lwcell.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwcell/lwcell_buff.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwcell/lwcell_call.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwcell/lwcell_conn.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwcell/lwcell_debug.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwcell/lwcell_device_info.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwcell/lwcell_evt.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwcell/lwcell_http.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwcell/lwcell_input.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwcell/lwcell_int.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwcell/lwcell_mem.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwcell/lwcell_network.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwcell/lwcell_operator.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwcell/lwcell_parser.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwcell/lwcell_pbuf.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwcell/lwcell_phonebook.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwcell/lwcell_sim.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwcell/lwcell_sms.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwcell/lwcell_threads.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwcell/lwcell_timeout.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwcell/lwcell_unicode.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwcell/lwcell_ussd.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lwcell/lwcell_utils.c
)

# API sources
set(lwcell_api_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/src/api/lwcell_netconn.c
    ${CMAKE_CURRENT_LIST_DIR}/src/api/lwcell_network_api.c
)

# MQTT
set(lwcell_mqtt_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/src/apps/mqtt/lwcell_mqtt_client.c
    ${CMAKE_CURRENT_LIST_DIR}/src/apps/mqtt/lwcell_mqtt_client_api.c
    ${CMAKE_CURRENT_LIST_DIR}/src/apps/mqtt/lwcell_mqtt_client_evt.c
)

# All apps source files
set(lwcell_allapps_SRCS
    ${lwcell_mqtt_SRCS}
)

# Setup include directories
set(lwcell_include_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/src/include
)

# Add system port to core if user defined
if(DEFINED LWCELL_SYS_PORT)
    set(lwcell_core_SRCS ${lwcell_core_SRCS} ${CMAKE_CURRENT_LIST_DIR}/src/system/lwcell_sys_${LWCELL_SYS_PORT}.c)
    set(lwcell_include_DIRS ${lwcell_include_DIRS} ${CMAKE_CURRENT_LIST_DIR}/src/include/system/port/${LWCELL_SYS_PORT})
endif()

# Register core library to the system
add_library(lwcell INTERFACE)
target_sources(lwcell PUBLIC ${lwcell_core_SRCS})
target_include_directories(lwcell INTERFACE ${lwcell_include_DIRS})

# Register API to the system
add_library(lwcell_api INTERFACE)
target_sources(lwcell_api PUBLIC ${lwcell_api_SRCS})
target_include_directories(lwcell_api INTERFACE ${lwcell_include_DIRS})

# Register apps to the system
add_library(lwcell_apps INTERFACE)
target_sources(lwcell_apps PUBLIC ${lwcell_allapps_SRCS})
target_include_directories(lwcell_apps INTERFACE ${lwcell_include_DIRS})

# Create config file
if(DEFINED LWCELL_OPTS_DIR AND NOT EXISTS ${LWCELL_OPTS_DIR}/lwcell_opts.h)
    configure_file(${CMAKE_CURRENT_LIST_DIR}/src/include/lwcell/lwcell_opts_template.h ${LWCELL_OPTS_DIR}/lwcell_opts.h COPYONLY)
endif()