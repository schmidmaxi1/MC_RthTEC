/* ----------------------------------------------------------
 *
 * USART SPI mode functions
 *
 * Author:  Johannes Knauss 2017, mail@iet-chiemsee.de
 *
 *
 * ----------------------------------------------------------
 */

/* ----------------------------------------------------------
 *						 ERKLÄRUNG:
 * -----------------------------------------------------------------------------------
 *											 Maxi; 21.11.2018
 *	
 * Für SPI-Kommunikation besitz der MEGA 2560 nur ein Register 
 *		--> SPDR: SPI Data Register
 * Die zu sendenden Daten werden hier abgelegt & und die zu lesenden Daten werden hier 
 * abgeholt und aus RingBuffern 
 * gehohlt bzw. in ihm abgelegt
 * -Funktionen: _USART_SPI_RxBuffer() & _USART_SPI_TxBuffer()
 * -RingBuffer usart_spi_rx_buffer[] & usart_spi_tx_buffer[]
 *
 * RingBuffer-Counter:
 * char_write: Bestimmt an welcher Stelle in den Buffer abgelegt wird (Zwischspeicher)
 * char_read: Bestimmt an welcher Stelle auf dem Buffer verwendet wird (senden / lesen)
 *
 * USART_SPI_SetCS(volatile uint8_t *csr, uint8_t csp, uint8_t neg, uint8_t len) 
 * Port(csr:CSRegister) und Pin (CSP: CS Pin) des ChipSelects werden lokal 
 * eingestellt und gespeichert.
 * neg: ??????????????????????????????
 * len: (length) Anzahl Bytes die gesendet werden sollen
 * 
 * USART_SPI_ToggleCS(uint8_t set):
 * Chip select wird auf 0 bzw. 1 gesetzt
 *
 * USART_SPI_ReceiveLen(uint8_t len):
 * Löst eine Anzahl len an Bytes aus dem Buffer heraus und schreibt sie in das 
 * globale Auswerte-Feld usart_spi_rx[] hier können die Daten abgeholt werden
 *
 * USART_SPI_TXWaiting() & USART_SPI_RXWaiting():
 * Geben die Anzahl an Bytes zurück die noch im Buffer liegen, aber nicht versendet
 * bzw. Ausgelesen wurden
 *
 * USART_SPI_ClearRx():
 * Setzt die Counter für den RX-RingBuffer auf 0 (Daten werden nicht gelöscht, sind 
 * aber unauffindbar)
 *
 * USART_SPI_TxByte(char byte) & USART_SPI_TxBytes(char *pointer, uint8_t len)
 * Input ist 1 Byte bzw. mehrere Bytes
 * Für mehrere Bytes werden die Bytes aufgeteilt und einzeln ausgeführt
 * Für das este Byte wird das senden in der Funktion ausgelöst
 * Alle weiteren werden über den Interrupt "senden fertig" versendet
 *
 * ------------------------------------------------------------------------------------
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "usart_spi.h"



/*
**	LOKALE Variablen
*/

volatile char usart_spi_rx_buffer[USART_SPI_RX_BUFFER_LENGHT];
volatile char usart_spi_tx_buffer[USART_SPI_TX_BUFFER_LENGHT];

volatile uint8_t usart_spi_rx_char_read = 0;
volatile uint8_t usart_spi_rx_char_write = 0;
volatile uint8_t usart_spi_rx_buffer_overrun = 0;
volatile uint8_t usart_spi_rx_lenght = 0;
//Edit: Maxi
volatile uint8_t usart_spi_rx_flag = 0;

volatile uint8_t usart_spi_tx_char_read = 0;
volatile uint8_t usart_spi_tx_char_write = 0;
volatile uint8_t usart_spi_tx_buffer_empty = 1;

volatile uint8_t *usart_spi_cs_port = &PORTB;
uint8_t usart_spi_cs_pin = 0;
uint8_t usart_spi_cs_neg = 0;
uint8_t usart_spi_cs_len = 1;
uint8_t usart_spi_byte_count = 0;



/*
**	LOKALE Funktionen 
*/

inline void _USART_SPI_RxBuffer();
inline void _USART_SPI_TxBuffer();


/*
**	UART RX-Interupt
**	
*/

//Wird nie ausgelöst
ISR(USART2_RX_vect)
{
	_USART_SPI_RxBuffer();
	TransmitStringLn("ja");
}

/*
**	UART TX-Interupt
**	
*/

//Wird ausgelöst wenn Byte gesendet wurde
ISR(SPI_STC_vect)
{
	if(usart_spi_rx_flag){_USART_SPI_RxBuffer();}	//edit Maxi	
	_USART_SPI_TxBuffer();	
}

/*
**	Funktionen
*/

// -------------------------------------------------------------
// USART Schnittstelle initialisieren					 (Check)
// -------------------------------------------------------------

void USART_SPI_Init()
{
    USART_SPI_InitBaudrate(USART_SPI_BAUD);
}

void USART_SPI_InitBaudrate(uint16_t baudrate)
{
	UBRR2 = 0;
	
	set_bit(USART_SPI_SS);	    			    //RX as input							//??????????????????

	set_in(USART_SPI_RX);	    			    //RX as input
	set_out(USART_SPI_TX);	    			    //TX as output
	set_out(USART_SPI_XCK);	    			    //XCK as output
	//XCK2_DDR |= _BV(XCK2);

	//SPI Control Register
	SPCR = _BV(SPE) | _BV(MSTR) | _BV(CPHA);
	//SPE:SPI Enable		[on]
	//MSTR: Master Mode		[on]
	//CPOL: Clock polarity	[rising]
	//CPHA: Clock phase		[setup]	--> MODE 1

	#if (USART_SPI_USE_RX_INTERRUPT)
	SPCR |= _BV(SPIE);		                //USART RX ISR aktivieren
	#endif

	#if (USART_SPI_USE_TX_INTERRUPT)
	SPCR |= _BV(SPIE);		                //USART TX ISR aktivieren
	#endif
	//SPIE: SPI Interrupt Enable [on]


	// uint16_t ubrr_value = ((F_CPU+(uint32_t)baudrate*800)/((uint32_t)baudrate*1600)-1);

	// UBRR2H = ubrr_value >> 8;
	// UBRR2L = ubrr_value & 0xFF;
	
	// Clock einstellen
	SPCR |= _BV(SPR1) | _BV(SPR0);		                //USART TX ISR aktivieren
}

// -------------------------------------------------------------
// USART SPI CS setzen & umschalten						 (Check)
// -------------------------------------------------------------

void USART_SPI_SetCS(volatile uint8_t *csr, uint8_t csp, uint8_t neg, uint8_t len)
{
    usart_spi_cs_port = csr;
    usart_spi_cs_pin = csp;
    usart_spi_cs_neg = neg;
    usart_spi_cs_len = len;
}

void USART_SPI_ToggleCS(uint8_t set)
{
	if ((usart_spi_cs_neg && set) || (!usart_spi_cs_neg && !set))
	{
		//Clear
		*usart_spi_cs_port &= ~_BV(usart_spi_cs_pin);
	}
	else
	{
		//Set
		*usart_spi_cs_port |= _BV(usart_spi_cs_pin);
	}
}

// -------------------------------------------------------------
// Anzahl Zeichen von UART lesen						 (Check)
// -------------------------------------------------------------

uint8_t USART_SPI_ReceiveLen(uint8_t len)
{
		uint8_t usart_spi_rx_lenght = 0;

		//Maxi: Verhindern das nicht mehr Daten abgeholt werden als Buffer lang ist
		if (len > USART_SPI_RX_BUFFER_LENGHT) len = USART_SPI_RX_BUFFER_LENGHT;

		while((usart_spi_rx_char_read != usart_spi_rx_char_write || usart_spi_rx_buffer_overrun) && usart_spi_rx_lenght < len)
		{
			//Zeichen in RX Array übertragen
			usart_spi_rx[usart_spi_rx_lenght] = usart_spi_rx_buffer[usart_spi_rx_char_read];
			//Nächste stelle
			usart_spi_rx_lenght++;

			#if (USART_SPI_RX_INTERRUPT)
			SPCR &= ~_BV(SPIE);						//RX Interrupt aus
			#endif

			usart_spi_rx_char_read++;

			//Ring-Buffer am Ende, reset
			if(usart_spi_rx_char_read == USART_SPI_RX_BUFFER_LENGHT)
			{
				usart_spi_rx_char_read = 0;
			}

			usart_spi_rx_buffer_overrun = 0;

			#if (USART_SPI_RX_INTERRUPT)
			SPCR |= _BV(SPIE);		                //USART RX ISR aktivieren
			#endif

		}

		return usart_spi_rx_lenght;
}


// -------------------------------------------------------------
// Anzahl Zeichen im RX bzw. TX Buffer					 (Check)
// -------------------------------------------------------------

uint8_t USART_SPI_RXWaiting()
{
	if(usart_spi_rx_buffer_overrun) return USART_SPI_RX_BUFFER_LENGHT;
	else return (usart_spi_rx_char_write - usart_spi_rx_char_read);
}

uint8_t USART_SPI_TXWaiting()
{
	return usart_spi_tx_char_write - usart_spi_tx_char_read;
}

// -------------------------------------------------------------
// RX Buffer leeren										 (Check)
// -------------------------------------------------------------

uint8_t USART_SPI_ClearRx()
{
	uint8_t dummy;

#if (USART_SPI_USE_RX_INTERRUPT)
	SPCR &= ~_BV(SPIE);         //RX Interrupt aus
#endif

	dummy = SPDR;
	usart_spi_rx_char_read = 0;
	usart_spi_rx_char_write = 0;
    
	dummy += SPDR;
	usart_spi_rx_buffer_overrun = 0;

#if (USART_SPI_USE_RX_INTERRUPT)
	SPCR |= _BV(SPIE);          //RX ISR aktivieren
#endif

    return dummy;
}

// -------------------------------------------------------------
// Byte an TX Buffer anhängen							 (Check)
// -------------------------------------------------------------

#include <util/delay.h>

uint8_t USART_SPI_TxByte(char byte)
{	
	//Abbruchbedingung für schreiben in tx_Buffer
	//Nur wenn Ring_Buffer vollgeschrieben ist (senden der Daten zu langsam)
	//Nur der fall, falls write und read counter gleich, aber schon Daten im Buffer
    if(usart_spi_tx_char_write == usart_spi_tx_char_read && !usart_spi_tx_buffer_empty) return 0;

	//Byte in Buffer legen
	usart_spi_tx_buffer[usart_spi_tx_char_write] = byte;

#if (USART_SPI_USE_TX_INTERRUPT)
	SPCR &= ~_BV(SPIE);						//TX Interrupt aus
#endif

	//Buffer-Counter hochzählen
	usart_spi_tx_char_write++;
	
    //Ring-Buffer überlauf prüfen
    if(usart_spi_tx_char_write == USART_SPI_TX_BUFFER_LENGHT)
	{
		usart_spi_tx_char_write = 0;
	}

	
	//Neue Übertragung starten (würd nur für erste Byte ausgelöst, alle andern über Interrupt)
    if(usart_spi_tx_buffer_empty)
	{
		//CS low setzen
        USART_SPI_ToggleCS(1);

		//Senden
		SPDR = usart_spi_tx_buffer[usart_spi_tx_char_read];		//sende erstes Zeichen

        usart_spi_byte_count++;
		
		//Flag das aktuell gesendet wird
		usart_spi_tx_buffer_empty = 0;
	}
	
#if (USART_SPI_USE_TX_INTERRUPT)
	SPCR |= _BV(SPIE);		//TX ISR aktivieren
#endif

	return 1;
}

void USART_SPI_TxBytes(char *pointer, uint8_t len)
{
	while(len)
	{
		//Warten falls Buffer schon Vollgeschreiben ist
		while(!USART_SPI_TxByte(*pointer)){}
		pointer++;
		len--;
	}
}

// -------------------------------------------------------------
// Byte in Tx schreiben und aus Rx Buffer lesen			 (Check)
// -------------------------------------------------------------

//Edit: Maxi 21.11.2018
void USART_SPI_RxBytes(char *pointer, uint8_t length)
{

	//Flag setzen, das auch gelesen werden soll
	usart_spi_rx_flag = 1;
	
	//Alle Bytes in Buffer legen, gesendet wird durch Interrupt
	USART_SPI_TxBytes(pointer, length);
	
}

// -------------------------------------------------------------
// Eingangs/Ausgangsbuffer verarbeiten					(unused)		
// -------------------------------------------------------------

inline void USART_SPI_DoRxTx()
{
	//Zeichen im Eingangs-Buffer
    if(UCSR2A & _BV(RXC2))
	{
        _USART_SPI_RxBuffer();
	}

	//Ausgangs-Buffer leer
    if(UCSR2A & _BV(TXC2))
	{
        UCSR2A |= _BV(TXC2);

        _USART_SPI_TxBuffer();
	}
}


// -------------------------------------------------------------
// Zeichen vom Eingangsregister lesen					 (Check)
// -------------------------------------------------------------

inline void _USART_SPI_RxBuffer()
{
	//Ring-Buffer nicht voll
	if(!usart_spi_rx_buffer_overrun)
	{
		//Maxi: Daten wurden in SPDR (SPI Data Register) abgelegt
		//Maxi: Daten werdein in eingenen Buffer übernommen an aktuelle Stelle geschrieben
		usart_spi_rx_buffer[usart_spi_rx_char_write] = SPDR;

		//Maxi: aktuelle Stelle hochzählen
		usart_spi_rx_char_write++;

		//Ring-Buffer am Ende, reset
		//Maxi: Wenn an letzer stelle angekommen, dann auf 0 setzen
		if(usart_spi_rx_char_write == USART_SPI_RX_BUFFER_LENGHT)
		{
			usart_spi_rx_char_write = 0;
		}

		//Ring-Buffer voll (zu wenig Daten abgeholt)
		if(usart_spi_rx_char_write == usart_spi_rx_char_read)
		{
			usart_spi_rx_buffer_overrun = 1;
		}
		//Wenn alle Bits gelesen, Flag zurücksetzen
		if (usart_spi_byte_count == usart_spi_cs_len)
		{
			usart_spi_rx_flag = 0;
		}
	}
	else
	{
		USART_SPI_ClearRx();
	}
}


// -------------------------------------------------------------
// Zeichen ins Ausgangsregister schreiben			     (Check)
// -------------------------------------------------------------

inline void _USART_SPI_TxBuffer()
{
	//Wenn alle Bits gesendet wurden --> CS zurück + byte_counter zuückseten
    if (usart_spi_byte_count == usart_spi_cs_len)
    {
        USART_SPI_ToggleCS(0);
        usart_spi_byte_count = 0;
    }

    //Noch zeichen zum senden vorhanden
	if(!usart_spi_tx_buffer_empty)
	{
		//Zeichen im Hardware-Buffer
        //while ( !( SPSR & _BV(UDRE2) ) );

        usart_spi_tx_char_read++;

		//Ring-Buffer am Ende, reset
        if(usart_spi_tx_char_read == USART_SPI_TX_BUFFER_LENGHT)
		{
			usart_spi_tx_char_read = 0;
		}

		//Alle Zeichen im Buffer gesendet?
		// --> Flag setzen
		if(usart_spi_tx_char_write == usart_spi_tx_char_read)
		{
			usart_spi_tx_buffer_empty = 1;
		}
        // --> nächstes zeichen senden
		else
		{
            USART_SPI_ToggleCS(1);
			SPDR = usart_spi_tx_buffer[usart_spi_tx_char_read];
            usart_spi_byte_count++;
		}
	}
}
