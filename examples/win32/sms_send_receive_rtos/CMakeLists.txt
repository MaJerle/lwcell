cmake_minimum_required(VERSION 3.22)

# Setup project
project(sms_send_receive_rtos)
add_executable(${PROJECT_NAME})

# Add key executable block
target_sources(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/main.c

    # Port
    ${CMAKE_CURRENT_LIST_DIR}/../../../lwgsm/src/system/lwgsm_sys_win32.c
    ${CMAKE_CURRENT_LIST_DIR}/../../../lwgsm/src/system/lwgsm_ll_win32.c

    # Snippets
    ${CMAKE_CURRENT_LIST_DIR}/../../../snippets/sim_manager.c
    ${CMAKE_CURRENT_LIST_DIR}/../../../snippets/network_utils.c
    ${CMAKE_CURRENT_LIST_DIR}/../../../snippets/examples_common_lwgsm_callback_func.c
    ${CMAKE_CURRENT_LIST_DIR}/../../../snippets/sms_send_receive.c
    ${CMAKE_CURRENT_LIST_DIR}/../../../snippets/sms_send_receive_thread.c
)

# Add key include paths
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/

    # Port
    ${CMAKE_CURRENT_LIST_DIR}/../../../lwgsm/src/include/system/port/win32

    # Snippets
    ${CMAKE_CURRENT_LIST_DIR}/../../../snippets/include
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
add_subdirectory("../../../lwgsm" lwgsm)
target_link_libraries(${PROJECT_NAME} lwgsm)
