/*
 * Copyright (c) 2021, Nuvoton Technology Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MBED_LCD_API_H
#define MBED_LCD_API_H

#include "device.h"
#include "pinmap.h"

#ifdef __cplusplus
extern "C" {
#endif

void lcd_init(void);
void lcd_free(void);
void lcd_printf(const char *InputStr);
void lcd_putChar(uint32_t u32Index, uint8_t u8Ch);
void lcd_printNumber(uint32_t InputNum);
void lcd_setSymbol(uint32_t u32Symbol, uint32_t u32OnOff);

#ifdef __cplusplus
}
#endif

#endif
