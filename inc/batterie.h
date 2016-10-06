/*
 * batterie.h
 *
 *  Created on: 02.01.2014
 *      Author: willem
 */

#ifndef BATTERIE_H_
#define BATTERIE_H_
extern uint16_t voltage;
extern int16_t current;
extern uint8_t voltage_sample_counter[2];
extern uint16_t voltage_sample[2];
extern uint8_t batstat;
extern uint8_t loading_value;
extern uint8_t batterie_charging;
extern uint16_t private_percent;
extern uint8_t battery_logbuffer[258];

#define BAT_CHARGING 1
#define BAT_NOT_CHARGING 2

void get_batterie_voltage();
void update_batterie_display();
void LCD_DrawBatterie(uint8_t Xpos, uint16_t Ypos, uint16_t length, uint16_t color, uint16_t backcolor);
uint32_t Flash_Software_Init(uint32_t *, uint16_t *, uint16_t *);
uint16_t Battery_Generate_Hash(uint8_t *);
uint8_t Fill_Battery_Logbuffer(uint8_t *);

extern const uint8_t Batterie[];


#endif /* BATTERIE_H_ */
