# Copyright (c) 2020-2021 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

add_library(mbed-m2354-lcd INTERFACE)

target_include_directories(mbed-m2354-lcd
    INTERFACE
        .
)

target_sources(mbed-m2354-lcd
    INTERFACE
        lcd_api.c
        lcdlib.c
)

