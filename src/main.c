/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V2.0.0
  * @date    11-July-2011
  * @brief   This file provides main program functions.
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
#include "main.h"
#include "touchscreen.h"
#include "hw_config.h"
#include "cursor.h"
#include "graphicObject.h"
#include "pictures.h"
#include "uiframework.h"
#include "batterie.h"
#include "watersensor.h"
#include "Common/fonts.h"
#include "temperatur.h"
#include "switches.h"
#include <stdio.h>
#include "gl_fonts.h"
#include "adc.h"
#include "i2c.h"
#include "rtc.h"
#include "spi.h"
#include "flash.h"
#include "settings.h"
#include "i2c_switch.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "usb_cdc.h"
#include "usb_endp.h"
#include "bq34z100g1.h"
/** @addtogroup Embedded_GUI_Example
  * @{
  */

/** @defgroup Main
  * @brief Main program body
  * @{
  */

/** @addtogroup Embedded_GUI_Example
  * @{
  */

/** @defgroup Main
  * @brief Main program body
  * @{
  */

/* External variables --------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#if defined(USE_STM32100E_EVAL)
#define LAST_FLASH_MEMORY_ADDRESS	((uint32_t)0x08080000)
#elif defined(USE_STM322xG_EVAL)
#define LAST_FLASH_MEMORY_ADDRESS	((uint32_t)0x08100000)
#elif defined(USE_STM3210C_EVAL)
#define LAST_FLASH_MEMORY_ADDRESS	((uint32_t)0x08040000)
#endif
/* Private macros ------------------------------------------------------------*/

/**
  * @brief   Small printf for GCC/RAISONANCE
  */
#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)

#endif /* __GNUC__ */

/* Private variables ---------------------------------------------------------*/
uint8_t RESET_VALUES;
uint8_t counter;
volatile uint8_t irqvals;
volatile uint8_t SPI1_Bus_Token;

/* Private function prototypes -----------------------------------------------*/
void InputInterface_Init(void);
void ShowLoadingLogo(void);
void CatchInputEvents(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */


int main(void)
{

	/* Define Variables */
	uint8_t switch_display, debounce_display, debounce_minute, retval, minutes;
	uint32_t flash_address;
	uint16_t flash_block;
	uint16_t flash_page;
	uint8_t togglevar;

	uint8_t flash_data[40];
	uint8_t i, j;


	/* Startup code for clock etc ... */
	RCC_Configuration();

	/* Device specific NVIC Configuration */
	NVIC_Configuration();

	/* Initialize Variables */
	battery_logbuffer[256]=0;
	battery_logbuffer[257]=0;
	switch_display = 0b00000001;
	debounce_display = 0;
	debounce_minute = 0;
	batstat=3;
	voltage_sample[1]=0;
	voltage_sample[2]=0;
	voltage_sample_counter[1]=0;
	voltage_sample_counter[2]=0;
	RESET_VALUES=1;
	private_percent=200;
	prevlevel[0]=120;
	prevlevel[1]=120;
	SPI1_Bus_Token=0;

	// Setup the ADC
	// ADC_Setup();
	read_settings();
	SWITCH_Setup();

	STM_EVAL_LEDInit(LED1);
	STM_EVAL_FETInit(FET1);
	STM_EVAL_FETInit(FET2);
	//STM_EVAL_LEDInit(LED2);
	//STM_EVAL_LEDInit(LED3);
	//STM_EVAL_LEDInit(LED4);
	//GPIO_SetBits(GPIOA, GPIO_Pin_11);

	STM_EVAL_LEDOff(LED1);
	STM_EVAL_FETOff(FET1);
	STM_EVAL_FETOff(FET2);
	//STM_EVAL_LEDOn(LED2);
	//STM_EVAL_LEDOn(LED3);
	//STM_EVAL_LEDOn(LED4);
	STM_EVAL_LEDOn(LED1);

	/*
	 * Initialize Flash specific Controller Pins
	 * In this case this is only the Chip Select Pin
	 */
	Flash_Hardware_Init();
	/*
	 * Initialize the SPI Interface
	 * needs to be done before any SPI transfer takes place
	 * Should be done after Chip Select lines are initialized and held in
	 * deselected state.
	 */
	SPI1_BusInit();

	/* If the LCD Panel has a resolution of 320x240 this command is not needed, it's set by default */
	/* Set_LCD_Resolution( 320, 240 ); */

	/* Initialize the LCD */
	while(SPI1_Bus_Token != 0)
	{
	}
	GL_LCD_Init();



	while(SPI1_Bus_Token != 0)
	{
	}
	GL_Clear(GL_White);

	while(SPI1_Bus_Token != 0)
	{
	}
	GL_SetFont(GL_FONT_BIG);
	/* Manually free the i2c bus via bitbanging */
	I2C_free_bus(I2C2);
	/* Check for devices on i2c bus via bitbanging */
	I2C_scan_bus();

	/* Initialize the I2C Interface */
	I2C_LowLevel_Init(I2C2);
	rtc_prevtime.second=80;
	rtc_prevtime.minute=80;
	rtc_prevtime.hour=80;
	rtc_prevdate.day=80;
	rtc_prevdate.month=80;
	rtc_prevdate.year=80;
	rtc_prevdate.weekday=80;


	GL_SetFont(GL_FONT_BIG);

	//ADC_Setup();



	while(SPI1_Bus_Token != 0)
	{
	}
	GL_Clear(GL_White);

	/* The function rtc_write_init
	 * can be used to program the RTC chip
	 * with user defined values.
	 * The function is found in rtc.c
	 */
	//rtc_write_init();

	//Timeout100usIntConfiguration();
	rtc_init();
	initialize_system_clock();
	initialize_i2c_switch(1);
	bq34z100g1_init();
	rtc_time.unixtime=synchronize_unixtime();





	//InitializeDisplayTimer();
	LCD_DrawBatterie(0x3, 0x4, 277, GL_Black, GL_White);

	watersensor_setup();
	Flash_Software_Init(&flash_address, &flash_page, &flash_block);

	Setup_USB_Hardware();
	usb_cdc_init();
	//USB_Init();


	/* Infinite main loop ------------------------------------------------------*/
	while (1)
	{
		/* This checks whether the USB Bus is connected */
		if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_10))
		{
			if(bDeviceState == UNCONNECTED)
			{
				USB_Init();
			}
			decode_usb_message();
		}
		else
		{
			if(bDeviceState != UNCONNECTED)
			{
				PowerOff();
				bDeviceState == UNCONNECTED;
			}
		}
		// get_batterie_voltage();
		// get_temperatur();
 		//if (get_switch(3) == 1)
		//{
		//	debounce_display = ((debounce_display << 1) | 0b00000001);
		//	if(debounce_display == 0b11111111)
		//	{
		//		switch_display = (switch_display | 0b10000000);
		//		switch_display = (switch_display & 0b10111111);
		//	}
		//}
		//else
		//{
		//	debounce_display = ((debounce_display << 1) & 0b11111110);
		//	if(debounce_display == 0b00000000)
		//	{
		//		switch_display = (switch_display | 0b01000000);
		//	}
		//}
		// Switch 2 = Display on off
		if (get_switch(2) == 1)
		{
			debounce_display = ((debounce_display << 1) | 0b00000001);
			if(debounce_display == 0b11111111)
			{
				STM_EVAL_LEDOn(LED1);
			}
		}
		else
		{
			debounce_display = ((debounce_display << 1) & 0b11111110);
			if(debounce_display == 0b00000000)
			{
				STM_EVAL_LEDOff(LED1);
			}
		}
		if (get_switch(1) == 1)
		{
			debounce_minute = ((debounce_minute << 1) | 0b00000001);
			if(debounce_minute == 0b11111111)
			{
				switch_display = (switch_display | 0b00100000);
				switch_display = (switch_display & 0b11101111);
			}
		}
		else
		{
			debounce_minute = ((debounce_minute << 1) & 0b11111110);
			if(debounce_minute == 0b00000000)
			{
				switch_display = (switch_display | 0b00010000);
			}
		}
		if ((irqvals & 0b01000000) == 0b01000000)
		{
			paint_date();
			irqvals = (irqvals & 0b10111111);
		}
		if (( irqvals & 0b00100000) == 0b00100000)
		{
			if(update_unix_time())
			{
				if(battery_logbuffer[256] != 3)
				{
					Fill_Battery_Logbuffer(battery_logbuffer);
					battery_logbuffer[257]++;
				}
				if(battery_logbuffer[257] == 31)
				{
					battery_logbuffer[256]=3;
					Flash_4Write(flash_address, battery_logbuffer);
					do
					{
						flash_block++;
						if(flash_block==1024)
						{
							flash_block=0;
							if(flash_page == 63)
							{
								flash_page=0;
							}
							else
							{
								flash_page++;
							}
							while(Check_Flash_Page_Free(flash_page))
							{
								Flash_4Page_Erase(flash_page*0x40000);
							}
						}
						flash_address=(flash_page*0x40000)+(flash_block*0x100);
					} while(Check_Flash_Block_Free(flash_address) != 0);
					battery_logbuffer[256]=0;
					battery_logbuffer[257]=0;
				}
				paint_date();
				read_voltage();
				update_batterie_display();
				paint_temperatur();
				update_water_display();
				STM_EVAL_FETToggle(FET1);
				STM_EVAL_FETToggle(FET2);
				if(togglevar==0)
				{
					write_i2c_switch(1, 0x01, 0b11111111);
					togglevar=1;
				}
				else
				{
					write_i2c_switch(1, 0x01, 0b11011111);
					togglevar=0;
				}
				irqvals = (irqvals & 0b11011111);
			}
		}
	}
}





void Timeout100usIntConfiguration(void) // wird in main ausgeführt
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

}
// initialisierung der delay funktion
void Timeout100usConfiguration(uint8_t Value)
/*~-*/
{
	uint16_t period;
	NVIC_InitTypeDef NVIC_InitStruct;
	irqvals  = (irqvals & 0b01111111);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	/*~+:Configure TIM2 for clock generation*/
	TIM_DeInit(TIM2);
	/*~+:Time base configuration*/
	period=(100*Value)-1;
	TIM_TimeBaseStructure.TIM_Period = (100*Value)-1;
	TIM_TimeBaseStructure.TIM_Prescaler = 71;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	//TIM_ClearFlag(TIM2, TIM_FLAG_Update);
	//TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	//TIM_ClearFlag(TIM2, TIM_FLAG_Update); /* clear int flag */
	//TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	TIM_Cmd(TIM2, ENABLE); /* start timer */
}

void TIM2_IRQHandler(void)
{

   //TIM_ClearFlag(TIM2, TIM_FLAG_Update); /* clear int flag */
   TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

   irqvals = irqvals | 0b10000000;
   /* stop timer */
   TIM_Cmd(TIM2, DISABLE);
}
/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
