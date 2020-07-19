/******************************************************************************
*
* The avr device acts as a worker on the I2C bus at address 0x18
*
* It implements the following registers:
*
* Name                  Address     Size    R/W
* ====================  =======     ======  ===
* REG_RMS_WSIZE         0xF0        16-bit  R/W
* REG_DB                0xF2        16-bit  R
* REG_RESET             0xF4        8-bit   W
*
* I2C Write
* ---------------------
* In order to write to a register, first the register address must be sent
* in write mode, then send pairs of register address and register data.
* 
*  --- ------------------- --- --- --------------- --- --------------- --- 
* | S | Worker address    | W | A | Register addr | A | Register data | A |
*  --- ------------------- --- --- --------------- --- --------------- --- 
* 
*                                   --------------- --- --------------- --- ----
*                                  | Register addr | A | Register data | A | ST |
*                                   --------------- --- --------------- --- ----
*
* I2C Read
* ---------------------
* In order to read from a register, first the register address must be sent
* in write mode.
*
*  --- ------------------- --- --- --------------- --- 
* | S | Worker address    | W | A | Register addr | A |
*  --- ------------------- --- --- --------------- --- 
* 
*  --- ------------------- --- --- --------------- --- --------------- --- ----
* | S | Worker address    | R | A | Register data | A | Register data | A | ST |
*  --- ------------------- --- --- --------------- --- --------------- --- ----
*
******************************************************************************/

#include <stdint.h>
#include <stddef.h>
#include <util/twi.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <scheduler.h>

#include "twi_atmega328p.h"

TWI_PARAMS params;

void setupTWI()
{
    TWAR = ((I2C_BUS_ADDRESS << 1) & 0xFE);
    TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWINT);
}

void I2CReceiveHandler(uint8_t rxByte)
{
    static int      state = I2C_RX_STATE_REGADDR;
    static int      i = 0;

    switch (state) {
        case I2C_RX_STATE_REGADDR:
            params.txRegAddress = rxByte;

            switch (params.txRegAddress) {
                case REG_DB:
                    params.txDataLength = 2;
                    break;
            }

            state = I2C_RX_STATE_REGVALUE;
            break;

        case I2C_RX_STATE_REGVALUE:
            params.rxData[i++] = rxByte;
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
