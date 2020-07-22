#ifndef BAUD
#define BAUD		57600
#endif

#include <stdint.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>

#include "serial_atmega328p.h"

#define enableTxInterrupt()			UCSR0B |= _BV(UDRIE0)
#define disableTxInterrupt()		UCSR0B &= ~_BV(UDRIE0)


uint8_t			txBuffer[80];
uint8_t			txLength = 0;

void setupSerial()
{
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

#if USE_2X
	UCSR0A |= _BV(U2X0);
#else
	UCSR0A &= ~(_BV(U2X0));
#endif

	/*
	** Set data byte size to 8-bit, 1 stop bit
	*/
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00) | _BV(USBS0);
	
	/*
	** Enable Tx
	** Enable Tx interrupts
	*/
	UCSR0B = _BV(TXEN0) | _BV(TXCIE0);
}

int getNextTxByte(uint8_t isInit)
{
	static uint8_t		i = 0;
	int					rtn;
	
	if (isInit) {
		i = 0;
	}

	if (i == txLength) {
		disableTxInterrupt();
		rtn = -1;
	}
	else {
		rtn = (int)txBuffer[i++];
	}

	return rtn;
}

void txstr(char * pszData, uint8_t dataLength)
{
	int			i;
	
	if (dataLength > sizeof(txBuffer)) {
		dataLength = sizeof(txBuffer);
	}
	
	for (i = 0;i < dataLength;i++) {
		txBuffer[i] = pszData[i];
	}
	
	txLength = dataLength;
	
	UDR0 = getNextTxByte(1);
	
	enableTxInterrupt();
}

void txmsg(uint8_t * pMsg, uint8_t dataLength)
{
	int			i;

	if (dataLength > sizeof(txBuffer)) {
		dataLength = sizeof(txBuffer);
	}

	for (i = 0;i < dataLength;i++) {
		txBuffer[i] = pMsg[i];
	}

	txLength = dataLength;

	UDR0 = getNextTxByte(1);

	enableTxInterrupt();
}

/*
** Tx Complete (Data Register Empty) Interrupt Handler
*/
void handleDRE()
{
	int b;

	b = getNextTxByte(0);

	if (b != -1) {
		UDR0 = (uint8_t)b;
	}
}
