#include "spi.h"
#include "rtc.h"
#include "flash.h"
#include "hw_config.h"
#include "gl_fonts.h"
#include "core_cm3.h"

/*
 * The flash_data_buffer can be used by software for data,
 * this buffer can be manipulated by the software
 * the flash command buffer is to provide the address and other commands to the spi interface
 * this buffer should only be manipulated by the flash routines and thus is neither extern nor volatile
 */

volatile uint8_t flash_data_buffer[256];
uint8_t flash_rx_buffer[256];
uint8_t flash_command_buffer[6];
volatile uint8_t flash_sector[64];


/*
 * Set up Flash GPIO Ports
 * In Case of the Spansion S25FL127S it is only the Chip Select line
 * The SPI lines are configured in general SPI Configuration
 *
 */

void Flash_Hardware_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIO clock */
	RCC_APB2PeriphClockCmd(FLASH_CS_GPIO_PERIPH , ENABLE);

	/* Configure Chip Select (NCS) in Output Push-Pull mode */
	GPIO_InitStructure.GPIO_Pin = FLASH_CS_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(FLASH_CS_GPIO_PORT, &GPIO_InitStructure);

	/* Set Pin to its default state, high in Case of the Spansion S25FL127S */
	GPIO_WriteBit(FLASH_CS_GPIO_PORT, FLASH_CS_PIN, Bit_SET);
}

/*
 * The Check_Flash_Page_Free function returns 0 if
 * the Flash Page is completely 0xFF
 * otherwise the function returns 1
 */
uint8_t Check_Flash_Page_Free(uint16_t page)
{
	uint16_t j,k;
	uint8_t variable, tmp;
	uint32_t address;
	tmp=0xFF;
	/*
	 * j stands for a 256 byte block within the 256 kbyte page.
	 * Each 256 kbyte page has 1024*256 byte
	 */
	for(j=0; j<1024; j++)
	{

		/*
		 * 0x40000 = 256 kbyte , 0x100 = 256 byte
		 */
		Flash_4Read(((page*0x40000)+(j*0x100)), 256, (uint32_t) flash_data_buffer);

		/*
		 * k counts for each timestamp written within the 256 byte block
		 * a block has
		 *
		 */
		variable=0;
		while(SPI1_BLOCK != SPI_BLOCK_FREE)
		{
			if(DMA_GetITStatus(DMA1_IT_TC2))
			{
				variable=1;
			}
		}

		for(k=0; k<256; k++)
		{
			tmp = tmp & flash_data_buffer[k];
		}
	}
	if(tmp==0xFF)
		return 0;
	else
		return 1;

}

/*
 * The Check_Flash_Page_Free function returns 0 if
 * the Flash Page is completely 0xFF
 * otherwise the function returns 1
 */
uint8_t Check_Flash_Block_Free(uint32_t address)
{
	uint16_t i;
	uint8_t variable, tmp;
	tmp=0xFF;
	/*
	 * j stands for a 256 byte block within the 256 kbyte page.
	 * Each 256 kbyte page has 1024*256 byte
	 */
	Flash_4Read(address, 256, (uint32_t) flash_data_buffer);
	/*
	 * k counts for each timestamp written within the 256 byte block
	 * a block has
	 */
	variable=0;
	while(SPI1_BLOCK != SPI_BLOCK_FREE)
	{
		if(DMA_GetITStatus(DMA1_IT_TC2))
		{
			variable=1;
		}
	}
	for(i=0; i<256; i++)
	{
		tmp = tmp & flash_data_buffer[i];
	}
	if(tmp==0xFF)
		return 0;
	else
		return 1;

}




/*
 * This routine checks whether uniform 256 kb page size is set.
 * If it is not set, the routine sets 256 kb page size
 */
uint8_t Flash_Set_256kb_Pages(void)
{
	uint8_t SR1, SR2, CR;
	SR1=Flash_RDSR1();
	SR2=Flash_RDSR2();
	CR=Flash_RDCR();
	/* Test whether 256kb pages already set */
	if( (SR2 & 0b10000000) != 0b10000000)
	{
		SR2 = SR2 | 0b10000000;
		Flash_WREN();
		Flash_WRR(SR1, SR2, CR);
	}
	SR2=Flash_RDSR2();
	if( (SR2 & 0b10000000) == 0b10000000)
	{
		return 0;
	}
	return 1;
}

/*
 * For deletion of a Flash page it is neccessary to get the
 * base address of the 256 kbyte sector
 * The next function returns the base address to any given address
 */
uint32_t Flash_Get_Page_Base(uint32_t address)
{
	return 0x40000*(address/0x40000);
}

/*
 * A function to Read Electronic Manufacturer and device Signature
 */
uint16_t Flash_REMS(void)
{
	uint16_t i;
	flash_command_buffer[0]=0x90;
	for(i=1; i< 6; i++)
	{
		flash_command_buffer[i]=0x00;
	}
	while(SPI1_BLOCK != SPI_BLOCK_FREE);
	SPI1_BLOCK=SPI_BLOCK_FLASH_WRITE;
	SPI1_send(5, SPI_BLOCK_FLASH_WRITE, (uint32_t)flash_command_buffer, (uint32_t)flash_command_buffer);
	while(SPI1_BLOCK != SPI_BLOCK_FREE);
	i=(0x0000 | flash_command_buffer[4]<<8 | flash_command_buffer[5]);
	return i;

}

/*
 * A function to Read Electronic Signature
 */
uint8_t Flash_RES(void)
{
	uint8_t i;
	flash_command_buffer[0]=0xAB;
	for(i=1; i< 4; i++)
	{
		flash_command_buffer[i]=0xFF;
	}
	flash_command_buffer[4]=0x00;
	while(SPI1_BLOCK != SPI_BLOCK_FREE);
	SPI1_BLOCK=SPI_BLOCK_FLASH_WRITE;
	SPI1_send(5, SPI_BLOCK_FLASH_WRITE, (uint32_t)flash_command_buffer, (uint32_t)flash_command_buffer);
	while(SPI1_BLOCK != SPI_BLOCK_FREE);
	return flash_command_buffer[4];

}

/*
 * Write Disable
 */
uint8_t Flash_WRDI(void)
{
	flash_command_buffer[0]=0x04;
	while(SPI1_BLOCK != SPI_BLOCK_FREE);
	SPI1_BLOCK=SPI_BLOCK_FLASH_WRITE;
	SPI1_send(1, SPI_BLOCK_FLASH_WRITE, (uint32_t)flash_command_buffer, (uint32_t)flash_command_buffer);
	while(SPI1_BLOCK != SPI_BLOCK_FREE);
	return 0;
}

/*
 * Write Enable
 * Needs to be called before writing to flash memory or status register
 */
uint8_t Flash_WREN(void)
{
	flash_command_buffer[0]=0x06;
	while(SPI1_BLOCK != SPI_BLOCK_FREE);
	SPI1_BLOCK=SPI_BLOCK_FLASH_WRITE;
	SPI1_send(1, SPI_BLOCK_FLASH_WRITE, (uint32_t)flash_command_buffer, (uint32_t)flash_command_buffer);
	while(SPI1_BLOCK != SPI_BLOCK_FREE);
	return 0;
}

/*
 * Read Status Register 1
 */
uint8_t Flash_RDSR1(void)
{
	flash_command_buffer[0]=0x05;
	flash_command_buffer[1]=0x00;
	while(SPI1_BLOCK != SPI_BLOCK_FREE);
	SPI1_BLOCK=SPI_BLOCK_FLASH_WRITE;
	SPI1_send(2, SPI_BLOCK_FLASH_WRITE, (uint32_t)flash_command_buffer, (uint32_t)flash_command_buffer);
	while(SPI1_BLOCK != SPI_BLOCK_FREE);
	return flash_command_buffer[1];
}

/*
 * Read Status Register 2
 */
uint8_t Flash_RDSR2(void)
{
	flash_command_buffer[0]=0x07;
	flash_command_buffer[1]=0x00;
	while(SPI1_BLOCK != SPI_BLOCK_FREE);
	SPI1_BLOCK=SPI_BLOCK_FLASH_WRITE;
	SPI1_send(2, SPI_BLOCK_FLASH_WRITE, (uint32_t)flash_command_buffer, (uint32_t)flash_command_buffer);
	while(SPI1_BLOCK != SPI_BLOCK_FREE);
	return flash_command_buffer[1];
}
/*
 * A function to read the flash configuration register
 */
uint8_t Flash_RDCR(void)
{
	flash_command_buffer[0]=0x35;
	flash_command_buffer[1]=0x00;
	while(SPI1_BLOCK != SPI_BLOCK_FREE);
	SPI1_BLOCK=SPI_BLOCK_FLASH_WRITE;
	SPI1_send(2, SPI_BLOCK_FLASH_WRITE, (uint32_t)flash_command_buffer, (uint32_t)flash_command_buffer);
	while(SPI1_BLOCK != SPI_BLOCK_FREE);
	return flash_command_buffer[1];
}
/*
 * The WRR function writes the 2 status registers and the configuration register
 * All bits declared as reserved for future rfu must be 0
 */
uint8_t Flash_WRR(uint8_t SR1, uint8_t SR2, uint8_t CR)
{
	flash_command_buffer[0]=0x01;
	flash_command_buffer[1]=SR1;
	flash_command_buffer[2]=CR;
	flash_command_buffer[3]=SR2;
	while(SPI1_BLOCK != SPI_BLOCK_FREE);
	SPI1_BLOCK=SPI_BLOCK_FLASH_WRITE;
	SPI1_send(4, SPI_BLOCK_FLASH_WRITE, (uint32_t)flash_command_buffer, (uint32_t)flash_command_buffer);
	while(SPI1_BLOCK != SPI_BLOCK_FREE);
	return 0;
}

/*
 * A function to clear Status Register 1 in case of a failure
 *
 */
uint8_t Flash_CLSR(void)
{
	flash_command_buffer[0]=0x30;
	while(SPI1_BLOCK != SPI_BLOCK_FREE);
	SPI1_BLOCK=SPI_BLOCK_FLASH_WRITE;
	SPI1_send(1, SPI_BLOCK_FLASH_WRITE, (uint32_t)flash_command_buffer, (uint32_t)flash_command_buffer);
	while(SPI1_BLOCK != SPI_BLOCK_FREE);
	return 0;
}

uint8_t Flash_4Read(uint32_t address, uint16_t nbytes, uint32_t rxbuffer_address)
{
	uint32_t swap;
	uint32_t* addresspointer;
	flash_rx_address=rxbuffer_address;
	flash_tx_address=rxbuffer_address;
	flash_nbytes = nbytes;
	flash_command_buffer[0] = 0x13;
	addresspointer=&flash_command_buffer[1];
	swap=__REV(address);
	*addresspointer=swap;
	while(SPI1_BLOCK != SPI_BLOCK_FREE);
	SPI1_BLOCK = SPI_BLOCK_FLASH_COMMAND;
	SPI1_send(5, SPI_BLOCK_FLASH_COMMAND, (uint32_t)flash_command_buffer, (uint32_t)rxbuffer_address);
	return 0;
}

/*
 * A function that deletes the Page at the given address
 */
uint8_t Flash_4Page_Erase(uint32_t address)
{
	uint8_t tmpdata;
	Flash_WREN();
	/* Next issue the write command */
	flash_command_buffer[0]=0xDC;

	*(flash_command_buffer+1) = __REV(address);

	while(SPI1_BLOCK != SPI_BLOCK_FREE);
	SPI1_BLOCK = SPI_BLOCK_FLASH_WRITE;
	SPI1_send(5, SPI_BLOCK_FLASH_WRITE, (uint32_t)flash_command_buffer, (uint32_t)flash_rx_buffer);
	tmpdata=0;
	while(SPI1_BLOCK != SPI_BLOCK_FREE)
	{
		if(DMA_GetITStatus(DMA1_IT_TC2))
		{
			tmpdata=2;
		}
	}
	/* Next have a look whether the command was successfull */
	tmpdata=2;
	tmpdata=Flash_RDSR1();
	/* if bit 1 is set, writing is in progress */
	while(tmpdata & 0b00000001)
	{
		tmpdata=Flash_RDSR1();

	}
	/* The next case checks whether writing was impossible */
	if(tmpdata & 0b00100000)
	{
		Flash_CLSR();
		return 2;
	}
	return 0;
}

/*
 * A function to program a 256 byte block at the given address
 * the programmed block is found at the txbuffer_address
 */
uint8_t Flash_4Write(uint32_t address, uint32_t txbuffer_address)
{
	uint8_t tmpdata;
	uint16_t i;

	/* initialize variable */
	tmpdata=0xFF;
	/* find out whether block is free */
	Flash_4Read(address, 256, (uint32_t) flash_rx_buffer);
	i=0;
	while(SPI1_BLOCK != SPI_BLOCK_FREE)
	{
		if(DMA_GetITStatus(DMA1_IT_TC2))
		{
			i=1;
		}
	}
	for(i=0; i<256; i++)
	{
		tmpdata= tmpdata & flash_rx_buffer[i];
	}
	if(tmpdata != 0xFF)
	{
		/* Flash Block not empty */
		return 1;
	}
	/* Set up the data to write */
	/* Always write complete Page of 256 byte */
	flash_nbytes = 256;
	flash_rx_address=txbuffer_address;
	flash_tx_address=txbuffer_address;
	/* First of all send just the Write Enable */
	Flash_WREN();
	/* Next issue the write command */
	flash_command_buffer[0]=0x12;

	*(flash_command_buffer+1) = __REV(address);

	while(SPI1_BLOCK != SPI_BLOCK_FREE);
	SPI1_BLOCK = SPI_BLOCK_FLASH_COMMAND;
	SPI1_send(5, SPI_BLOCK_FLASH_COMMAND, (uint32_t)flash_command_buffer, (uint32_t)flash_command_buffer);
	i=0;
	while(SPI1_BLOCK != SPI_BLOCK_FREE)
	{
		if(DMA_GetITStatus(DMA1_IT_TC2))
		{
			i=2;
		}
	}
	/* Next have a look whether the command was successfull */
	i=2;
	tmpdata=Flash_RDSR1();
	/* if bit 1 is set, writing is in progress */
	while(tmpdata & 0b00000001)
	{
		tmpdata=Flash_RDSR1();

	}
	/* The next case checks whether writing was impossible */
	if(tmpdata & 0b00100000)
	{
		Flash_CLSR();
		return 2;
	}
	return 0;

}

