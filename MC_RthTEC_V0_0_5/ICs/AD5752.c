/*
 * AD5752.c
 *
 * Created: 04.07.2019 15:49:13
 *  Author: schmidm
 */ 

#include "AD5752.h"
#include "../main.h"
//#include "../usart_spi.h"

/*	Explanation: Communication Protocol
 *	1. Byte: (Write: 0 | Read: 1)
 *		R/!W
 *		0
 *		Register (3Bit)
 *		Channel-ADR (3Bit)
 *	2. Byte: Value MSB
 *	3. Byte: Value LSB
 */

/*	Explanation: Registers

 *	1. DAC-Register:
 *		Value of Output (16bit)
 *
 *	2. Output Range select register
 *		Select the Range of the Output
 *
 *	3. Control Register
 *		Not used yet
 *
 *	4. Power Control Register
 *		Power up Channel A & B
 *		Temperature Warning	(not used)
 *		OverCurrent Warining (not used)
 */


// -------------------------------------------------------------
// DAC Initialisieren
// -------------------------------------------------------------


void DAC_AD5752_Range_and_PowerUp(uint8_t range, uint8_t powerUP, volatile uint8_t *port, uint8_t pin)
{
	//Warten bis fertig gesendet
	while(USART_SPI_TXWaiting());
	//CS ändern
	USART_SPI_SetCS(port, pin, 1, 3);
	
	//OUTPUT Range Select Register for both channels
	USART_SPI_TxByte(DAC_REG_RANGE | DAC_ADR_DAC_AB);	
	USART_SPI_TxByte(0);
	USART_SPI_TxByte(range);
	
	
	//Power Control Register  
	USART_SPI_TxByte(DAC_REG_POWER);	//Read/Write bit -->0 (write)
	USART_SPI_TxByte(0);
	USART_SPI_TxByte(powerUP);
}


// -------------------------------------------------------------
// Ausgang setzen
// -------------------------------------------------------------

void DAC_AD5752_Set(uint16_t val, volatile uint8_t *port, uint8_t pin, uint8_t ch)
{
	//Wait to finish previous communication
	while(USART_SPI_TXWaiting());
	//CS change
	USART_SPI_SetCS(port, pin, 1, 3);
	
	//Send message
	USART_SPI_TxByte(DAC_REG_DAC | ch);	
	USART_SPI_TxByte(val>>8);
	USART_SPI_TxByte(val);
}
