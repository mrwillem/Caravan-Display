
#ifndef USB_CDC_H
#define USB_CDC_H

extern volatile uint8_t bufferPos;
extern uint8_t buffer[128];
extern uint8_t SendBuffer[128];

void usb_cdc_init(void);
void update_rx_buffer(void);
uint16_t htoa(uint8_t);
uint8_t atoh(uint8_t, uint8_t );

void decode_usb_message(void);

#endif // USB_CDC_H
