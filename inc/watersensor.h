/*
 * batterie.h
 *
 *  Created on: 15.03.2014
 *      Author: willem
 */

#ifndef WATERSENSOR_H_
#define WATERSENSOR_H_

extern uint8_t prevlevel[2];

void watersensor_setup();
uint8_t get_waterlevel();
uint8_t get_wastelevel();
void update_water_display();
void LCD_DrawWaterTank(uint8_t Xpos, uint16_t Ypos, uint16_t emptylength, uint16_t fulllength, uint16_t color, uint16_t backcolor);

void LCD_DrawWasteTank(uint8_t Xpos, uint16_t Ypos, uint16_t color, uint16_t backcolor);
const uint8_t Wassertank[];
const uint8_t WasteWater[];
#endif /* WATERSENSOR_H_ */
