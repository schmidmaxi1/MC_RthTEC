/*
 * LTC1864.c
 *
 * Created: 20.08.2019 18:21:50
 *  Author: schmidm
 */ 

//*******************************************************************
//								Includes
//*******************************************************************

//Check Order!!!

#include "LTC1864.h"

#include "../helper.h"
#include "../usart_spi.h"

#include <util/delay.h>
#include <avr/io.h>	//Is neccessary because IOs are switch directly

//*******************************************************************
//								Explanation
//*******************************************************************

/*	Explanation: Communication Protocol

There is no real Communication Protocol
Only a operation sequence:

-With the rising Edge of /CS (data sheet: CONV) the LTC1864 samples the voltage
-After a conversation time (3.3µs) the data is available on SPI register
-While /CS is low the 16bit a shifted out with the SPI-CLK (Input-Message not relevant)

 */

/*	Explanation: Registers

No Registers

 */

//////////////////////////////////////////////////////////////////////////
//								Functions								//
//////////////////////////////////////////////////////////////////////////

uint16_t LTC1864_getBIT_OneShot(volatile uint8_t *port, uint8_t pin)
{
	//Complete Measurement-Sequence with return value
	//CS to LOW --> Wait --> CS to HIGH (Sample) --> Wait converstion time --> CS to LOW --> Reseive data --> CS Low
	
	//Wait to finish previous communication
	while(USART_SPI_TXWaiting());
	//CS change
	USART_SPI_SetCS(port, pin, 1, 2);
	
	//CS to LOW
	_clear_bit(*port, pin);
	
	//Wait short time
	_delay_us(2);
	
	//CS to HIGH (Sample)
	_set_bit(*port, pin);
		
	//Wait conversion time (3.3µs)
	_delay_us(5);
	
	//Send 16bit message and receive bit value
	char message[2] = {0x00, 0x00};
	USART_SPI_RxBytes(message, 2);
	
	//Wait until all bytes are available
	while(USART_SPI_RXWaiting() < 2){}

	//Read two bytes
	USART_SPI_ReceiveLen(2);
		
	//Return
	return usart_spi_rx[0] << 8 | usart_spi_rx[1];
}

uint16_t LTC1864_getBIT_LastSample_and_NewShot(volatile uint8_t *port, uint8_t pin)
{
	//Receive the data form the last measurement and execute new one
		
	//Wait to finish previous communication
	while(USART_SPI_TXWaiting());
	//CS change
	USART_SPI_SetCS(port, pin, 1, 2);
	
	//Send 16bit message and receive bit value
	char message[2] = {0x00, 0x00};
	USART_SPI_RxBytes(message, 2);
		
	//Wait until all bytes are available
	while(USART_SPI_RXWaiting() < 2){}

	//Read two bytes
	USART_SPI_ReceiveLen(2);
		
	//Return
	return usart_spi_rx[0] << 8 | usart_spi_rx[1];
}