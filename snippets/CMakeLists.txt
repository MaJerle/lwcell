cmake_minimum_required(VERSION 3.13)

#Debug message
message("Entering ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt")

# Register library to the system
add_library(lwgsm_snippets INTERFACE)

# Setup generic source files
target_sources(lwgsm_snippets PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/call.c
    ${CMAKE_CURRENT_LIST_DIR}/call_sms.c
    ${CMAKE_CURRENT_LIST_DIR}/client.c
    ${CMAKE_CURRENT_LIST_DIR}/device_info.c
    ${CMAKE_CURRENT_LIST_DIR}/mqtt_client_api.c
    ${CMAKE_CURRENT_LIST_DIR}/netconn_client.c
    ${CMAKE_CURRENT_LIST_DIR}/network_utils.c
    ${CMAKE_CURRENT_LIST_DIR}/sim_manager.c
    ${CMAKE_CURRENT_LIST_DIR}/sms_send_receive.c
    ${CMAKE_CURRENT_LIST_DIR}/sms_send_receive_thread.c
    )

# Setup include directories
target_include_directories(lwgsm_snippets INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include
    )

#Debug message
message("Exiting ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt")
