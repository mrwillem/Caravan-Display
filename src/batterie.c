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
#include "batterie.h"
#include "adc.h"
#include "flash.h"
#include "spi.h"
#include "rtc.h"
#include "i2c.h"
#include "bq34z100g1.h"
#include <stdint.h>
#include "gl_fonts.h"

uint16_t voltage, voltage_sample[2];
int16_t current;
uint8_t voltage_sample_counter[2];
uint8_t batstat;
uint8_t loading_value;
uint8_t batterie_charging;
uint16_t private_percent;
uint8_t battery_logbuffer[258];

void get_batterie_voltage()
{

	// Calculations are in mV, 5,93 is about 1210 / 205 kOHM for the resistors,
	// The actual value is 5.93 for this specific board because of inaccuracies
	// 3330 mv is the reference voltage for ADC conversion
	//
	// The actual value is filtered digitally by the averaging of 16 x 16 values
	// So it is like a second order filter which gives a high precision noise filter
	// The "experimental" uncertainty is only 5 mv
	if (voltage_sample_counter[1] == 15)
	{
		voltage_sample_counter[1]=0;
		voltage_sample[1]=voltage_sample[1]>>4;
		if (voltage_sample_counter[2] == 15)
		{
			voltage_sample_counter[2]=0;
			voltage_sample[2]=voltage_sample[2]>>4;
			voltage = (uint16_t) (5.93*((3330*voltage_sample[2])>>12));
			voltage_sample[2]=voltage_sample[1];
		}
		else
		{
			voltage_sample[2] += voltage_sample[1];
			voltage_sample_counter[2]++;
		}
		voltage_sample[1]=ADCBuffer[0];
	}
	else
	{
		voltage_sample[1] += ADCBuffer[0];
		voltage_sample_counter[1]++;
	}

	//voltage_samples[voltage_sample] = (uint16_t) (5.96*((3330*ADCBuffer[0])>>12));


	if(RESET_VALUES==1)
	{
		//voltage_sample=voltage_sample>>4;
		//voltage = (uint16_t) (5.96*((3330*voltage_sample)>>12));
		// bubble sort the values
		//for(i=0; i<41; i++)
		//{
		//	j=i;
		//	while( j < 39)
		//	{
		//		if( voltage_samples[j] > voltage_samples[j+1])
		//		{
		//			tmp=voltage_samples[j+1];
		//			voltage_samples[j+1]=voltage_samples[j];
		//			voltage_samples[j]=tmp;
		//		}
		//		j++;
		//	}

		//}
		//for(i=16; i<25; i++)
		//{
			//voltage_samples[15]=(voltage_samples[15]+voltage_samples[i])>>1;
		//}
		//voltage=voltage_samples[15];
		RESET_VALUES=0;

	}

}

uint8_t calculate_batterie_percent()
{
	// Calculation of the batterie charging state is difficult because
	// the voltage of the batterie changes when power consumption increases
	// The following voltage values resemble the nominal charging capacity when the
	// batterie idles for at least 5 hours ...
	// ... which of course is never achieveable in a camping van.
	// 100 % -> 12,78 V
	// 75 % -> 12,55 V
	// 50 % -> 12,3 V
	// 25 % -> 12 V
	// 0% -> 11,76 V
	// Thus the values have to be adapted and the actual displayed value will never
	// show the "real" remaining percentage of the batterie.
	// The solar charging device cuts the power off at about 11,2 or 11,6 volts
	// depending on the power consumption of the system.
	// Since the power consumption monitors only the power port which in my case
	// drives a power mosfet (BTS555) which has current sensing function
	if(current > 0)
	{
		batterie_charging=BAT_CHARGING;
	}
	else
	{
		batterie_charging=BAT_NOT_CHARGING;
	}
	if(voltage > 13049)
	{
		return 100;
	}
	if((voltage <13050)&&(voltage>12549))
	{
		return (uint8_t) (75+(((voltage-12550)*25)/500));
	}
	if((voltage < 12550) && (voltage > 12299))
	{
		return (uint8_t) (50+(((voltage-12300)*25)/250));
	}
	if((voltage < 12300) && (voltage > 11999))
	{
		return (uint8_t) (25+(((voltage-12000)*25)/300));
	}
	if((voltage < 12000) && (voltage > 11759))
	{
		return (uint8_t) (0+(((voltage-11760)*25)/240));
	}
	if(voltage < 11760)
	{
		return 0;
	}
	return 0;
}

void update_batterie_display()
{

	uint16_t percent=0;
	uint16_t powerlines=0;
	uint16_t color=0;
	uint8_t xcoord, npix;
	int index, i;
	char text[4];
	if(batterie_charging == BAT_CHARGING)
	{
		loading_value+=20;
		if(loading_value > 101)
		{
			loading_value = 20;
		}
		percent=loading_value;
		calculate_batterie_percent();
	}
	else
	{
		percent=calculate_batterie_percent();

	}

	// update the display only if the values have changed
	if(private_percent != percent)
	{
		private_percent = percent;


		if(private_percent <= 11) // Red Color
		{
				color=GL_Red;
				if((batstat==0) || (batstat==3))
				{
					LCD_DrawBatterie(0x3, 0x4, 277, GL_Red, GL_White);
					batstat=1;
				}
		}
		else
		{
			if((batstat==1) || (batstat==3))
			{
				LCD_DrawBatterie(0x3, 0x4, 277, GL_Black, GL_White);
				batstat=0;
			}
			if(private_percent <= 26) // Yellow Color
			{
				if( batterie_charging == BAT_CHARGING )
				{
					color = GL_Green;
				}
				else
				{
					color=GL_Yellow;
				}
			}
			else
			{
				color=GL_Green;
			}

		}
		powerlines=(int)((3*private_percent)+0.5);

		for(i=300; i>(300-powerlines); i--)
		{
			switch(i)
			{
				case(1): xcoord=29; npix=17; break;
				case(2): xcoord=29; npix=17; break;
				case(3): xcoord=29; npix=17; break;
				case(4): xcoord=29; npix=17; break;
				case(5): xcoord=29; npix=17; break;
				case(6): xcoord=29; npix=17; break;
				case(7): xcoord=29; npix=17; break;
				case(8): xcoord=29; npix=17; break;
				case(9): xcoord=29; npix=17; break;
				case(10): xcoord=29; npix=17; break;
				case(11): xcoord=29; npix=17; break;
				case(12): xcoord=29; npix=17; break;
				case(13): xcoord=29; npix=17; break;
				case(14): xcoord=29; npix=17; break;
				case(15): xcoord=29; npix=17; break;
				case(16): xcoord=29; npix=17; break;
				case(17): xcoord=29; npix=17; break;
				case(18): xcoord=10; npix=55; break;
				case(19): xcoord=9; npix=57; break;
				case(20): xcoord=8; npix=59; break;
				case(298): xcoord=8; npix=59; break;
				case(299): xcoord=9; npix=57; break;
				case(300): xcoord=10; npix=55; break;
				default: xcoord=7; npix=61; break;
			}
			LCD_SetCursor((3+xcoord), (i+9));

			for(index = 0; index < npix; index++)
			{
				LCD_WriteRAM(color>>8);
				LCD_WriteRAM(color);
			}
		}
		color=GL_White;
		for(i=(300-powerlines); i>0; i--)
		{
			switch(i)
			{
				case(1): xcoord=29; npix=17; break;
				case(2): xcoord=29; npix=17; break;
				case(3): xcoord=29; npix=17; break;
				case(4): xcoord=29; npix=17; break;
				case(5): xcoord=29; npix=17; break;
				case(6): xcoord=29; npix=17; break;
				case(7): xcoord=29; npix=17; break;
				case(8): xcoord=29; npix=17; break;
				case(9): xcoord=29; npix=17; break;
				case(10): xcoord=29; npix=17; break;
				case(11): xcoord=29; npix=17; break;
				case(12): xcoord=29; npix=17; break;
				case(13): xcoord=29; npix=17; break;
				case(14): xcoord=29; npix=17; break;
				case(15): xcoord=29; npix=17; break;
				case(16): xcoord=29; npix=17; break;
				case(17): xcoord=29; npix=17; break;
				case(18): xcoord=10; npix=55; break;
				case(19): xcoord=9; npix=57; break;
				case(20): xcoord=8; npix=59; break;
				case(298): xcoord=8; npix=59; break;
				case(299): xcoord=9; npix=57; break;
				case(300): xcoord=10; npix=55; break;
				default: xcoord=7; npix=61; break;
			}
			LCD_SetCursor((3+xcoord), (i+9));
			for(index = 0; index < npix; index++)
			{
				LCD_WriteRAM(color>>8);
				LCD_WriteRAM(color);
			}
		}

		i=0;
		//DEBUG
		private_percent=voltage;

		percent=private_percent;
		LCD_SetFont(&GL_Font8Bit43x50);
		do

		{
			if(private_percent<1000)
					text[3]=32+10;
			if(private_percent<100)
				text[2]=32+10;
				//LCD_Display8BitChar(110,130,(32+10));
			if(private_percent<10)
			{
				text[1]=32+10;
				//LCD_Display8BitChar(130,130,(32+10));
				if(private_percent==0)
				{
					text[i]=32+0;

					//LCD_Display8BitChar(150-(20*i),130,text[i]);
				}
				else
				{
					text[i] = 32+(percent % 10);
					//LCD_Display8BitChar(150-(20*i),130,text[i]);
				}
				i++;
				text[i]=32+0;
				//LCD_Display8BitChar(150-(20*i),130,text[i]);
				break;
			}
			else
			{
				text[i] = 32+(percent % 10);
				//LCD_Display8BitChar(150-(20*i),130,text[i]);
				i++;
			}
			percent/=10;
		}while(percent>0);
		for(i=4; i>-1; i--)
		{
			LCD_Display8BitChar(170-(20*i),30,text[i]);
		}
		LCD_Display8BitChar(190,30,(16+32));
		LCD_SetFont(&GL_Font16x24);
	}



}


void LCD_DrawBatterie(uint8_t Xpos, uint16_t Ypos, uint16_t length, uint16_t color, uint16_t backcolor)
{
  uint32_t index = 0, i = 0, row=0,  temp=0;
  uint16_t line=0;
  uint8_t  Xaddress = 0;
  Xaddress = Xpos;

  LCD_SetCursor(Xaddress, Ypos);
  /* Write Header */

  for(index = 0; index < 260; temp++)
  {

    for(i = 0; i < 75; temp++)
    {
    	for (row=0; row<8; row++)
    	{
    		if((((Batterie[index] & ((0x80 << ((75 / 12 ) * 8 ) ) >> row)) == 0x00) &&(75 <= 12))||
    		(((Batterie[index] & (0x1 << row)) == 0x00)&&(75 > 12 )))

    		{
    			LCD_WriteRAM(backcolor>>8);
    			LCD_WriteRAM(backcolor);
    		}
    		else
    		{
    			LCD_WriteRAM(color>>8);
    			LCD_WriteRAM(color);
    		}
    		i++;
    	}
    	index++;
    }

    //Xaddress++;
    Ypos++;
    LCD_SetCursor(Xaddress, Ypos);
  }
  /* Write Middle Part */

  for(line = 0; line < length; line++)
  {
	index=260;
    for(i = 0; i < 75; temp++)
    {
    	for (row=0; row<8; row++)
    	{
    		if((((Batterie[index] & ((0x80 << ((75 / 12 ) * 8 ) ) >> row)) == 0x00) &&(75 <= 12))||
    		(((Batterie[index] & (0x1 << row)) == 0x00)&&(75 > 12 )))

    		{
    			LCD_WriteRAM(backcolor>>8);
    			LCD_WriteRAM(backcolor);
    		}
    		else
    		{
    			LCD_WriteRAM(color>>8);
    			LCD_WriteRAM(color);
    		}
    		i++;
    	}
    	index++;
    }

    //Xaddress++;
    Ypos++;
    LCD_SetCursor(Xaddress, Ypos);
  }
  /* Write Bottom Part */
  temp=0;
  for(index = 270; index < 380; temp++)
  {
    for(i = 0; i < 75; temp++)
    {
    	for (row=0; row<8; row++)
    	{
    		if((((Batterie[index] & ((0x80 << ((75 / 12 ) * 8 ) ) >> row)) == 0x00) &&(75 <= 12))||
    		(((Batterie[index] & (0x1 << row)) == 0x00)&& (75 > 12 )))

    		{
    			LCD_WriteRAM(backcolor>>8);
    			LCD_WriteRAM(backcolor);
    		}
    		else
    		{
    			LCD_WriteRAM(color>>8);
    			LCD_WriteRAM(color);
    		}
    		i++;
    	}
    	index++;
    }

    //Xaddress++;
    Ypos++;
    LCD_SetCursor(Xaddress, Ypos);
  }
}
/*
 * A Function that assembles a 31 second logbuffer block to be transmitted to flash memory
 */
uint8_t Fill_Battery_Logbuffer(uint8_t *memory)
{


	uint32_t *wordpointer;
	uint16_t *intpointer;
	uint16_t volt;
	int16_t mAmps;
	uint16_t remaining;
	uint16_t temp;
	while(I2C2_BUSY);
	I2C2_BUSY=1;
	Buffer_Tx2[0]=0x04;
	I2C_Master_BufferWrite(I2C2, Buffer_Tx2, 1,  BQ34Z100G1_WRITE_ADDRESS);
	I2C_Master_BufferRead(I2C2, Buffer_Rx2, 16, BQ34Z100G1_READ_ADDRESS);

	/* Fill all values at once with the buffer returned by read operation */
	volt=Buffer_Rx2[4] | (Buffer_Rx2[5]<<8);
	mAmps=Buffer_Rx2[12] | (Buffer_Rx2[13]<<8);
	remaining=Buffer_Rx2[0] | (Buffer_Rx2[1]<<8);
	temp=Buffer_Rx2[8] | (Buffer_Rx2[9]<<8);

	I2C2_BUSY=0;
	switch(memory[257])
	{
	case 0:
		wordpointer = memory;
		*wordpointer = rtc_time.unixtime;
		memory[4]=0xAB;
		memory[5]=0xCD;
		memory[6]=0x01;
		memory[7]=0x04;
		/*
		 * The dataset pointer gets incremented here because in case of one only the header is generated.
		 * First data is filled in directly after header generation.
		 * To avoid overwriting of the first dataset and to make things easier
		 * the switch block is not interrupted here. After case 1, case 2 is automagically executed.
		 * Because of the incrementation here case 3 is executed the second time this function is called
		 */
		memory[257]++;
	case 1:
		intpointer = (memory+(memory[257]*8));
		*intpointer =remaining;
		intpointer = memory+((memory[257]*8)+2);
		*intpointer=volt;
		intpointer = memory+((memory[257]*8)+4);
		*intpointer = mAmps;
		intpointer = memory+((memory[257]*8)+6);
		*intpointer = temp;

		break;
	default:
		intpointer = (memory+(memory[257]*8));
		*intpointer =remaining;
		intpointer = memory+((memory[257]*8)+2);
		*intpointer=volt;
		intpointer = memory+((memory[257]*8)+4);
		*intpointer = mAmps;
		intpointer = memory+((memory[257]*8)+6);
		*intpointer = temp;
		break;
	}
	if(memory[257]==30)
	{
		intpointer=(memory+254);
		*intpointer=Battery_Generate_Hash(memory);
	}
	return 0;
}

/*
 * Flash Software Init scans the flash array for the latest written data
 * This should be initiated once on system startup
 */
uint32_t Flash_Software_Init(uint32_t *address, uint16_t *page, uint16_t *block )
{
	uint16_t i,j,k;
	uint8_t tmp;
	uint8_t currentblock;
	uint8_t timedisplay_data[3];
	uint16_t *hashpointer;
	uint16_t hashsum;
	uint32_t *stamppointer;
	uint32_t timestamp;
	timestamp=0;
	*page=0;
	*block=0;

	Flash_Set_256kb_Pages();
	/*
	 * Set LCD Font
	 */
	LCD_SetFont(&GL_Font8Bit43x50);

	/*
	 * Set initial values for page and block
	 * if no data was written previously the values will stay
	 * 0 and the flash will be written from beginning
	 */
	*page=0;
	*block=0;
	/*
	 * i counts up the 256 kbyte pages
	 * a 128 Mbit =16 mbyte device has 64 * 256 kbyte page organization
	 */
	for(i=0; i<64; i++)
	{
		currentblock=i;
		timedisplay_data[0]=currentblock%10;
		currentblock = currentblock /10;
		timedisplay_data[1]=currentblock%10;
		currentblock = currentblock/10;
		timedisplay_data[2]=currentblock%10;
		LCD_Display8BitChar(140 ,70,(32+timedisplay_data[2]));
		LCD_Display8BitChar(160 ,70,(32+timedisplay_data[1]));
		LCD_Display8BitChar(180, 70,(32+timedisplay_data[0]));


		for(j=0; j<1024; j++)
		{

			/*
			 * 0x40000 = 256 kbyte , 0x100 = 256 byte
			 */
			*address=(i*0x40000)+(j*0x100);
			Flash_4Read(*address, 256, (uint32_t) flash_data_buffer);

			/*
			 * k counts for each timestamp written within the 256 byte block
			 * a block has
			 *
			 */
			tmp=0;
			while(SPI1_BLOCK != SPI_BLOCK_FREE)
			{
				if(DMA_GetITStatus(DMA1_IT_TC2))
				{
					tmp=1;
				}
			}
			tmp=0xFF;
			if(flash_data_buffer[0]==0xFF)
			{
				/*
				 * The block is maybe free
				 * Continue to next block
				 */
				continue;
			}
			else
			{
				stamppointer=&flash_data_buffer[0];
				/*
				 * The last stored time should be in the past
				 *
				 */
				if(*stamppointer < rtc_time.unixtime)
				{
					/*
					 * Since this device was build after
					 * January 1970 the timestamp cannot be zero thus zero is excluded by the comparrisson
					 */
					if(*stamppointer > timestamp )
					{
						/*
						 * We get here if the actual block has newer data than the last block
						 */
						hashsum=Battery_Generate_Hash(flash_data_buffer);
						hashpointer=&flash_data_buffer[254];
						if(hashsum == *hashpointer)
						{
							/*
							 * We found valid flash data
							 * Store information of page, block and timestamp
							 */
							*page=i;
							*block=(j+1);
							if(*block == 1024)
							{
								*block=0;
								*page=i+1;
								if(*page == 64)
								{
									*page=0;
								}
							}
							timestamp=*stamppointer;
						}
						else
						{
							continue;
						}
					}
					else
					{
						continue;
					}
				}
			}
		}
	}
	/*
	 * if *block is 0 data will be written to a new page
	 * so check that the page is empty
	 */
	if(*block==0)
	{
		if(Check_Flash_Page_Free(*page))
		{
			Flash_4Page_Erase(*page*0x40000);
		}
	}
	*address=(*page*0x40000)+(*block*0x100);
	return 0;
}


/*
 * A quick and dirty hash function to generate a 16 bit checksum of each datablock
 * This is probably enough to check whether the stored data is valid
 */
uint16_t Battery_Generate_Hash(uint8_t *data)
{
	uint8_t i;
	uint16_t tmp;
	tmp=0;
	for(i=0; i<254;i++)
	{
		tmp=(tmp+((data[i]*data[i])<<8));
	}
	return tmp;
}

const uint8_t Batterie[] =
{
		/* Character Data - Index: 0 Batterie Header */
		0x00,0x00,0x00,0xF0,0xFF,0x3F,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0xF8,0xFF,0x7F,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0xFC,0xFF,0xFF,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0xFE,0xFF,0xFF,0x01,0x00,0x00,0x00,
		0x00,0x00,0x00,0xFF,0xFF,0xFF,0x03,0x00,0x00,0x00,
		0x00,0x00,0x80,0xFF,0xFF,0xFF,0x07,0x00,0x00,0x00,
		0x00,0x00,0x80,0x1F,0x00,0xC0,0x0F,0x00,0x00,0x00,
		0x00,0x00,0x80,0x1F,0x00,0xC0,0x0F,0x00,0x00,0x00,
		0x00,0x00,0x80,0x1F,0x00,0xC0,0x0F,0x00,0x00,0x00,
		0x00,0x00,0x80,0x1F,0x00,0xC0,0x0F,0x00,0x00,0x00,
		0x00,0x00,0x80,0x1F,0x00,0xC0,0x0F,0x00,0x00,0x00,
		0x00,0x00,0x80,0x1F,0x00,0xC0,0x0F,0x00,0x00,0x00,
		0x00,0x00,0x80,0x1F,0x00,0xC0,0x0F,0x00,0x00,0x00,
		0x00,0x00,0x80,0x1F,0x00,0xC0,0x0F,0x00,0x00,0x00,
		0x00,0x00,0x80,0x1F,0x00,0xC0,0x0F,0x00,0x00,0x00,
		0xF0,0xFF,0xFF,0x1F,0x00,0xC0,0xFF,0xFF,0x7F,0x00,
		0xF8,0xFF,0xFF,0x1F,0x00,0xC0,0xFF,0xFF,0xFF,0x00,
		0xFC,0xFF,0xFF,0x1F,0x00,0xC0,0xFF,0xFF,0xFF,0x01,
		0xFE,0xFF,0xFF,0x1F,0x00,0xC0,0xFF,0xFF,0xFF,0x03,
		0xFF,0xFF,0xFF,0x1F,0x00,0xC0,0xFF,0xFF,0xFF,0x07,
		0xFF,0xFF,0xFF,0x1F,0x00,0xC0,0xFF,0xFF,0xFF,0x07,
		0xFF,0xFF,0xFF,0x1F,0x00,0xC0,0xFF,0xFF,0xFF,0x07,
		0xFF,0xFF,0xFF,0x1F,0x00,0xC0,0xFF,0xFF,0xFF,0x07,
		0xFF,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x07,
		0xFF,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,0x07,
		0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF8,0x07,
		/* Character Data Index: 26 * 10 byte = 260 */
		/* Batterie sides */
		0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0x07,
		/* Character Data Index: 27 * 10 byte = 270 */
		/* Batterie Bottom 11 lines*/
		0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF8,0x07,
		0xFF,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,0x07,
		0xFF,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x07,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x07,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x07,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x07,
		0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x07,
		0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x03,
		0xFC,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,
		0xF8,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
		0xF0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x00
};

