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
#include "temperatur.h"
#include "adc.h"
#include "gl_fonts.h"
#include "batterie.h"
#include <stdint.h>

uint16_t temperatur;

void get_temperatur()
{
	uint16_t val;
	/* 3,3 Volt reference voltage */
	/* Adjustment Factor is set in hw_config.h */
	val=((ADC_BASE_VOLTAGE+TEMP_ADJUST_FACTOR)*(ADCBuffer[1]-ADC_OFFSET))>>12;
	val=val-500;
	if(RESET_VALUES==1)
	{
		temperatur=val;
		RESET_VALUES=0;
	}
	else
	{
		temperatur=(temperatur+val)>>1;

	}
}
void paint_temperatur(void)
{
	uint16_t tempdata;
	LCD_SetFont(&GL_Font8Bit43x50);
	if(current < 0)
	{
		/* discharging */
		LCD_Display8BitChar(180,0,'d');
		tempdata=(uint16_t)((~current)+1);
	}
	else
	{
		/* charging */
		LCD_Display8BitChar(180,0,'c');
		tempdata=(uint16_t)current;

	}
	tempdata*=4;
	/* temp might be adjusted by interrupts so make a quick change */

	uint8_t temperatur_data[5];
	temperatur_data[0]=(tempdata%10);
	tempdata=tempdata/10;
	/* Set a fixed ',' in position one of the Array */
	temperatur_data[1]=(tempdata%10);
	tempdata=tempdata/10;
    temperatur_data[2]=(tempdata%10);
	tempdata=tempdata/10;
	/* 10 is the [SPACE] sign in this case */
	/* So we just add leading spaces instead of zeros */
	if(tempdata == 0)
	{
		temperatur_data[3]=10;

	}
	else
	{
		temperatur_data[3]=(tempdata%10);
		tempdata=tempdata/10;
	}
	if(tempdata == 0)
	{
		temperatur_data[4]=10;

	}
	else
	{
		temperatur_data[4]=(tempdata%10);
	}

	/* Select very Big 8Bit special font */
	LCD_SetFont(&GL_Font8Bit43x50);
	LCD_Display8BitChar(80 ,0,(32+temperatur_data[4]));
	LCD_Display8BitChar(100 ,0,(32+temperatur_data[3]));
	LCD_Display8BitChar(120,0,(32+temperatur_data[2]));
	LCD_Display8BitChar(140,0,(32+temperatur_data[1]));
	LCD_Display8BitChar(160,0,(32+temperatur_data[0]));

}
