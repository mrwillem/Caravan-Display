/**
 *   @file rtc.h
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

#ifndef RTC_H_
#define RTC_H_

/*
 * define seconds per day
 */
#define SPD 86400

/*
 * The I2C Address of the clock chip used
 *
 */
#define I2C_RTC_ADDRESS_WRITE 0b11010000
#define I2C_RTC_ADDRESS_READ 0b11010001

#include "stm32f10x_conf.h"


struct str_time
{
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t localhour;
	uint32_t unixtime;
};
typedef struct str_time time;

struct str_date
{
	uint8_t day;
	uint8_t month;
	uint8_t year;
	uint8_t weekday;
	uint8_t localday;
	uint8_t localmonth;
	uint8_t localyear;
	uint8_t localweekday;
};
typedef struct str_date date;

/* exported variables */
extern volatile time rtc_time;
extern volatile date rtc_date;
extern time rtc_prevtime;
extern date rtc_prevdate;

uint8_t DecToBcd(uint8_t val);
uint8_t BcdToDec(uint8_t val);

uint8_t rtc_init(void);

void rtc_setup(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
/*
void rtc_set_date(date* datum);
void rtc_set_time(time* zeit);
*/
void rtc_write_minute(uint8_t minute);

void rtc_enable(uint8_t e);
void rtc_write_ram(uint8_t, uint8_t);
uint8_t rtc_read_ram(uint8_t);


void rtc_read(void);

void rtc_inc(void);

uint8_t rtc_getSecond(void);
uint8_t rtc_getMinute(void);
uint8_t rtc_getHour(void);
uint8_t rtc_getDay(void);
uint8_t rtc_getMonth(void);
uint8_t rtc_getYear(void);
time rtc_getTime(void);
time* rtc_getTimeP(void);
date rtc_getDate(void);
date* rtc_getDateP(void);

void paint_date();
void rtc_write_init(void);
uint8_t isleap(uint16_t);
uint8_t daysinmonth(uint8_t, uint16_t);

void initialize_system_clock(void);

void system_clock_irqhandler(void);


uint8_t update_unix_time(void);

#endif /* RTC_H_ */
