#include <stdint.h>

#ifndef _INCL_TWI_ATMEGA328P
#define _INCL_TWI_ATMEGA328P

#define I2C_RXTX_BUFFER_SIZE          8

#define I2C_BUS_ADDRESS             0x18

#define I2C_RX_STATE_REGADDR        0x01
#define I2C_RX_STATE_REGVALUE       0x02

#define DEVICE_RESET                0xB6

#define REG_RMS_WSIZE               0xF0
#define REG_DB                      0xF2
#define REG_RESET                   0xF4

typedef struct {
    uint8_t     rxData[I2C_RXTX_BUFFER_SIZE];
    uint8_t     txData[I2C_RXTX_BUFFER_SIZE];

    int         txrxDataLength;
    int         regAddress;
}
TWI_PARAMS;

void setupTWI();
void handleTWI();

#endif
