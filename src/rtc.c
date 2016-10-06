/**
 *   @file rtc.c
 *
 *   @date 05.07.2012
 * @author Pascal Gollor
 *     web http://www.pgollor.de
 *
 * @copyright Dieses Werk ist unter einer Creative Commons Lizenz vom Typ
 * Namensnennung - Nicht-kommerziell - Weitergabe unter gleichen Bedingungen 3.0 Deutschland zugänglich.
 * Um eine Kopie dieser Lizenz einzusehen, konsultieren Sie
 * http://creativecommons.org/licenses/by-nc-sa/3.0/de/ oder wenden Sie sich
 * brieflich an Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 *
 * -- englisch version --
 * @n This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Germany License. To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 *
 */

#include "rtc.h"

#include "i2c.h"

#include "gl_fonts.h"

#include "main.h"

#include "settings.h"


/**
 * @brief system time
 */
volatile time rtc_time;
time rtc_prevtime;
/**
 * @brief system date
 */
volatile date rtc_date;
date rtc_prevdate;

/**
 * @brief init flag
 */
uint8_t init_rtc = 0;
uint8_t databuffer[8];
uint8_t oldsecond;

/**
 * decimal to binary
 * @param val : decimal
 * @return binary
 */
uint8_t DecToBcd(uint8_t val)
{
	return (val/10*16) + (val%10);
}

/**
 * binary to decimal
 * @param val : binary
 * @return decimal
 */
uint8_t BcdToDec(uint8_t val)
{
	return (val/16*10) + (val%16);
}


/**
 * initialize the RTC device on startup
 * @retval 0: accessible
 * @retval 1: failed to access device
 */
uint8_t rtc_init(void)
{
	init_rtc = 0;

	rtc_time.hour = 0;
	rtc_time.minute = 0;
	rtc_time.second = 0;
	rtc_date.day = 0;
	rtc_date.month = 0;
	rtc_date.weekday = 0;
	rtc_date.year = 0;

	while(I2C2_BUSY);
	I2C2_BUSY=1;
	Buffer_Tx2[0]=0x0E;
	I2C_Master_BufferWrite(I2C2, databuffer, 1, I2C_RTC_ADDRESS_WRITE);
	I2C_Master_BufferRead(I2C2, databuffer,5, I2C_RTC_ADDRESS_READ);
	if ( (Buffer_Rx2[0] & 0b10000000) == 0b10000000 )
	{
		Buffer_Tx2[0]=0x0E;
		Buffer_Tx2[1]=0b00000000;
		//EOSC Set
		I2C_Master_BufferWrite(I2C2, databuffer, 2, I2C_RTC_ADDRESS_WRITE);
	}
	I2C2_BUSY=0;
	init_rtc = 1;
	rtc_read();


	return 1;
}

/**
 * @brief enable RTC device
 * @param e: enable
 *
 * e = 1: enable
 * e = 0: disable
 */
void rtc_enable(uint8_t e)
{
	while(I2C2_BUSY);
	I2C2_BUSY=1;
	uint8_t second = rtc_read_ram(0x00);
	if (e == 1)
	{
		if(second & 0b10000000)
		{
			second = second & 0b01111111;
			Buffer_Tx2[0]=0x00;
			Buffer_Tx2[1]=second;
			I2C_Master_BufferWrite(I2C2, databuffer, 2, I2C_RTC_ADDRESS_WRITE);
		}
	}
	else if (e == 0)
	{
		if ((second & 0b10000000) == 0 )
		{
			second = second | 0b10000000;
			Buffer_Tx2[0]=0x00;
			Buffer_Tx2[1]=second;
			I2C_Master_BufferWrite(I2C2, databuffer, 2,  I2C_RTC_ADDRESS_WRITE);
		}
	}
	I2C2_BUSY=0;
}

/**
 * @brief read from RTC RAM
 * @param addr : address
 * @return value from address
 */
uint8_t rtc_read_ram(uint8_t addr)
{
	while(I2C2_BUSY);
	I2C2_BUSY=1;
	//databuffer[0]=addr;
	//I2C_Master_BufferWrite(I2C2, databuffer, 1,  I2C_RTC_ADDRESS_WRITE);
	Buffer_Tx2[0]=addr;
	I2C_Master_BufferWrite(I2C2, databuffer, 1,  I2C_RTC_ADDRESS_WRITE);

	I2C_Master_BufferRead(I2C2, databuffer, 1,  I2C_RTC_ADDRESS_READ);

	I2C2_BUSY=0;
	return (uint8_t) Buffer_Rx2[0];
}

/**
 * @brief write to RTC RAM
 * @param addr : address
 * @param val : value to write
 */
void rtc_write_ram(uint8_t addr, uint8_t val)
{

	if (init_rtc == 0)
		return;
	while(I2C2_BUSY);
	I2C2_BUSY=1;
	Buffer_Tx2[0]=addr;
	Buffer_Tx2[1]=val;
	I2C_Master_BufferWrite(I2C2, databuffer, 2,  I2C_RTC_ADDRESS_WRITE);
	I2C2_BUSY=0;

}

void rtc_write_init(void)
{
	while(I2C2_BUSY);
	I2C2_BUSY=1;
	// year
	Buffer_Tx2[7]=0b00010110;
	// month
	Buffer_Tx2[6]=0b00000110;
	// day
	Buffer_Tx2[5]=0b00010110;
	// dayofweek 1= monday
	Buffer_Tx2[4]=0b00000100;
	//hours
	Buffer_Tx2[3]=0b00000001;
	// minutes
	Buffer_Tx2[2]=0b00010100;
	// seconds
	Buffer_Tx2[1]=0b00000000;
	// address
	Buffer_Tx2[0]=0x00;
	I2C_Master_BufferWrite(I2C2, databuffer, 8, I2C_RTC_ADDRESS_WRITE);
	I2C2_BUSY=0;
}

void rtc_write_minute(uint8_t minute)
{
	while(I2C2_BUSY);
	I2C2_BUSY=1;
	// minutes
	Buffer_Tx2[1]=DecToBcd(minute);
	// address of minute register
	Buffer_Tx2[0]=0x01;
	I2C_Master_BufferWrite(I2C2, databuffer, 2, I2C_RTC_ADDRESS_WRITE);
	I2C2_BUSY=0;
}
/**
 * @brief read date and time from RTC Device
 *
 * save date and time in global vars
 */
void rtc_read(void)
{
	uint8_t tmp;
	if (init_rtc == 0)
		return;
	while(I2C2_BUSY);
	I2C2_BUSY=1;
	Buffer_Tx2[0]=0x00;
	I2C_Master_BufferWrite(I2C2, databuffer, 1,  I2C_RTC_ADDRESS_WRITE);
	I2C_Master_BufferRead(I2C2, databuffer, 7, I2C_RTC_ADDRESS_READ);
	for(tmp=0; tmp<8; tmp++)
	{
		databuffer[tmp]=Buffer_Rx2[tmp];
	}
	tmp=(databuffer[0]&0b01111111);
	rtc_time.second = BcdToDec(tmp);

	tmp=(databuffer[1]&0b01111111);
	rtc_time.minute = BcdToDec(tmp);

	tmp=(databuffer[2]&0b00111111);
	rtc_time.hour=BcdToDec(tmp);
	if((timezone<0) || (timezone>23))
	{
		timezone=0;
	}
	rtc_time.localhour=rtc_time.hour+timezone;
	if(rtc_time.localhour > 23)
	{
		rtc_time.localhour-=24;
		rtc_date.localday=rtc_date.day+1;
		rtc_date.localweekday=rtc_date.weekday+1;
		if(rtc_date.day+1 > daysinmonth(rtc_date.month, rtc_date.year))
		{
			rtc_date.localday=1;
			if(rtc_date.month+1 > 12)
			{
				rtc_date.localmonth=1;
				rtc_date.localyear=rtc_date.year+1;
			}
			else
			{
				rtc_date.localmonth=rtc_date.month;
				rtc_date.localyear=rtc_date.year;
			}
		}
		else
		{
			rtc_date.localday=rtc_date.day+1;
		}
	}

	tmp=(databuffer[3]&0b00000111);
	rtc_date.weekday = BcdToDec(tmp);

	tmp=(databuffer[4]&0b00111111);
	rtc_date.day = BcdToDec(tmp);

	tmp=(databuffer[5]&0b00011111);
	rtc_date.month = BcdToDec(tmp);
	tmp=databuffer[6];
	rtc_date.year = BcdToDec(tmp);
	I2C2_BUSY=0;
}


void paint_date()
{
	uint8_t timedata;
	/* temp might be adjusted by interrupts so make a quick change */
	uint8_t timedisplay_data[8];
	rtc_read();

	timedata=rtc_time.second;
	timedisplay_data[0]=(timedata%10);
	timedata=timedata/10;
	timedisplay_data[1]=(timedata%10);
	timedata=rtc_time.minute;
	timedisplay_data[2]=(timedata%10);
	timedata=timedata/10;
	timedisplay_data[3]=(timedata%10);
	timedata=rtc_time.localhour;
	timedisplay_data[4]=(timedata%10);
	timedata=timedata/10;
	timedisplay_data[5]=(timedata%10);
	/* Select very Big 8Bit special font */
	LCD_SetFont(&GL_Font8Bit43x50);
	if(rtc_prevtime.hour != rtc_time.hour)
	{
		rtc_prevtime.hour = rtc_time.hour;
		LCD_Display8BitChar(80 ,70,(32+timedisplay_data[5]));
		LCD_Display8BitChar(100 ,70,(32+timedisplay_data[4]));
		LCD_Display8BitChar(120, 70,(32+26));
	}
	if(rtc_prevtime.minute != rtc_time.minute)
	{
		rtc_prevtime.minute = rtc_time.minute;
		LCD_Display8BitChar(140 ,70,(32+timedisplay_data[3]));
		LCD_Display8BitChar(160 ,70,(32+timedisplay_data[2]));
		LCD_Display8BitChar(180, 70,(32+26));
	}
	if(rtc_prevtime.second != rtc_time.second)
	{
		LCD_Display8BitChar(200 ,70,(32+timedisplay_data[1]));
		LCD_Display8BitChar(220 ,70,(32+timedisplay_data[0]));
	}
	if(rtc_prevdate.weekday != rtc_date.weekday)
	{
		rtc_prevdate.weekday = rtc_date.weekday;
		/* Prepare the Date */
		switch(rtc_date.weekday)
		{
			case 1:{
				// Codes for Monday
				timedisplay_data[8]=27;
				timedisplay_data[7]=31;
				break;
			}
			case 2:{
				// Tuesday
				timedisplay_data[8]=28;
				timedisplay_data[7]=32;
				break;
			}
			case 3:{
				// Wednesday
				timedisplay_data[8]=27;
				timedisplay_data[7]=32;
				break;
			}
			case 4:{
				// Thursday
				timedisplay_data[8]=28;
				timedisplay_data[7]=31;
				break;
			}
			case 5:{
				// Friday
				timedisplay_data[8]=29;
				timedisplay_data[7]=33;
				break;
			}
			case 6:{
				// Saturday
				timedisplay_data[8]=30;
				timedisplay_data[7]=34;
				break;
			}
			case 7:{
				// Sunday
				timedisplay_data[7]=30;
				timedisplay_data[8]=31;
				break;
			}
		}
		//LCD_Display8BitChar(80 ,20,(32+timedisplay_data[8]));
		//LCD_Display8BitChar(100 ,20,(32+timedisplay_data[7]));
		//LCD_Display8BitChar(120, 20,(32+10));
	}
	if(rtc_prevdate.day != rtc_date.day)
	{
		rtc_prevdate.day = rtc_date.day;
		timedata=rtc_date.day;
		timedisplay_data[5]=(timedata%10);
		timedata=timedata/10;
		timedisplay_data[6]=(timedata%10);
		//LCD_Display8BitChar(140 ,20,(32+timedisplay_data[6]));
		//LCD_Display8BitChar(160 ,20,(32+timedisplay_data[5]));
		//LCD_Display8BitChar(180, 20,(32+10));

	}
	if(rtc_prevdate.month != rtc_date.month)
	{
		rtc_prevdate.month = rtc_date.month;
		timedata=rtc_date.month;
		timedisplay_data[3]=(timedata%10);
		timedata=timedata/10;
		timedisplay_data[4]=(timedata%10);
		//LCD_Display8BitChar(200 ,20,(32+timedisplay_data[4]));
		//LCD_Display8BitChar(220 ,20,(32+timedisplay_data[3]));
	}


	//LCD_Display8BitChar(80 ,30,(32+timedisplay_data[6]));

	LCD_SetFont(&GL_Font16x24);
}

/*
 * A Function to get the unix timestamp from current date
 * It returns the seconds passed since 1st January 1970 00:00:00
 */
uint32_t synchronize_unixtime()
{
	uint32_t timestamp;
	uint16_t i;
	uint16_t year;
	timestamp=0;
	year=2000+rtc_date.year;
	for(i=1970;i<year;i++)
	{
		if(isleap(i))
			timestamp += 366*SPD;
		else
			timestamp += 365*SPD;
	}
	for(i=1;i<rtc_date.month;i++)
	{
		timestamp +=((daysinmonth(i,year))*SPD);
	}
	for(i=1;i<rtc_date.day;i++)
	{
		timestamp+=SPD;
	}
	for(i=0;i<rtc_time.hour;i++)
	{
		timestamp+=3600;
	}
	for(i=0;i<rtc_time.minute;i++)
	{
		timestamp+=60;
	}
	timestamp+=rtc_time.second;
	return timestamp;
}

/*
 * The function returns the number of days of the month
 */
uint8_t daysinmonth(uint8_t month, uint16_t year)
{
	if (month == 4 || month == 6 || month == 9 || month == 11)
		return 30;
	else if (month == 2)
	{
		if (isleap(year))
			return 29;
		else
			return 28;
	}
	else
		return 31;
}

/*
 * The function returns either 1 if the year is a leap year
 * or 0 if the year is no leap year
 */
uint8_t isleap(uint16_t year)
{
	if ( year%400 == 0)
		return 1;
	else if ( year%100 == 0)
		return 0;
	else if ( year%4 == 0 )
		return 1;
	else
		return 0;
}




uint8_t update_unix_time(void)
{
	rtc_read();
	if(rtc_time.second == oldsecond)
	{
		return 0;
	}
	if(rtc_time.second == oldsecond+1)
	{
		rtc_time.unixtime++;
		oldsecond=rtc_time.second;
		return 0;
	}
	else if(rtc_time.second != oldsecond)
	{
		rtc_time.unixtime=synchronize_unixtime();
	}
	return 1;
}

/*
 * Setup a interrupt on a one second base
 */
void initialize_system_clock(void)
{
	/* Setup a Timer each 500 milli seconds */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBase_InitStructure;
    TIM_TimeBase_InitStructure.TIM_Prescaler = 17999;
    TIM_TimeBase_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBase_InitStructure.TIM_Period = 1999;
    TIM_TimeBase_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBase_InitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBase_InitStructure);

    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    /* Enable the Timer Interrupt */
    NVIC_InitTypeDef nvicStructure;
    nvicStructure.NVIC_IRQChannel = TIM3_IRQn;
    nvicStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
    nvicStructure.NVIC_IRQChannelSubPriority = 0x0F;
    nvicStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicStructure);

    /* All is set up, enable timer */
    TIM_Cmd(TIM3, ENABLE);
}

/*
 * Interrupt handler for system clock
 */
void system_clock_irqhandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        irqvals =  (irqvals | 0b00100000);
    }
}
