#ifndef _SPI_H
#define _SPI_H
#include "hw_config.h"
#define SPI_BLOCK_FREE 0
#define SPI_BLOCK_FAILURE 1
/*
 * For flash writes we use three different .
 * After the FLASH WRITE COMMAND STATE the interrupt handler should not pull CS High
 * but instead should initiate the data transfer
 *
 */


#define SPI_BLOCK_LCD_WRITE_REG 3
#define SPI_BLOCK_LCD_WRITE_RAM 2

extern volatile uint8_t GL_spi1_block;

#ifdef _USE_SPI_FLASH
#define SPI_BLOCK_FLASH_WRITE 4
#define SPI_BLOCK_FLASH_COMMAND 5
#define SPI_BLOCK_FLASH_DATA 6
extern volatile uint32_t flash_rx_address;
extern volatile uint32_t flash_tx_address;
extern volatile uint16_t flash_nbytes;
#endif // _USE_SPI_FLASH

uint8_t SPI1_send(uint8_t, uint8_t, uint32_t , uint32_t);
void SPI1_BusInit(void);
void spi_handleDMA1Ch2Interrupt(void);
#endif //_SPI_H
