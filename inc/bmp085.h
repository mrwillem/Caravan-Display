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

#ifndef BMP085_H_
#define BMP085_H_





///////////////////////////////////////////////////////////////////////////////
// BMP085 Variables
///////////////////////////////////////////////////////////////////////////////

#define BMP085_ADDRESS 0xEE

///////////////////////////////////

//#define OSS 0  //  4.5 mSec conversion time (222.22 Hz)
#define OSS 1                   //  7.5 mSec conversion time (133.33 Hz)
//#define OSS 2  // 13.5 mSec conversion time ( 74.07 Hz)
//#define OSS 3  // 25.5 mSec conversion time ( 39.22 Hz)

///////////////////////////////////////

#define BMP085_PROM_START_ADDR  0xAA
#define BMP085_PROM_DATA_LEN    22

#define BMP085_T_MEASURE        0x2E    // temperature measurement
#define BMP085_P_MEASURE        0x34 + (OSS<<6) // pressure measurement
#define BMP085_CTRL_MEAS_REG    0xF4
#define BMP085_ADC_OUT_MSB_REG  0xF6

///////////////////////////////////////

#include "stm32f10x_conf.h"

void readTemperatureRequestPressure(void);
void readPressureRequestPressure(void);
void readPressureRequestTemperature(void);
void calculateTemperature(void);
void calculatePressureAltitude(void);
void initPressure(void);

void paint_pressure();


#endif /* BMP085_H_ */
