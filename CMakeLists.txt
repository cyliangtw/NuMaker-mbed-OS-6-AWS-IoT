# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.19.0 FATAL_ERROR)

set(MBED_PATH ${CMAKE_CURRENT_SOURCE_DIR}/mbed-os CACHE INTERNAL "")
set(MBED_CONFIG_PATH ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")
set(APP_TARGET NuMaker-mbed-6-AWS-IoT)

include(${MBED_PATH}/tools/cmake/app.cmake)

project(${APP_TARGET})

add_subdirectory(${MBED_PATH})
add_subdirectory(MQTT)

add_executable(${APP_TARGET})

mbed_configure_app_target(${APP_TARGET})

target_include_directories(${APP_TARGET}
    PRIVATE
        .
        my-tlssocket
        pre-main
        targets/TARGET_NUVOTON
)

target_sources(${APP_TARGET}
    PRIVATE
        main.cpp
        my-tlssocket/MyTLSSocket.cpp
        pre-main/dispatch_host_command.cpp
        pre-main/fetch_host_command.cpp
        pre-main/mbed_main.cpp
        pre-main/mem_stats.cpp
        pre-main/provision.cpp
        pre-main/pump_host_command.cpp
        $<$<IN_LIST:NUVOTON,${MBED_TARGET_LABELS}>:targets/TARGET_NUVOTON/platform_entropy.cpp>
)

target_link_libraries(${APP_TARGET}
    PRIVATE
        mbed-os
        mbed-mbedtls
        mbed-netsocket
        mbed-wifi
        MQTT
)

# Meet unclear trouble with genex inside target_link_libraries. Change to if-else as workaround.
if("DEVICE_FLASH=1" IN_LIST MBED_TARGET_DEFINITIONS)
    target_link_libraries(${APP_TARGET}
        PRIVATE
            mbed-storage-kvstore
    )
endif()

if("NU_IOT_M2354" IN_LIST MBED_TARGET_LABELS)
    add_library(mbed-nu-iot-m2354 INTERFACE)
    add_subdirectory(targets/TARGET_NUVOTON/TARGET_NU_IOT_M2354)
    add_subdirectory(targets/TARGET_NUVOTON/TARGET_M2354/LCD)
    target_include_directories(mbed-nu-iot-m2354
        INTERFACE
            targets/TARGET_NUVOTON/TARGET_NU_IOT_M2354
    )
    target_link_libraries(mbed-nu-iot-m2354
        INTERFACE
            mbed-m2354
            mbed-m2354-tfm
            mbed-m2354-lcd 
    )
endif()

if("NU_M2354" IN_LIST MBED_TARGET_LABELS)
    add_subdirectory(targets/TARGET_NUVOTON/TARGET_M2354/LCD)
    target_link_libraries(mbed-nu-m2354
        INTERFACE
            mbed-m2354
            mbed-m2354-tfm
            mbed-m2354-lcd 
    )
endif()


mbed_set_post_build(${APP_TARGET})

option(VERBOSE_BUILD "Have a verbose build process")
if(VERBOSE_BUILD)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()
