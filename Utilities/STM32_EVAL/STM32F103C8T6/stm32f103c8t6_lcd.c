/**
  ******************************************************************************
  * @file    stm32f103c8t6_lcd.c
  * @author  MCD Application Team
  * @version V4.6.1
  * @date    18-April-2011
  * @brief   This file includes the LCD driver for AM-240320LTNQW00H (LCD_HX8312),
  *          AM-240320L8TNQW00H (LCD_ILI9320), AM-240320LDTNQW00H (LCD_SPFD5408B)
  *          , ILI9341 Generic LCD Module
  *          Liquid Crystal Display Module of STM3210B-EVAL board.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************  
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f103c8t6_lcd.h"

/** @addtogroup Utilities
  * @{
  */

/** @addtogroup STM32_EVAL
  * @{
  */ 

/** @addtogroup STM3210B_EVAL
  * @{
  */
    
/** @defgroup STM3210B_EVAL_LCD 
  * @briefThis file includes the LCD driver for AM-240320LTNQW00H (LCD_HX8312),
  *          AM-240320L8TNQW00H (LCD_ILI9320), AM-240320LDTNQW00H (LCD_SPFD5408B)
  *          ILI9341 Generic Driver
  *          Liquid Crystal Display Module of STM3210B-EVAL board.
  * @{
  */ 

#define LCD_ILI9320        0x9320
#define LCD_ILI9341		   0x9341
#define LCD_HX8312         0x8312
#define LCD_SPFD5408       0x5408

#define USE_LCD_ILI9341

#ifdef USE_LCD_ILI9341
 #include "stm32f103c8t6_lcd_ili9341.c"
#elif defined USE_LCD_ILI9320
 #include "stm32f103c8t6_lcd_others.c"
#elif defined USE_LCD_HX8312
 #include "stm32f103c8t6_lcd_others.c"
#elif defined USE_LCD_SPFD5408
 #include "stm32f103c8t6_lcd_others.c"
#else
 #error "Please select first the STM32 EVAL board to be used (in stm32_eval.h)"
#endif



