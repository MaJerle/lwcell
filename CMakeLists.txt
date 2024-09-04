cmake_minimum_required(VERSION 3.22)

# Setup project
project(LwLibPROJECT)

if(NOT PROJECT_IS_TOP_LEVEL)
    add_subdirectory(lwcell)
else()
    add_executable(${PROJECT_NAME})
    target_sources(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/dev/main.c

        # Development additional files
        ${CMAKE_CURRENT_LIST_DIR}/lwcell/src/system/lwcell_mem_lwmem.c
        ${CMAKE_CURRENT_LIST_DIR}/../lwmem/lwmem/src/lwmem/lwmem.c
        ${CMAKE_CURRENT_LIST_DIR}/../lwmem/lwmem/src/system/lwmem_sys_win32.c

        # Port specific
        ${CMAKE_CURRENT_LIST_DIR}/lwcell/src/system/lwcell_ll_win32.c
    )
    target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/dev
        ${CMAKE_CURRENT_LIST_DIR}/../lwmem/lwmem/src/include
    )

    # Add subdir with lwcell and link to the project
    set(LWCELL_OPTS_FILE ${CMAKE_CURRENT_LIST_DIR}/dev/lwcell_opts.h)
    set(LWCELL_SYS_PORT "win32")
    add_subdirectory(lwcell)
    target_link_libraries(${PROJECT_NAME} PUBLIC lwcell_api)
    target_link_libraries(${PROJECT_NAME} PUBLIC lwcell_apps)
    target_link_libraries(${PROJECT_NAME} PUBLIC lwcell)
    add_subdirectory(snippets)
    target_link_libraries(${PROJECT_NAME} PUBLIC lwcell_snippets)

    # Add compile options to the library, which will propagate options to executable through public link
    target_compile_definitions(lwcell PUBLIC WIN32 _DEBUG CONSOLE LWCELL_DEV)
    target_compile_options(lwcell PUBLIC -Wall -Wextra -Wpedantic)
endif()
