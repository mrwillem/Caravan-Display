#ifndef BQ34Z100G1_H
#define BQ34Z100G1_H


#define BQ34Z100G1_WRITE_ADDRESS 0xAA
#define BQ34Z100G1_READ_ADDRESS 0xAB
uint16_t read_voltage(void);
void bq34z100g1_init(void);
extern uint8_t bq34_usb_mode;


#endif /* BQ34Z100G1_H */
