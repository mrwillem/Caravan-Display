#include "hw_config.h"
#include "core_cm3.h"
#include "i2c.h"
#include "batterie.h"
#include "bq34z100g1.h"
#include "temperatur.h"

/*
 * Define global variables
 */

uint8_t bq34_usb_mode;


/*
 * a function to read the voltage
 */
uint16_t read_voltage(void)
{
	if(bq34_usb_mode == 0)
	{
		voltage=0;
		/* 0x08 / 0x09 */
		while(GL_I2C2_busy);
		GL_I2C2_busy=1;
		Buffer_Tx2[0]=0x08;
		I2C_Master_BufferWrite(I2C2, Buffer_Tx2, 1,  BQ34Z100G1_WRITE_ADDRESS);
		I2C_Master_BufferRead(I2C2, Buffer_Rx2, 2, BQ34Z100G1_READ_ADDRESS);
		voltage=Buffer_Rx2[0] | (Buffer_Rx2[1]<<8);
		Buffer_Tx2[0]=0x10;
		I2C_Master_BufferWrite(I2C2, Buffer_Tx2, 1,  BQ34Z100G1_WRITE_ADDRESS);
		I2C_Master_BufferRead(I2C2, Buffer_Rx2, 2, BQ34Z100G1_READ_ADDRESS);
		current=Buffer_Rx2[0] | (Buffer_Rx2[1]<<8);
		GL_I2C2_busy=0;
		return voltage;
	}
	else
	{
		voltage=10;
		temperatur=10;
	}
}

/*
 * Initialize global variables
 */
void bq34z100g1_init(void)
{
	bq34_usb_mode=0;
}
