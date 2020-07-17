#include <stdint.h>

#ifndef _INCL_TWI_ATMEGA328P
#define _INCL_TWI_ATMEGA328P

#define I2C_BUS_ADDRESS             0xF8

#define I2C_RX_STATE_REGADDR        0x01

void setupTWI();
void handleTWIByteRxTx();

#endif
