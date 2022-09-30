cmake_minimum_required(VERSION 3.22)

# Setup project
project(LwLibPROJECT)

if(NOT PROJECT_IS_TOP_LEVEL)
    add_subdirectory(lwgsm)
else()
    # Set as executable
    add_executable(${PROJECT_NAME})

    # Add key executable block
    target_sources(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/dev/main.c

        # Development additional files
        ${CMAKE_CURRENT_LIST_DIR}/lwgsm/src/system/lwgsm_mem_lwmem.c
        ${CMAKE_CURRENT_LIST_DIR}/../lwmem/lwmem/src/lwmem/lwmem.c
        ${CMAKE_CURRENT_LIST_DIR}/../lwmem/lwmem/src/system/lwmem_sys_win32.c

        # Port specific
        ${CMAKE_CURRENT_LIST_DIR}/lwgsm/src/system/lwgsm_sys_win32.c
        ${CMAKE_CURRENT_LIST_DIR}/lwgsm/src/system/lwgsm_ll_win32.c
    )

    # Add key include paths
    target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/dev
        ${CMAKE_CURRENT_LIST_DIR}/../lwmem/lwmem/src/include

        # Port specific
        ${CMAKE_CURRENT_LIST_DIR}/lwgsm/src/include/system/port/win32
    )

    # Compilation definition information
    target_compile_definitions(${PROJECT_NAME} PUBLIC
        WIN32
        _DEBUG
        CONSOLE
        LWGSM_DEV
    )

    # Compiler options
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    # Add subdir with lwgsm and link to the project
    add_subdirectory(lwgsm)
    target_link_libraries(${PROJECT_NAME} lwgsm)
    target_link_libraries(${PROJECT_NAME} lwgsm_api)
    target_link_libraries(${PROJECT_NAME} lwgsm_apps)
    add_subdirectory(snippets)
    target_link_libraries(${PROJECT_NAME} lwgsm_snippets)
endif()
