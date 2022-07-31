cmake_minimum_required(VERSION 3.22)

# Setup project
project(${PROJECT_NAME})
add_executable(${PROJECT_NAME})
message("Project name: ${PROJECT_NAME}")

# Add source files
target_sources(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}/main.c

    # Port
    ${CMAKE_CURRENT_LIST_DIR}/../../lwgsm/src/system/lwgsm_sys_win32.c
    ${CMAKE_CURRENT_LIST_DIR}/../../lwgsm/src/system/lwgsm_ll_win32.c

    # Snippets
    ${CMAKE_CURRENT_LIST_DIR}/../../snippets/sim_manager.c
    ${CMAKE_CURRENT_LIST_DIR}/../../snippets/network_utils.c
    ${CMAKE_CURRENT_LIST_DIR}/../../snippets/examples_common_lwgsm_callback_func.c
)

# Add include paths
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}/

    # Port
    ${CMAKE_CURRENT_LIST_DIR}/../../lwgsm/src/include/system/port/win32

    # Snippets
    ${CMAKE_CURRENT_LIST_DIR}/../../snippets/include
)

# Compilation definition information
target_compile_definitions(${PROJECT_NAME} PUBLIC
    WIN32
    _DEBUG
    CONSOLE
)

# Compiler options
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# Add subdir with lwgsm and link to the project
add_subdirectory("../../lwgsm" lwgsm)
target_link_libraries(${PROJECT_NAME} lwgsm)

# Project specific sources and libs
if (${PROJECT_NAME} STREQUAL "call_rtos")
target_sources(${PROJECT_NAME} PUBLIC   ${CMAKE_CURRENT_LIST_DIR}/../../snippets/call.c)
endif()
if (${PROJECT_NAME} STREQUAL "call_sms_rtos")
target_sources(${PROJECT_NAME} PUBLIC   ${CMAKE_CURRENT_LIST_DIR}/../../snippets/call_sms.c)
endif()
if (${PROJECT_NAME} STREQUAL "device_info_rtos")
target_sources(${PROJECT_NAME} PUBLIC   ${CMAKE_CURRENT_LIST_DIR}/../../snippets/device_info.c)
endif()
if (${PROJECT_NAME} STREQUAL "mqtt_client_api_rtos")
target_sources(${PROJECT_NAME} PUBLIC   ${CMAKE_CURRENT_LIST_DIR}/../../snippets/mqtt_client_api.c)
target_link_libraries(${PROJECT_NAME}   lwgsm_api)
target_link_libraries(${PROJECT_NAME}   lwgsm_apps)
endif()
if (${PROJECT_NAME} STREQUAL "netconn_client_rtos")
target_sources(${PROJECT_NAME} PUBLIC   ${CMAKE_CURRENT_LIST_DIR}/../../snippets/netconn_client.c)
target_link_libraries(${PROJECT_NAME}   lwgsm_api)
target_link_libraries(${PROJECT_NAME}   lwgsm_apps)
endif()
if (${PROJECT_NAME} STREQUAL "sms_send_receive_rtos")
target_sources(${PROJECT_NAME} PUBLIC   ${CMAKE_CURRENT_LIST_DIR}/../../snippets/sms_send_receive.c)
target_sources(${PROJECT_NAME} PUBLIC   ${CMAKE_CURRENT_LIST_DIR}/../../snippets/sms_send_receive_thread.c)
endif()
