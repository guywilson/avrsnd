#include <stdint.h>
#include <stddef.h>
#include <util/twi.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <scheduler.h>

#include "twi_atmega328p.h"

void setupTWI()
{
    TWAR = ((I2C_BUS_ADDRESS << 1) & 0xFE);
    TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWINT);
}

void I2CReceiveHandler(uint8_t rxCommand)
{
    static int      state = I2C_RX_STATE_REGADDR;

    switch (state) {
        case I2C_RX_STATE_REGADDR:
            break;
    }
}

void I2CTransmitHandler()
{

}

void handleTWIByteRxTx()
{
    uint8_t     status;

    status = (TWSR & 0xF8);

    switch (status) {
        case TW_SR_DATA_ACK:
            I2CReceiveHandler(TWDR); 
            break;

        case TW_ST_SLA_ACK:
            I2CTransmitHandler();
            break;

        case TW_ST_DATA_ACK:
            I2CTransmitHandler();
            break;

        case TW_BUS_ERROR:
            TWCR = 0;
            break;
    }

    TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWINT);
}
