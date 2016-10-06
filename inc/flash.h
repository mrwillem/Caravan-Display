#ifndef _FLASH_H
#define _FLASH_H

extern volatile uint8_t flash_data_buffer[256];
extern volatile uint8_t flash_sector[64];

void Flash_Hardware_Init(void);

uint8_t Flash_RDSR1(void);
uint8_t Flash_RDSR2(void);
uint8_t Flash_RDCR(void);
uint8_t Flash_WRR(uint8_t, uint8_t, uint8_t);
uint8_t Flash_WREN(void);
uint8_t Flash_WRDI(void);
uint8_t Flash_Set_256kb_Pages(void);
uint8_t Flash_4Read(uint32_t, uint16_t, uint32_t);
uint8_t Flash_4Write(uint32_t, uint32_t);
uint16_t Flash_REMS(void);
uint8_t Flash_RES(void);
uint8_t Flash_4Page_Erase(uint32_t );

uint8_t Check_Flash_Block_Free(uint32_t);
uint8_t Check_Flash_Page_Free(uint16_t);




#endif /* _FLASH_H */
