#include "main.h"
#include "settings.h"
#include "usb_cdc.h"
#include "usb_endp.h"
#include "usb_conf.h"
#include "usb_desc.h"
#include "usb_prop.h"
#include "usb_pwr.h"
#include "usb_lib.h"
#include "batterie.h"
#include "spi.h"
#include "i2c.h"
#include "bq34z100g1.h"
#include "flash.h"
#include "rtc.h"


volatile uint8_t bufferPos;
uint8_t buffer[128];
uint8_t SendBuffer[128];

/*
 * Just a short function to initialize the global variables 
 * shared with the usb ISRs 
 */
void usb_cdc_init(void)
{
	bufferPos=0;
	count_in=0;
}

/*
 * This routine gets called by the USB RX interrupt handler
 */
void update_rx_buffer(void)
{
	uint8_t i;
	if (( bufferPos < 129 ) && (count_out != 0) && (bDeviceState == CONFIGURED))
	{
		for(i=0; i < count_out; i++)
		{
			buffer[bufferPos] = buffer_out[i];
			bufferPos++;
		}
		count_out=0;
	}
}

/*
 * A function that handles all usb serial communication with the device
 */
void decode_usb_message(void)
{

	uint8_t j, k;
	/* write_bytes is the byte counter for the serial answer */
	uint8_t write_bytes;
	/*
	 * bq34cmdtype either write or compare (1 or 2) byteno number of bytes to write to bq34 device in flash mode
	 * chksum checksum byte for flash data received from host
	 */
	uint8_t bq34cmdtype, byteno, chksum;
	/*
	 * a 16 bit variable to calculate the checksum
	 */
	uint16_t tmpchcksum;
	/*
	 * a pointer to handle the events where a 16 bit value is in two 8 bit registers
	 * !!!!! This is not portable and the endianess must be respected
	 */
	uint16_t *pointer;
	/* Variable for a i2c slave address mainly used in flash program mode */
	uint8_t i2c_slave_address;
	/*
	 * Check for CR LF combination from terminal
	 * 0x0D = CR 0x0A = LF
	 */
	if(bufferPos > 2)
	{
		if(((buffer[bufferPos-1]==0x0a) && (buffer[bufferPos-2]==0x0d)) || (bufferPos > 127))
		{
			/* timezone is a global variable defined in rtc.h */
			if(strncmp(buffer, "timezone", 8) == 0)
			{
				j=buffer[9]-0x30;
				j=j*10;
				j+=(buffer[10]-0x30);
				timezone=j;
				strncpy(SendBuffer, "Timezone Set", 12);
				write_bytes=12;
			}
			/* timezone is a global variable defined in rtc.h */
			if(strncmp(buffer, "settime", 7) == 0)
			{
				while(I2C2_BUSY);
				I2C2_BUSY=1;
				j=buffer[9]-0x30;
				j=j*10;
				j+=(buffer[10]-0x30);
				Buffer_Tx2[3]=DecToBcd(j);
				j=buffer[12]-0x30;
				j=j*10;
				j+=(buffer[13]-0x30);
				Buffer_Tx2[2]=DecToBcd(j);
				j=buffer[14]-0x30;
				j+=(buffer[15]-0x30);
				Buffer_Tx2[1]=DecToBcd(1);
				// address
				Buffer_Tx2[0]=0x00;
				I2C_Master_BufferWrite(I2C2, Buffer_Tx2, 8, I2C_RTC_ADDRESS_WRITE);
				I2C2_BUSY=0;
				rtc_time.unixtime=synchronize_unixtime();
				strncpy(SendBuffer, "Time Set", 8);
				write_bytes=8;
			}
			/* The readbattery command returns the content of the flash that stores the battery data */
			else if(strncmp(buffer, "readbattery", 10) == 0)
			{
				usb_cdc_send_battery_data();
				write_bytes=0;
			}
			/* i2c_send sends the hex bytes defined to the BQ34Z100G1 device....
			 * Needs some modification to address other I2C devices as well 
			 */
			else if(strncmp(buffer, "i2c_send", 8) == 0)
			{
				k=0;
				for(j=8; j < bufferPos-2; j++)
				{
					if(atoh(j, k))
					{
						j+=2;
						k++;
					}
				}
				if(k != 0)
				{
					I2C_Master_BufferWrite(I2C2, Buffer_Tx2, k,  BQ34Z100G1_WRITE_ADDRESS);
					strncpy(SendBuffer, "Data Written", 12);
					write_bytes=12;
				}
				else
				{
					strncpy(SendBuffer, "Invalid", 7);
					write_bytes=7;
				}

			}
			/* this sets bq34_usb_mode variable to 1. The variable is defined in bq34z100-g1.h. 
			 * when set to 1 no other communication with the device will take place by the rest of the software
			 */
			else if(strncmp(buffer, "i2c_lock", 8) == 0)
			{
				bq34_usb_mode=1;
				strncpy(SendBuffer, "SetMode", 7);
				write_bytes=7;
			}
			else if(strncmp(buffer, "i2c_free", 8) == 0)
			{
				bq34_usb_mode=0;
				strncpy(SendBuffer, "SetMode", 7);
				write_bytes=7;
			}
			/* The i2c_read command reads the device from the bq34z100g1 
			 * after writing to the specified registers.
			 */
			else if(strncmp(buffer, "i2c_read", 8) == 0)
			{
				k=(buffer[12]-0x30)*10+(buffer[13]-0x30);
				if(atoh(9, 0))
				{
					if(k > 0 && k < 33)
					{
						I2C_Master_BufferWrite(I2C2, Buffer_Tx2, 1,  BQ34Z100G1_WRITE_ADDRESS);
						I2C_Master_BufferRead(I2C2, Buffer_Rx2, k, BQ34Z100G1_READ_ADDRESS);
						for(j=0; j<k; j++)
						{
							/*
							 * This trick lets us store 16 bit in the 8 bit array
							 * The htoa function returns a 16 bit unsingned int which contains 
							 * the ascii code of both nibbles of the byte provided 
							 * !!!!! This is designed for little endian plattform
							 */
							pointer=&SendBuffer[j*3];
							*pointer=htoa(Buffer_Rx2[j]);
							SendBuffer[(((j+1)*3)-1)]=0x20;
						}
						write_bytes=k*3;
						if ( k>20)
						{
							count_in=64;
							SendBuffer[write_bytes]=0x0d;
							SendBuffer[write_bytes+1]=0x0a;
							USB_SIL_Write(EP1_IN, SendBuffer, 64);
							SetEPTxValid(ENDP1);
							while(count_in);
							count_in= (write_bytes+2-64);
							USB_SIL_Write(EP1_IN, &SendBuffer[64], write_bytes-64+2);
							SetEPTxValid(ENDP1);
							while(count_in);
							write_bytes=0;
						}
					}
				}
			}
			/* i2c_getblock sends the content of a whole flashblock of the bq34 device to the terminal
			 */
			else if(strncmp(buffer, "bqx_subcmd", 10) == 0)
			{
				k=(buffer[20]-0x30)*10+(buffer[21]-0x30);
				if((atoh(11, 0)) && (atoh(14, 1)) && (atoh(17, 2)))
				{
					if(k > 0 && k < 33)
					{
						I2C_Master_BufferWrite(I2C2, Buffer_Tx2, 1,  BQ34Z100G1_WRITE_ADDRESS);
						I2C_Master_BufferRead(I2C2, Buffer_Rx2, k, BQ34Z100G1_READ_ADDRESS);
						for(j=0; j<k; j++)
						{
							/*
							 * This trick lets us store 16 bit in the 8 bit array
							 * The htoa function returns a 16 bit unsingned int which contains
							 * the ascii code of both nibbles of the byte provided
							 * !!!!! This is designed for little endian plattform
							 */
							pointer=&SendBuffer[j*3];
							*pointer=htoa(Buffer_Rx2[j]);
							SendBuffer[(((j+1)*3)-1)]=0x20;
						}
						write_bytes=k*3;
						if ( k>20)
						{
							count_in=64;
							SendBuffer[write_bytes]=0x0d;
							SendBuffer[write_bytes+1]=0x0a;
							USB_SIL_Write(EP1_IN, SendBuffer, 64);
							SetEPTxValid(ENDP1);
							while(count_in);
							count_in= (write_bytes+2-64);
							USB_SIL_Write(EP1_IN, &SendBuffer[64], write_bytes-64+2);
							SetEPTxValid(ENDP1);
							while(count_in);
							write_bytes=0;
						}
					}
				}
			}
			else if(strncmp(buffer,"set_bq34_rommode", 16) == 0)
			{
				/*
				 * We will misuse write bytes here as flash mode indicator to save memory
				 * as long as write_bytes stays at 127 only the following while loop is executed
				 */
				write_bytes=127;
				strncpy(SendBuffer, "bq34_rommode_set", 16);
				SendBuffer[16]=0x0d;
				SendBuffer[17]=0x0a;
				USB_SIL_Write(EP1_IN, SendBuffer, 17);
				SetEPTxValid(ENDP1);
				bufferPos=0;
				/*
				 * k will be neede in the followin while loop to determine whether the pagage consists of
				 * more than 51 command bytes
				 */
				k=0;
				/*
				 * go into routine which disables rest of system software
				 */
				while(write_bytes==127)
				{
					/*
					 * Check cr lf ending package
					 */
					if(bufferPos == 64)
					{
						if((buffer[0]==0x01) && (buffer[1]==0x5D) && (buffer[2]==0x7D) && (buffer[62]==0x0a) && (buffer[63]==0x0d) )
						{
							/*
							 * Check if everything is messed up
							 */
							if(k>64)
							{
								k=0;
								strncpy(SendBuffer, "reset_bqtx", 8);
								SendBuffer[8]=0x0d;
								SendBuffer[9]=0x0a;
								USB_SIL_Write(EP1_IN, SendBuffer, 10);
								SetEPTxValid(ENDP1);
								bufferPos=0;
								continue;
							}
							/*
							 * Check if we were asked to leave programming mode
							 * here write_bytes is set to a lower value than 127 so the for loop is exited
							 */
							if(strncmp(&buffer[6], "exit_programm_mode", 18) == 0)
							{
								strncpy(SendBuffer, "acknol", 6);
								write_bytes=6;
								break;
							}
							else
							{
								chksum=0xFF;
								chksum-=buffer[0];
								chksum-=buffer[1];
								chksum-=buffer[2];
								bq34cmdtype=buffer[3];
								byteno=buffer[4];
								chksum-=buffer[3];
								chksum-=buffer[4];
								chksum-=buffer[5];
								i2c_slave_address=buffer[5];
								for(j=6; j<61; j++)
								{
									chksum=chksum-buffer[j];
									Buffer_Tx2[(j+k-6)]=buffer[j];
								}
								/* Check for checksum mismatch */
								if(chksum == buffer[61])
								{
									/* if k is 51 then the buffer should already be filled with data from two packets
									 * this is the case if the command has more than 51 bytes. Up to now I never saw a command 
								  	 * with more than 101 byte
									 * 101 byte fit in two usb transfers of maximal 64 bytes */
									if((byteno < 52) || (k==51))
									{
										/*
										 * cmdtype 1 is a write command
										 */
										if(bq34cmdtype==1)
										{
											/*
											 * The data will be found in Buffer_Tx2[1] and so on
											 * Buffer_Tx2[0] is I2C address
											 */
											I2C_Master_BufferWrite(I2C2, Buffer_Tx2, byteno, i2c_slave_address);
											strncpy(SendBuffer, "acknol", 6);
											SendBuffer[5]=0x0d;
											SendBuffer[6]=0x0a;
											USB_SIL_Write(EP1_IN, SendBuffer, 7);
											bufferPos=0;
											SetEPTxValid(ENDP1);
										}
										/*
										 * cmdtype 2 is a compare command
										 */
										else if(bq34cmdtype==2)
										{
											/*
											 * Shuffle rx buffer
											 * Byteno reflects the whole bytes transferred.
											 * The first byte is the address, in case of a read the second byte is the register
											 * Thus Buffer_Tx2[2] contains the first byte to be validated
											 */
											for(j=0;j<byteno-1; j++)
											{
												Buffer_Rx2[j]=0xFF-Buffer_Tx2[j+2];
											}
											I2C_Master_BufferWrite(I2C2, Buffer_Tx2, 1, i2c_slave_address);
											I2C_Master_BufferRead(I2C2, Buffer_Rx2, byteno-1, i2c_slave_address);
											for(j=0; j<byteno-1; j++)
											{

												/*
												 * if the comparisson gets wrong then the command failed
												 */
												if(Buffer_Tx2[j+1] != Buffer_Rx2[j])
												{
													/*
													 * Major fault, host software has to decide whats to to
													 */
													strncpy(SendBuffer, "cmpfail", 7);
													SendBuffer[7]=0x0d;
													SendBuffer[8]=0x0a;
													USB_SIL_Write(EP1_IN, SendBuffer, 9);
													bufferPos=0;
													SetEPTxValid(ENDP1);
													j=byteno;
													break;
												}

											}
											if(j == (byteno-1))
											{
												strncpy(SendBuffer, "acknol", 6);
												SendBuffer[5]=0x0d;
												SendBuffer[6]=0x0a;
												USB_SIL_Write(EP1_IN, SendBuffer, 7);
												bufferPos=0;
												SetEPTxValid(ENDP1);
											}
											else
											/*
											 * should never happen
											 */
											{
												/*
												 * Major fault, host software has to decide whats to to
												 */
												strncpy(SendBuffer, "cmpfail", 7);
												SendBuffer[7]=0x0d;
												SendBuffer[8]=0x0a;
												USB_SIL_Write(EP1_IN, SendBuffer, 9);
												bufferPos=0;
												SetEPTxValid(ENDP1);
												j=byteno;
											}

										}
										k=0;
									}
									else
									{
										/*
										 * If the command has more than 51 bytes a second usb transfer is needed
										 * The next variable stored will be in Buffer_Tx2+k
										 */
										k=k+51;
										strncpy(SendBuffer, "next_pack", 9);
										SendBuffer[9]=0x0d;
										SendBuffer[10]=0x0a;
										USB_SIL_Write(EP1_IN, SendBuffer, 11);
										bufferPos=0;
										SetEPTxValid(ENDP1);

									}
								}
								/*
								 * Else there is a checksum mismatch, please resend
								 */
								else
								{
									strncpy(SendBuffer, "resend", 6);
									SendBuffer[6]=0x0d;
									SendBuffer[7]=0x0a;
									USB_SIL_Write(EP1_IN, SendBuffer, 8);
									bufferPos=0;
									SetEPTxValid(ENDP1);
								}
							}
						}
						else
						{
							strncpy(SendBuffer, "resend", 10);
							SendBuffer[10]=0x0d;
							SendBuffer[11]=0x0a;
							USB_SIL_Write(EP1_IN, SendBuffer, 12);
							bufferPos=0;
							SetEPTxValid(ENDP1);
						}
					}
					/*
					 * Check whether host asks if we are already in flash programm mode
					 */
					if(strncmp(buffer,"set_bq34_rommode", 16) == 0)
					{
						strncpy(SendBuffer, "bq34_rommode_set", 16);
						SendBuffer[16]=0x0d;
						SendBuffer[17]=0x0a;
						USB_SIL_Write(EP1_IN, SendBuffer, 18);
						bufferPos=0;
						buffer[0]=0x01;
						SetEPTxValid(ENDP1);
					}

					/*
					 * Something went really wrong with last package
					 * Do a full resend meaning retransmit the whole last package
					 */
					if(bufferPos > 64)
					{
						strncpy(SendBuffer, "reset_bqtx", 10);
						SendBuffer[10]=0x0d;
						SendBuffer[11]=0x0a;
						USB_SIL_Write(EP1_IN, SendBuffer, 12);
						bufferPos=0;
						SetEPTxValid(ENDP1);
					}
				}
			}
			else
			{
				strncpy(SendBuffer, "Invalid", 7);
				write_bytes=7;
			}
			bufferPos =0;
			if( write_bytes > 0)
			{
				SendBuffer[write_bytes]=0x0d;
				SendBuffer[write_bytes+1]=0x0a;
				USB_SIL_Write(EP1_IN, SendBuffer, write_bytes+2);
				SetEPTxValid(ENDP1);
			}
		}
		else if((buffer[0]=='A') && (buffer[1]=='T') && (buffer[2]=='\r'))
		{
			bufferPos =0;
			SendBuffer[0]='O'; SendBuffer[1]='K'; SendBuffer[2]='\r';
			USB_SIL_Write(EP1_IN, SendBuffer, 3);
			SetEPTxValid(ENDP1);
		}
	}
}


void usb_cdc_send_battery_data(void)
{
	uint16_t  j;
	uint8_t i,tmp;
	uint32_t address;
	for(i=0; i<64; i++)
	{
		for(j=0; j<1023; j++)
		{
			/*
			 * 0x40000 = 256 kbyte , 0x100 = 256 byte
			 */
			address=(i*0x40000)+(j*0x100);
			Flash_4Read(address, 256, (uint32_t) flash_data_buffer);
			/*
			 * k counts for each timestamp written within the 256 byte block
			 * a block has
			 */
			tmp=0;
			while(SPI1_BLOCK != SPI_BLOCK_FREE)
			{
				if(DMA_GetITStatus(DMA1_IT_TC2))
				{
					tmp=1;
				}
			}
			while(count_in);
			count_in=64;
			USB_SIL_Write(EP1_IN, &flash_data_buffer[0], 64);
			SetEPTxValid(ENDP1);
			while(count_in);
			count_in=64;
			USB_SIL_Write(EP1_IN, &flash_data_buffer[64], 64);
			SetEPTxValid(ENDP1);
			while(count_in);
			count_in=64;
			USB_SIL_Write(EP1_IN, &flash_data_buffer[128], 64);
			SetEPTxValid(ENDP1);
			while(count_in);
			count_in=64;
			USB_SIL_Write(EP1_IN, &flash_data_buffer[172], 64);
			SetEPTxValid(ENDP1);

		}
	}
}

/* the function returns the ascii codes for both nibbles of the value provide
 * in one 16 bit integer
 */
uint16_t htoa(uint8_t hex)
{
	uint8_t tmp;
	uint16_t out;
	tmp=(hex & 0b00001111);
	if(tmp > 9)
	{
		out=((tmp+0x37) << 8);
	}
	else
	{
		out=((tmp+0x30) << 8);
	}
	tmp=((hex >> 4) & 0b00001111);
	if(tmp > 9)
	{
		out+=(tmp+0x37);
	}
	else
	{
		out+=(tmp+0x30);
	}
	return out;

}

/*
 * The function decodes the value represented by two HEX nibbles in the input buffer 
 * string buffer at position bpos in the and stores it in the I2C send buffer at position k
 * it returns 0 on failure and 1 on success
 */
uint8_t atoh(uint8_t bpos, uint8_t k)
{
	/*
	 * If the ascii value is between 0x30 and 0x39 it is a digit
	 * the first nibble is shifted by four to the left as it represents
         * the four MSBs of the value
	 */
	if(buffer[bpos]> 0x29 && buffer[bpos] < 0x3A)
	{
		Buffer_Tx2[k]=((buffer[bpos]-0x30)<<4);
	}
	/*
	 * if the ascii value is between 0x41 and 0x46 it is an hex character
	 */
	else if(buffer[bpos]>0x40 && buffer[bpos]< 0x47)
	{
		Buffer_Tx2[k]=((buffer[bpos]-0x37)<<4);
	}
	else
	/*
	 *  Data not valid
	 */
	{
		return 0;
	}
	/*
	 * decode the lower nibble same as above
	 */
	if(buffer[bpos+1]> 0x29 && buffer[bpos+1] < 0x3A)
	{
		Buffer_Tx2[k]+=(buffer[bpos+1]-0x30);
	}
	else if(buffer[bpos+1]>0x40 && buffer[bpos+1]< 0x47)
	{
		Buffer_Tx2[k]+=(buffer[bpos+1]-0x37);
	}
	else
	{
		Buffer_Tx2[k]=0;
		return 0;
	}
	return 1;
}
