
#ifndef USB_ENDP_H
#define USB_ENDP_H

/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_mem.h"
#include "hw_config.h"
#include "usb_istr.h"
#include "usb_pwr.h"

extern uint8_t buffer_out[VIRTUAL_COM_PORT_DATA_SIZE];
extern __IO uint32_t count_out;
extern uint32_t count_in;

#endif //USB_ENP_H
