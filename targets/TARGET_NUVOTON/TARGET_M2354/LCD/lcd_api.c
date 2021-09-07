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

#include "lcd_api.h"
#include "lcdlib.h"

#include "cmsis.h"
#include "pinmap.h"
#include "PeripheralPins.h"


/**
  * @brief      Set Charge Pump Voltage
  *
  * @param[in]  voltage     The target charge pump voltage. It could be one of the following voltage level
  *                             - \ref LCD_CP_VOLTAGE_LV_0, 2.6 V
  *                             - \ref LCD_CP_VOLTAGE_LV_1, 2.8 V
  *                             - \ref LCD_CP_VOLTAGE_LV_2, 3.0 V
  *                             - \ref LCD_CP_VOLTAGE_LV_3, 3.2 V
  *                             - \ref LCD_CP_VOLTAGE_LV_4, 3.4 V
  *                             - \ref LCD_CP_VOLTAGE_LV_5, 3.6 V
  *
  * @return     None
  *
  * @details    This macro is used to set charge pump voltage for VLCD.
  */
#define LCD_SET_CP_VOLTAGE(voltage) (LCD->PCTL = (LCD->PCTL & ~LCD_PCTL_CPVSEL_Msk) | (voltage))

/*---------------------------------------------------------------------------------------------------------*/
/* Functions and variables declaration                                                                     */
/*---------------------------------------------------------------------------------------------------------*/
#define LCD_ALPHABET_NUM    7
static S_LCD_CFG_T g_LCDCfg =
{
    __LIRC,                     /*!< LCD clock source frequency */
    LCD_COM_DUTY_1_8,           /*!< COM duty */
    LCD_BIAS_LV_1_4,            /*!< Bias level */
    30,                         /*!< Operation frame rate */
    LCD_WAVEFORM_TYPE_A_NORMAL, /*!< Waveform type */
    LCD_DISABLE_ALL_INT,        /*!< Interrupt source */
    LCD_LOW_DRIVING_AND_BUF_ON, /*!< Driving mode */
//    LCD_VOLTAGE_SOURCE_AVDD,
    LCD_VOLTAGE_SOURCE_CP,      /*!< Voltage source */
};

static void configure_lcd_pins(void);

static void configure_lcd_pins(void)
{
    /*
        Summary of LCD pin usage:
            COM 0~5   : PC.0, PC.1, PC.2, PC.3, PC.4, PC.5
            COM 6~7   : PD.8, PD.9
            SEG 0     : PD.14
            SEG 1~4   : PH.11, PH.10, PH.9, PH.8
            SEG 5~12  : PE.0, PE.1, PE.2, PE.3, PE.4, PE.5, PE.6, PE.7
            SEG 13~14 : PD.6, PD.7
            SEG 15~21 : PG.15, PG.14, PG.13, PG.12, PG.11, PG.10, PG.9
            SEG 22~23 : PE.15, PE.14
            SEG 24~29 : PA.0, PA.1, PA.2, PA.3, PA.4, PA.5
            SEG 30~32 : PE.10, PE.9, PE.8
            SEG 33~36 : PH.7, PH.6, PH.5, PH.4
            SEG 37~39 : PG.4, PG.3, PG.2
    */

    /* COM 0~5 */
    pin_function(PC_0, LCD_COM0_PC0);
    pin_function(PC_1, LCD_COM1_PC1);
    pin_function(PC_2, LCD_COM2_PC2);
    pin_function(PC_3, LCD_COM3_PC3);
    pin_function(PC_4, LCD_COM4_PC4);
    pin_function(PC_5, LCD_COM5_PC5);

    /* COM 6~7 */
    pin_function(PD_8, LCD_COM6_PD8);
    pin_function(PD_9, LCD_COM7_PD9);

    /* SEG 0 */
    pin_function(PD_14, LCD_SEG0_PD14);

    /* SEG 1~4 */
    pin_function(PH_11, LCD_SEG1_PH11);
    pin_function(PH_10, LCD_SEG2_PH10);
    pin_function(PH_9, LCD_SEG3_PH9);
    pin_function(PH_8, LCD_SEG4_PH8);

    /* SEG 5~12 */
    pin_function(PE_0, LCD_SEG5_PE0);
    pin_function(PE_1, LCD_SEG6_PE1);
    pin_function(PE_2, LCD_SEG7_PE2);
    pin_function(PE_3, LCD_SEG8_PE3);
    pin_function(PE_4, LCD_SEG9_PE4);
    pin_function(PE_5, LCD_SEG10_PE5);
    pin_function(PE_6, LCD_SEG11_PE6);
    pin_function(PE_7, LCD_SEG12_PE7);


    /* SEG 13~14 */
    pin_function(PD_6, LCD_SEG13_PD6);
    pin_function(PD_7, LCD_SEG14_PD7);

    /* SEG 15~21 */
    pin_function(PG_15, LCD_SEG15_PG15);
    pin_function(PG_14, LCD_SEG16_PG14);
    pin_function(PG_13, LCD_SEG17_PG13);
    pin_function(PG_12, LCD_SEG18_PG12);
    pin_function(PG_11, LCD_SEG19_PG11);
    pin_function(PG_10, LCD_SEG20_PG10);
    pin_function(PG_9, LCD_SEG21_PG9);
    

    /* SEG 22~23 */
    pin_function(PE_15, LCD_SEG22_PE15);
    pin_function(PE_14, LCD_SEG23_PE14);
    
    /* SEG 24~29 */
    pin_function(PA_0, LCD_SEG24_PA0);
    pin_function(PA_1, LCD_SEG25_PA1);
    pin_function(PA_2, LCD_SEG26_PA2);
    pin_function(PA_3, LCD_SEG27_PA3);
    pin_function(PA_4, LCD_SEG28_PA4);
    pin_function(PA_5, LCD_SEG29_PA5);
    
    /* SEG 30~32 */
    pin_function(PE_10, LCD_SEG30_PE10);
    pin_function(PE_9, LCD_SEG31_PE9);
    pin_function(PE_8, LCD_SEG32_PE8);
    
    /* SEG 33~36 */
    pin_function(PH_7, LCD_SEG33_PH7);
    pin_function(PH_6, LCD_SEG34_PH6);
    pin_function(PH_5, LCD_SEG35_PH5);
    pin_function(PH_4, LCD_SEG36_PH4);
    
    /* SEG 37~39 */
    pin_function(PG_4, LCD_SEG37_PG4);
    pin_function(PG_3, LCD_SEG38_PG3);
    pin_function(PG_2, LCD_SEG39_PG2);
}

void lcd_init(void)
{
    uint32_t u32ActiveFPS;
    // Validation
    // MBED_ASSERT(...)
  
    // Set Pin out
    configure_lcd_pins();
  
    /* Select IP clock source
     *
     * NOTE: We must call secure version (from non-secure domain) because SYS/CLK regions are secure.
     */
    CLK_SetModuleClock_S(LCD_MODULE, CLK_CLKSEL1_LCDSEL_LIRC, 0);
//    CLK_SetModuleClock_S(LCD_MODULE, CLK_CLKSEL1_LCDSEL_LXT, 0);
    CLK_EnableModuleClock_S(LCD_MODULE);

    /* Reset module
     *
     * NOTE: We must call secure version (from non-secure domain) because SYS/CLK regions are secure.
     */
    SYS_ResetModule_S(LCD_RST);


    // Open LCD
    /* LCD Initialize and calculate real frame rate */
    u32ActiveFPS = LCD_Open(&g_LCDCfg); 

    /* Enable charge pump clock MIRC and output voltage level 2 for 3.0V */
    if(g_LCDCfg.u32VSrc == LCD_VOLTAGE_SOURCE_CP) {
        CLK_EnableModuleClock_S(LCDCP_MODULE);
        CLK_SetModuleClock_S(LCDCP_MODULE, CLK_CLKSEL1_LCDCPSEL_MIRC, 0);
        LCD_SET_CP_VOLTAGE(LCD_CP_VOLTAGE_LV_4);
    }
    /* Enable LCD display */
    LCD_ENABLE_DISPLAY();
}


void lcd_free(void)
{
    // Validation
    // MBED_ASSERT(...)
    
    /* Disable lcd interrupt */
    NVIC_DisableIRQ(LCD_IRQn);

    LCD_DISABLE_DISPLAY();
 
    /* Disable lcd module */
    LCD_Close();

    /* Disable IP clock
     *
     * NOTE: We must call secure version (from non-secure domain) because SYS/CLK regions are secure.
     */
    CLK_DisableModuleClock_S(LCD_MODULE);

    /* Free up pins to be GPIO */
}

void lcd_printf(char *InputStr)
{
    char text[LCD_ALPHABET_NUM+1];
    
    memset(text, 0x00, sizeof(text));
    if(strlen(InputStr) >= LCD_ALPHABET_NUM) {
        strncp(text, InputStr, LCD_ALPHABET_NUM);
    } else {
        strcpy(text, InputStr);
    }
    LCDLIB_Printf(ZONE_MAIN_DIGIT, text);
}

void lcd_putChar(uint32_t u32Index, uint8_t u8Ch)
{
    LCDLIB_PutChar(ZONE_MAIN_DIGIT, u32Index, u8Ch);
}

void lcd_printNumber(uint32_t InputNum)
{
    LCDLIB_PrintNumber(ZONE_MAIN_DIGIT, InputNum);
}

void lcd_setSymbol(uint32_t u32Symbol, uint32_t u32OnOff)
{
    LCDLIB_SetSymbol(u32Symbol, u32OnOff);
}
