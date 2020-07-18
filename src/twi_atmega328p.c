#include <stdint.h>
#include <stddef.h>
#include <util/twi.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <scheduler.h>

#include "twi_atmega328p.h"

static uint8_t rxData[I2C_RX_BUFFER_SIZE];

void setupTWI()
{
    TWAR = ((I2C_BUS_ADDRESS << 1) & 0xFE);
    TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWINT);
}

void I2CReceiveHandler(uint8_t rxByte)
{
    static int      state = I2C_RX_STATE_REGADDR;
    static int      i = 0;
    static int      dataLength;
    static uint8_t  regAddress;

    switch (state) {
        case I2C_RX_STATE_REGADDR:
            regAddress = rxByte;

            switch (regAddress) {
                case I2C_REG_DBA:
                    dataLength = 2;
                    break;
            }

            state = I2C_RX_STATE_REGVALUE;
            break;

        case I2C_RX_STATE_REGVALUE:
            rxData[i++] = rxByte;
            break;
    }
}

void I2CTransmitHandler()
{

}

void handleTWI()
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
