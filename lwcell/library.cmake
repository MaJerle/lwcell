# 
# LIB_PREFIX: LWCELL
#
# This file provides set of variables for end user
# and also generates one (or more) libraries, that can be added to the project using target_link_libraries(...)
#
# Before this file is included to the root CMakeLists file (using include() function), user can set some variables:
#
# LWCELL_SYS_PORT: If defined, it will include port source file from the library, and include the necessary header file.
# LWCELL_OPTS_FILE: If defined, it is the path to the user options file. If not defined, one will be generated for you automatically
# LWCELL_COMPILE_OPTIONS: If defined, it provide compiler options for generated library.
# LWCELL_COMPILE_DEFINITIONS: If defined, it provides "-D" definitions to the library build
#

# Custom include directory
set(LWCELL_CUSTOM_INC_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib_inc)

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
    ${LWCELL_CUSTOM_INC_DIR}
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
target_compile_options(lwcell PRIVATE ${LWCELL_COMPILE_OPTIONS})
target_compile_definitions(lwcell PRIVATE ${LWCELL_COMPILE_DEFINITIONS})

# Register API to the system
add_library(lwcell_api INTERFACE)
target_sources(lwcell_api PUBLIC ${lwcell_api_SRCS})
target_include_directories(lwcell_api INTERFACE ${lwcell_include_DIRS})
target_compile_options(lwcell_api PRIVATE ${LWCELL_COMPILE_OPTIONS})
target_compile_definitions(lwcell_api PRIVATE ${LWCELL_COMPILE_DEFINITIONS})

# Register apps to the system
add_library(lwcell_apps INTERFACE)
target_sources(lwcell_apps PUBLIC ${lwcell_allapps_SRCS})
target_include_directories(lwcell_apps INTERFACE ${lwcell_include_DIRS})
target_compile_options(lwcell_apps PRIVATE ${LWCELL_COMPILE_OPTIONS})
target_compile_definitions(lwcell_apps PRIVATE ${LWCELL_COMPILE_DEFINITIONS})

# Create config file if user didn't provide one info himself
if(NOT LWCELL_OPTS_FILE)
    message(STATUS "Using default lwcell_opts.h file")
    set(LWCELL_OPTS_FILE ${CMAKE_CURRENT_LIST_DIR}/src/include/lwcell/lwcell_opts_template.h)
else()
    message(STATUS "Using custom lwcell_opts.h file from ${LWCELL_OPTS_FILE}")
endif()
configure_file(${LWCELL_OPTS_FILE} ${LWCELL_CUSTOM_INC_DIR}/lwcell_opts.h COPYONLY)