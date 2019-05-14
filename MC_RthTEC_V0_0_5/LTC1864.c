/*
 * LTC1864.c
 *
 * Created: 20.11.2018 16:02:30
 *  Author: schmidm
 */ 

/*
 ** Includes
 */

#include "main.h"
#include "LTC1864.h"

#include <util/delay.h>

/*
 ** Funktionen
 */

//Test

//////////////////////////////////////////////////////////////////////////
//					1. Spannungs-Messung für MOSFETs					//
//////////////////////////////////////////////////////////////////////////

uint32_t Measure_Voltage_MOSFET_with_DataAnalyse()
{
	//Gibt die Spannung in mV auf der MOSFET-Quelle zurück

	//Variablen für Rückgabe
	uint32_t returnValue = 0;
	
	//Chip select festlegen
	//USART_SPI_SetCS(volatile uint8_t *csr, uint8_t csp, uint8_t neg, uint8_t len)
	USART_SPI_SetCS(&IO_PORT5, 6, 1, 2);
		
	//LTC1864 braucht steigende Flanke um Messung zu starten
	clear_bit(IO_PORT5_3);
	_delay_us(5);
	set_bit(IO_PORT5_3);
			
	//Kurz bis Daten aufgenommen (Datasheet > 3.2µs)
	_delay_us(5);
	
	
	//Nachricht definieren 0x0000
	char message[2] = {0x00, 0x00};
	USART_SPI_RxBytes(message, 2);
	
	//Warten bis alle Daten da sind
	while(USART_SPI_RXWaiting() < 2){}

	//Wieviele Bytes müssen gelesen werden
	USART_SPI_ReceiveLen(2);
	
	//Auslesen
	returnValue = usart_spi_rx[0] << 8 | usart_spi_rx[1];

	//Umrechnen
	returnValue = returnValue * 2500 * 11 / 65536;

	//Rückgabe
	return returnValue;	
}

void Measure_Voltage_MOSFET_without_DataAnalyse()
{
	//Gibt den Auftrag zum Messen, ohne die Daten aus dem Ring_Buffer abzuholen
	//Wird später gemacht
	//Probleme mit while(alle Daten da) im Interrupt
	
	//Chip select festlegen
	//USART_SPI_SetCS(volatile uint8_t *csr, uint8_t csp, uint8_t neg, uint8_t len)
	USART_SPI_SetCS(&IO_PORT5, 6, 1, 2);
	
	//LTC1864 braucht steigende Flanke um Messung zu starten
	clear_bit(IO_PORT5_3);
	_delay_us(5);
	set_bit(IO_PORT5_3);
	
	//Kurz bis Daten aufgenommen (Datasheet > 3.2µs)
	_delay_us(5);
	
	
	//Nachricht definieren 0x0000
	char message[2] = {0x00, 0x00};
	USART_SPI_RxBytes(message, 2);	
	
	return;
}

uint32_t Measure_Voltage_MOSFET_get_Data()
{
	//Variablen für Rückgabe
	uint32_t returnValue = 0;
	
	//Wieviele Bytes müssen gelesen werden
	USART_SPI_ReceiveLen(2);
		
	//Auslesen
	returnValue = usart_spi_rx[0] << 8 | usart_spi_rx[1];

	//Umrechnen
	returnValue = returnValue * 2500 * 11 / 65536;

	//Rückgabe
	return returnValue;
}


//////////////////////////////////////////////////////////////////////////
//					1. Spannungs-Messung für MOSFETs					//
//////////////////////////////////////////////////////////////////////////

uint32_t Measure_Voltage_LED(){return 0x00;}