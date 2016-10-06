#ifndef I2C_SWITCH_H
#define I2C_SWITCH_H

#define PCA9534_2_READ_ADDRESS  0x49 // 01001001
#define PCA9534_2_WRITE_ADDRESS 0x48 //01001000
void write_switch(uint8_t, uint8_t, uint8_t);
void initialize_i2c_switch(uint8_t);

#endif // I2C_SWITCH_H
