/******************************************************************************
*
* The avr device acts as a worker on the I2C bus at address 0x18
*
* It implements the following registers:
*
* Name                  Address     Size    R/W Remarks
* ====================  =======     ======  === ===============================
* REG_RMS_WSIZE         0xF0        16-bit  R/W No. of samples to measure peak
* REG_LOUDNESS          0xF2        8-bit   R   The relative loudness 0 - 255   
* REG_RESET             0xF3        8-bit   W   Send 0xB6 to reset the device
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
#include <string.h>
#include <stdlib.h>
#include <util/twi.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <scheduler.h>

#include "adctask.h"
#include "wdttask.h"
#include "adc_atmega328p.h"
#include "twi_atmega328p.h"
#include "serial_atmega328p.h"

TWI_PARAMS params;

char buffer[32];

void setupTWI()
{
    TWAR = ((I2C_BUS_ADDRESS << 1) & 0xFE);
    TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWINT);
}

int getRegisterSize(uint8_t registerAddress)
{
    int         registerSize = 0;

    switch (registerAddress) {
        case REG_RMS_WSIZE:
            registerSize = 2;
            break;

        case REG_LOUDNESS:
            registerSize = 1;
            break;

        case REG_RESET:
            registerSize = 1;
            break;

        default:
            /*
            ** Error condition, unrecognised register address...
            */
            registerSize = -1;
            break;
    }

    return registerSize;
}

ISR(TWI_vect, ISR_BLOCK)
{
    static int  state = I2C_RX_STATE_REGADDR;
    static int  rxDataIt = 0;
    static int  txDataIt = 0;
    uint16_t    windowSize;
    uint8_t     i2cStatus;

    i2cStatus = (TWSR & 0xF8);

    switch (i2cStatus) {
        case TW_SR_SLA_ACK:
            state = I2C_RX_STATE_REGADDR;
            break;

        case TW_SR_DATA_ACK:            // Writing to a register...
            switch (state) {
                case I2C_RX_STATE_REGADDR:
                    params.regAddress = TWDR;
                    params.txrxDataLength = getRegisterSize(params.regAddress);
                    state = I2C_RX_STATE_REGVALUE;
                    break;

                case I2C_RX_STATE_REGVALUE:
                    params.rxData[rxDataIt++] = TWDR;

                    if (rxDataIt == params.txrxDataLength) {
                        state = I2C_RX_STATE_END;
                        rxDataIt = 0;
                    }

                    switch (params.regAddress) {
                        case REG_RMS_WSIZE:
                            memcpy(&windowSize, &params.txData, 2);
                            break;

                        case REG_RESET:
                            if (params.rxData[0] == DEVICE_RESET) {
                                disableWDTReset();
                            }
                            break;

                        default:
                            /*
                            ** Error condition, unrecognised register address...
                            */
                            break;
                    }
                    break;

                case I2C_RX_STATE_END:
                    state = I2C_RX_STATE_REGADDR;
                    break;
            }
            break;

        case TW_SR_STOP:
            state = I2C_RX_STATE_REGADDR;
            break;

        case TW_ST_SLA_ACK:                 // Reading from a register...
            switch (params.regAddress) {
                case REG_RMS_WSIZE:
                    windowSize = getWindowSize();
                    memcpy(&params.txData, &windowSize, 2);
                    break;

                case REG_LOUDNESS:
                    params.txData[0] = getLoudness();
                    strcpy(buffer, "TX loudness value: ");
                    itoa((int)params.txData[0], &buffer[19], 10);
                    buffer[strlen(buffer)] = '\n';
                    txstr(buffer, strlen(buffer));
                    break;

                default:
                    /*
                    ** Error condition, unrecognised register address...
                    */
                    break;
            }

            /*
            ** Send data back to the manager...
            */
            TWDR = params.txData[txDataIt++];

            if (txDataIt == params.txrxDataLength) {
                txDataIt = 0;
            }
            break;

        case TW_ST_DATA_ACK:
            state = I2C_RX_STATE_REGADDR;
            break;

        case TW_ST_DATA_NACK:    // 0xC0: data transmitted, NACK received
        case TW_ST_LAST_DATA:    // 0xC8: last data byte transmitted, ACK received
        case TW_BUS_ERROR:       // 0x00: illegal start or stop condition
        default:
            state = I2C_RX_STATE_REGADDR;
            break;
    }

    TWCR |= _BV(TWINT);
}
