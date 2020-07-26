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
#include <util/twi.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <scheduler.h>

#include "adctask.h"
#include "wdttask.h"
#include "adc_atmega328p.h"
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
    uint16_t        windowSize;

    switch (state) {
        case I2C_RX_STATE_REGADDR:
            params.regAddress = rxByte;

            switch (params.regAddress) {
                case REG_RMS_WSIZE:
                    params.txrxDataLength = 2;
                    windowSize = getWindowSize();
                    memcpy(&params.txData, &windowSize, 2);
                    break;

                case REG_LOUDNESS:
                    params.txrxDataLength = 1;
                    params.txData[0] = getLoudness();
                    break;

                case REG_RESET:
                    params.txrxDataLength = 1;
                    break;

                default:
                    /*
                    ** Error condition, unrecognised register address...
                    */
                    break;
            }

            state = I2C_RX_STATE_REGVALUE;
            break;

        case I2C_RX_STATE_REGVALUE:
            params.rxData[i++] = rxByte;

            if (i == params.txrxDataLength) {
                state = I2C_RX_STATE_REGADDR;
                i = 0;

                switch (params.regAddress) {
                    case REG_RMS_WSIZE:
                        setWindowSize((uint16_t)params.rxData[0]);
                        break;

                    case REG_RESET:
                        if (rxByte == DEVICE_RESET) {
                            disableWDTReset();
                        }
                        break;

                    default:
                        /*
                        ** Error condition, unrecognised register address...
                        */
                        break;
                }
            }
            break;
    }
}

void I2CTransmitHandler()
{
    static int      i = 0;

    if (i < params.txrxDataLength) {
        TWDR = params.txData[i++];
    }
}

ISR(TWI_vect, ISR_BLOCK)
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
