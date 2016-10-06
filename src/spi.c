#include "spi.h"
#include "hw_config.h"

volatile uint8_t GL_spi1_block;
#ifdef _USE_SPI_FLASH
volatile uint32_t flash_rx_address;
volatile uint32_t flash_tx_address;
volatile uint16_t flash_nbytes;
#endif /* _USE_SPI_FLASH */

uint8_t SPI1_send(uint8_t n_bytes, uint8_t periph, uint32_t txdata_address, uint32_t rxdata_address)
{
	uint8_t* tmp;

	tmp=txdata_address;

	/* Set DMA RX and TX Buffers */
	DMA1_Channel2->CMAR = (uint32_t)rxdata_address;
	DMA1_Channel3->CMAR = (uint32_t)txdata_address;

	DMA_SetCurrDataCounter(DMA1_Channel3, n_bytes);
	DMA_SetCurrDataCounter(DMA1_Channel2, n_bytes);

	/* Set or Reset CHIP Select depending on hardware
	 */
	switch( periph )
	{
	case SPI_BLOCK_LCD_WRITE_RAM:
		/* Set Chip Select of LCD Display */
		GPIO_WriteBit(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_RESET);
		//GPIO_WriteBit(LCD_RS_GPIO_PORT, LCD_RS_PIN, Bit_SET);
	break;
	case SPI_BLOCK_LCD_WRITE_REG:
		/* Set Chip Select and Register Select of LCD Display */
		GPIO_WriteBit(LCD_RS_GPIO_PORT, LCD_RS_PIN, Bit_RESET);
		GPIO_WriteBit(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_RESET);
	break;
	case SPI_BLOCK_FLASH_WRITE:
	case SPI_BLOCK_FLASH_COMMAND:
		/* Set Chip Select of Flash Device */
		GPIO_WriteBit(FLASH_CS_GPIO_PORT, FLASH_CS_PIN, Bit_RESET);
	break;
	/* In case of a failure
	 * Reset everything to its default state
	 */
	default:
		GPIO_WriteBit(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);
		GPIO_WriteBit(LCD_RS_GPIO_PORT, LCD_RS_PIN, Bit_SET);
		GPIO_WriteBit(FLASH_CS_GPIO_PORT, FLASH_CS_PIN, Bit_SET);
		GL_spi1_block=SPI_BLOCK_FREE;
		return 1;
	break;
	}

	DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);

	DMA_Cmd(DMA1_Channel2, ENABLE);
	DMA_Cmd(DMA1_Channel3, ENABLE);

	return 0;
}

void SPI1_BusInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	GL_spi1_block=SPI_BLOCK_FREE;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);




	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_Init(SPI1, &SPI_InitStructure);

	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);

	SPI_Cmd(SPI1, ENABLE);


	// DMA Channel 2 - SPI RX
	DMA_InitStructure.DMA_BufferSize = 0;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStructure.DMA_MemoryBaseAddr = 0x00;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DR;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_Init(DMA1_Channel2, &DMA_InitStructure);

	DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);

	// DMA Channel 3 - SPI TX
	DMA_InitStructure.DMA_BufferSize = 0;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStructure.DMA_MemoryBaseAddr = 0x00;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DR;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_Init(DMA1_Channel3, &DMA_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}

void spi_handleDMA1Ch2Interrupt(void)
{
	//Test on DMA1 Channel2 Transfer Complete interrupt
	if(DMA_GetITStatus(DMA1_IT_TC2))
	{
		//Clear DMA1 Channel1 Half Transfer, Transfer Complete and Global interrupt pending bits
		DMA_ClearITPendingBit(DMA1_IT_GL2);

		//The next line waits for rx please enable only in case of rx
		while (DMA_GetFlagStatus(DMA1_FLAG_TC3) == RESET) {}

		// wait for tx to complete - page 692
		while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) {}
		while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET) {}

		DMA_ClearFlag(DMA1_FLAG_GL3); // Clear the global flag



		// Disable DMA
		DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, DISABLE);
		// Is it important to disable the SPI DMA hardware ??
		// SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, DISABLE);
		DMA_Cmd(DMA1_Channel2, DISABLE);
		DMA_Cmd(DMA1_Channel3, DISABLE);

		while (DMA1_Channel2->CCR & DMA_CCR2_EN);           // wait until DMA is actually off
		while (DMA1_Channel3->CCR & DMA_CCR3_EN);

		// Clear the Interrupt flag
		DMA_ClearFlag(DMA1_FLAG_TC2);

		/* Release Chip Select
		 *
		 */
		switch( GL_spi1_block )
		{
		case SPI_BLOCK_LCD_WRITE_RAM:
			/* Release Chip Select of LCD Display */
			GPIO_WriteBit(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);
			GL_spi1_block=SPI_BLOCK_FREE;
		break;
		case SPI_BLOCK_LCD_WRITE_REG:
			/* Release Chip Select and Register Select of LCD Display */
			//GPIO_WriteBit(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);
			GPIO_WriteBit(LCD_RS_GPIO_PORT, LCD_RS_PIN, Bit_SET);
			GPIO_WriteBit(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);
			GL_spi1_block=SPI_BLOCK_FREE;
		break;
		case SPI_BLOCK_FLASH_WRITE:
		case SPI_BLOCK_FLASH_DATA:
			/* Release Chip Select of Flash Device */
			GPIO_WriteBit(FLASH_CS_GPIO_PORT, FLASH_CS_PIN, Bit_SET);
			GL_spi1_block=SPI_BLOCK_FREE;
		break;
		case SPI_BLOCK_FLASH_COMMAND:
			 GL_spi1_block=SPI_BLOCK_FLASH_DATA;
			/*
			 * Initiate the Flash data transfer
			 */
			/* Set FLASH Data DMA RX and TX Buffers */
			DMA1_Channel2->CMAR = (uint32_t)flash_rx_address;
			DMA1_Channel3->CMAR = (uint32_t)flash_tx_address;

			DMA_SetCurrDataCounter(DMA1_Channel3, flash_nbytes);
			DMA_SetCurrDataCounter(DMA1_Channel2, flash_nbytes);


			DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);

			DMA_Cmd(DMA1_Channel2, ENABLE);
			DMA_Cmd(DMA1_Channel3, ENABLE);

		break;
		/*
		 * In case of a failure
		 * Reset everything to its default state
		 */
		default:
			GPIO_WriteBit(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);
			GPIO_WriteBit(LCD_RS_GPIO_PORT, LCD_RS_PIN, Bit_SET);
			GPIO_WriteBit(FLASH_CS_GPIO_PORT, FLASH_CS_PIN, Bit_SET);
			GL_spi1_block=SPI_BLOCK_FREE;
		break;
		}


	}
	else
	{
		/* Should not get here */
		GL_spi1_block=SPI_BLOCK_FAILURE;
	}
}
