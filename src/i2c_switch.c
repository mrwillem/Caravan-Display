#include "hw_config.h"
#include "core_cm3.h"
#include "i2c.h"
#include "i2c_switch.h"


/*
 * A function to set the initial register (input or output) of the switch
 *
 */
void initialize_i2c_switch(uint8_t switchnum)
{
	write_i2c_switch(switchnum, 0x03, 0b00001000);
}
/*
 * Simple function to write to the remote I2C port expander
 */
void write_i2c_switch(uint8_t switchnum, uint8_t reg, uint8_t data)
{
	while(I2C2_BUSY);
	I2C2_BUSY=1;
	Buffer_Tx2[0]=reg;
	Buffer_Tx2[1]=data;
	I2C_Master_BufferWrite(I2C2, Buffer_Tx2, 2,  PCA9534_2_WRITE_ADDRESS);
	I2C2_BUSY=0;
}
