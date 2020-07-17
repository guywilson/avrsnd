#include <stdint.h>

#ifndef _INCL_TWI_ATMEGA328P
#define _INCL_TWI_ATMEGA328P

#define I2C_RX_BUFFER_SIZE          8

#define I2C_BUS_ADDRESS             0xF8

#define I2C_RX_STATE_REGADDR        0x01
#define I2C_RX_STATE_REGVALUE       0x02

#define I2C_REG_COMMAND             0xF1
#define I2C_REG_DBA                 0xF2

void setupTWI();
void handleTWIByteRxTx();

#endif
