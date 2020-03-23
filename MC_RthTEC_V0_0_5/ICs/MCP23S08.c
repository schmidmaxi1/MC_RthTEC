/*
 * MCP23S08.c
 *
 * Created: 30.04.2019 15:24:59
 *  Author: schmidm
 */ 

//*******************************************************************
//								Includes
//*******************************************************************

#include "MCP23S08.h"

#include "../usart_spi.h"

//*******************************************************************
//								Explanation
//*******************************************************************

/*	Explanation: Communication Protocol
 *	1. Byte: Address & Read/Write (Write: 0 | Read: 1)
 *	2. Byte: Register
 *	3. Byte: Value
 */

//*******************************************************************
//								Functions
//*******************************************************************

void IO_Expander_Init(){}
	
void IO_Expander_set_Register(uint8_t reg, uint8_t value, volatile uint8_t *port, uint8_t pin)
{
	//Warten bis fertig gesendet
	while(USART_SPI_TXWaiting());
	//CS ändern
	USART_SPI_SetCS(port, pin, 1, 3);
	//Nachricht senden
	USART_SPI_TxByte(IO_Expander_adresse);	//Read/Write bit -->0 (write)
	USART_SPI_TxByte(reg);
	USART_SPI_TxByte(value);
}

uint8_t IO_Expander_get_Register(uint8_t reg, volatile uint8_t *port, uint8_t pin)
{
	//Warten bis fertig gesendet
	while(USART_SPI_TXWaiting());
	//CS ändern
	USART_SPI_SetCS(port, pin, 1, 3);
	//Nachricht senden
	//USART_SPI_TxByte(IO_Expander_adresse | 0x01);	//Read/Write bit -->1 (read)
	//USART_SPI_TxByte(reg);
	
	char message[3] = {IO_Expander_adresse | 0x01, reg, 0x00};
	USART_SPI_RxBytes(message, 3);
	
	//Warten bis alle Daten da sind
	while(USART_SPI_RXWaiting() < 3){}

	//Wieviele Bytes müssen gelesen werden
	USART_SPI_ReceiveLen(3);
	
	//Auslesen (nur letztes Byte wichtig)
	return usart_spi_rx[2];

}

