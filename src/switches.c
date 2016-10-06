/*
 * batterie.c
 *
 *  Created on: 02.01.2014
 *      Author: willem
 */

#include "main.h"
#include "touchscreen.h"
#include "hw_config.h"
#include "cursor.h"
#include "graphicObject.h"
#include "pictures.h"
#include "uiframework.h"
#include "batterie.h"
#include "switches.h"
#include <stdint.h>
#include "gl_fonts.h"


void SWITCH_Setup()
{
	// Switches are on PB5, PB6, PB7, PB8 and PB9
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

}

uint8_t get_switch(uint8_t SWITCH)
{
	uint8_t retval;
	retval = 0;
	if (SWITCH == 1)
	{
		retval = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5);
	}
	if (SWITCH == 2)
	{
		retval = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6);
	}
	if (SWITCH == 3)
	{
		retval = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7);
	}
	if (SWITCH == 4)
	{
		retval = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8);
	}
	if (SWITCH == 5)
	{
		retval = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9);
	}
	return retval;
}



