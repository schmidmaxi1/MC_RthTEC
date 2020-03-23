/* ----------------------------------------------------------
 *
 * USART0 Functions
 *
 * Written by Johannes Knauss 2016, mail@iet-chiemsee.de
 *
 *
 * ----------------------------------------------------------
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>

//#include "main.h"
#include "uart0.h"



/*
**	Variablen
*/

volatile char uart0_rx_buffer[UART0_RX_BUFFER_LENGHT];
volatile char uart0_tx_buffer[UART0_TX_BUFFER_LENGHT];

volatile uint8_t uart0_rx_char_read = 0;
volatile uint8_t uart0_rx_char_write = 0;
volatile uint8_t uart0_rx_overrun = 0;
volatile uint8_t uart0_rx_lenght = 0;
volatile uint8_t uart0_rx_lines = 0;
volatile uint8_t uart0_rx_frames = 0;
volatile uint8_t uart0_rx_blocked = 0;

volatile uint8_t uart0_tx_char_read = 0;
volatile uint8_t uart0_tx_char_write = 0;
volatile uint8_t uart0_tx_empty = 1;



/*
**	Funktionen
*/

inline void _UART0RxBuffer();
inline void _UART0TxBuffer();



/*
**	UART RX-Interupt
**	
*/

ISR(USART0_RX_vect)
{
	_UART0RxBuffer();
}



/*
**	UART TX-Interupt
**	
*/

ISR(USART0_TX_vect)
{
	_UART0TxBuffer();
}



/*
**	Funktionen
*/

// -------------------------------------------------------------
// UART Schnittstelle initialisieren
// -------------------------------------------------------------

void UART0Init()
{
	UART0InitBaudrate(UART0_BAUD);
}



// -------------------------------------------------------------
// UART Schnittstelle initialisieren, Baudrate
// -------------------------------------------------------------

void UART0InitBaudrate(uint16_t baudrate)
{
	DDRE |= _BV(1);	    						// PE1 (TX1) as output

	UCSR0B |= _BV(TXEN0) | _BV(RXEN0);			// USART0 RX/TX aktivieren

#if (USE_RX0_INTERRUPT)
	UCSR0B |= _BV(RXCIE0);		// USART0 RX ISR aktivieren
#endif

#if (USE_TX0_INTERRUPT)
	UCSR0B |= _BV(TXCIE0);		// USART0 TX ISR aktivieren
#endif

    UCSR0C |= _BV(UCSZ01) | _BV(UCSZ00);		// Asynchron 8N1
 
    uint16_t ubrr_value = ((F_CPU+(uint32_t)baudrate*800)/((uint32_t)baudrate*1600)-1);

    UBRR0H = ubrr_value >> 8;
    UBRR0L = ubrr_value & 0xFF;
}



// -------------------------------------------------------------
// Zeile von UART lesen
// -------------------------------------------------------------

uint8_t UART0ReceiveLine()
{
	uint8_t uart0_rx_lenght = 0;

    if(uart0_rx_lines)
    {
    	while(uart0_rx_lenght < UART0_RX_BUFFER_LENGHT)
    	{
    		uart0_rx[uart0_rx_lenght] = uart0_rx_buffer[uart0_rx_char_read];
		
#if (USE_RX0_INTERRUPT)
    		UCSR0B &= ~_BV(RXCIE0);                         //RX Interrupt aus
#endif

    		uart0_rx_char_read++;
    		if(uart0_rx_char_read == UART0_RX_BUFFER_LENGHT)
    		{
    			uart0_rx_char_read = 0;
    		}

    		uart0_rx_overrun = 0;

#if (USE_RX0_INTERRUPT)
	        UCSR0B |= _BV(RXCIE0);                          //RX ISR aktivieren
#endif

            if(uart0_rx[uart0_rx_lenght] == '\n')
            {

#if (USE_RX0_INTERRUPT)
    		    UCSR0B &= ~_BV(RXCIE0);						//RX Interrupt aus
#endif

                uart0_rx_lines--;

#if (USE_RX0_INTERRUPT)
                UCSR0B |= _BV(RXCIE0);                      //RX ISR aktivieren
#endif

                uart0_rx[uart0_rx_lenght + 1] = 0x00;       //'\0' anhängen (String-Ende)

                return uart0_rx_lenght;
            }

            //'\0' nicht übernehmen (Übertragungsfehler)
    		if(uart0_rx[uart0_rx_lenght])
            {
                uart0_rx_lenght++;
            }
    	}
    }
    else if(uart0_rx_overrun)
    {
        UART0ClearRx();
    }
    
    return 0;
}



// -------------------------------------------------------------
// Anzahl Zeichen von UART lesen
// -------------------------------------------------------------

uint8_t UART0ReceiveLen(uint8_t len)
{
	uint8_t uart0_rx_lenght = 0;

    if (len > UART0_RX_BUFFER_LENGHT) len = UART0_RX_BUFFER_LENGHT;

	while((uart0_rx_char_read != uart0_rx_char_write || uart0_rx_overrun) && uart0_rx_lenght < len)
	{
		uart0_rx[uart0_rx_lenght] = uart0_rx_buffer[uart0_rx_char_read];

		uart0_rx_lenght++;

#if (USE_RX0_INTERRUPT)
		UCSR0B &= ~_BV(RXCIE0);						//RX Interrupt aus
#endif

		uart0_rx_char_read++;
		if(uart0_rx_char_read == UART0_RX_BUFFER_LENGHT)
		{
			uart0_rx_char_read = 0;
		}

		uart0_rx_overrun = 0;

#if (USE_RX0_INTERRUPT)
	    UCSR0B |= _BV(RXCIE0);		                //RX ISR aktivieren
#endif

	}

	uart0_rx[uart0_rx_lenght] = 0x00;				//'\0' anhängen (String-Ende)

	return uart0_rx_lenght;
}



// -------------------------------------------------------------
// Anzahl Zeichen im RX Buffer
// -------------------------------------------------------------

uint8_t UART0CharsWaiting()
{
	if(uart0_rx_overrun) return UART0_RX_BUFFER_LENGHT;
	else return uart0_rx_char_write - uart0_rx_char_read;
}



// -------------------------------------------------------------
// Anzahl Zeichen im TX Buffer
// -------------------------------------------------------------

uint8_t UART0SendWaiting()
{
	return uart0_tx_char_write - uart0_tx_char_read;
}



// -------------------------------------------------------------
// RX Buffer leeren
// -------------------------------------------------------------

uint8_t UART0ClearRx()
{
	uint8_t dummy;

#if (USE_RX0_INTERRUPT)
	UCSR0B &= ~_BV(RXCIE0);         //RX Interrupt aus
#endif

	dummy = UDR0;
	uart0_rx_char_read = 0;
	uart0_rx_char_write = 0;

	dummy += UDR0;
	uart0_rx_overrun = 0;
	uart0_rx_lines = 0;

#if (USE_RX0_INTERRUPT)
	UCSR0B |= _BV(RXCIE0);          //RX ISR aktivieren
#endif

    return dummy;
}



// -------------------------------------------------------------
// Byte an TX Buffer anhängen
// -------------------------------------------------------------

uint8_t UART0TxByte(char byte)
{
	if(uart0_tx_char_write == uart0_tx_char_read && !uart0_tx_empty) return 0;

	uart0_tx_buffer[uart0_tx_char_write] = byte;

#if (USE_TX0_INTERRUPT)
	UCSR0B &= ~_BV(TXCIE0);						//TX Interrupt aus
#endif

	uart0_tx_char_write++;
	if(uart0_tx_char_write == UART0_TX_BUFFER_LENGHT)
	{
		uart0_tx_char_write = 0;
	}
	
	if(uart0_tx_empty)
	{
#if (UART0_USE_RS485)
#if (USE_RX0_INTERRUPT && UART0_USE_RS485_2_WIRE)
        uart0_rx_blocked = 1;
	    UCSR0B &= ~_BV(RXCIE0);         //RX ISR deaktivieren
#endif

#if (UART0_USE_RS485_DE)
        //enable TX_EN
        UART0_RS485_DE_PORT |= _BV(UART0_RS485_DE_PIN);
#endif
#endif

		UDR0 = uart0_tx_buffer[uart0_tx_char_read];		//sende erstes Zeichen

		uart0_tx_empty = 0;
	}
	
#if (USE_TX0_INTERRUPT)
	UCSR0B |= _BV(TXCIE0);		//TX ISR aktivieren
#endif

	return 1;
}



// -------------------------------------------------------------
// String auf UART ausgeben
// -------------------------------------------------------------

void UART0TransmitString(char *string)
{
	while(*string)
	{
		while(!UART0TxByte(*string));

	    string++;
	}
}



// -------------------------------------------------------------
// String & LF auf UART ausgeben
// -------------------------------------------------------------

void UART0TransmitStringLn(char *string)
{
	while(*string)
	{
		while(!UART0TxByte(*string));

	    string++;
	}
	
	while(!UART0TxByte('\n'));			// <LF> anhängen
}



// -------------------------------------------------------------
// Bytes auf UART ausgeben
// -------------------------------------------------------------

void UART0TransmitBytes(char *pointer, uint8_t len)
{
	while(len)
	{
		while(!UART0TxByte(*pointer));

	    pointer++;
		len--;
	}
}



// -------------------------------------------------------------
// Eingangs/Ausgangsbuffer verarbeiten
// -------------------------------------------------------------

inline void UART0DoRxTx()
{
	if(UCSR0A & _BV(RXC0))
	{
        _UART0RxBuffer();
	}

	if(UCSR0A & _BV(TXC0))
	{
        UCSR0A |= _BV(TXC0);

        _UART0TxBuffer();
	}
}



// -------------------------------------------------------------
// Zeichen vom Eingangsregister lesen
// -------------------------------------------------------------

inline void _UART0RxBuffer()
{
	if(uart0_rx_blocked)
    {
        //Clear UDR0
        uart0_active_timer = UDR0;
    }
    else if(!uart0_rx_overrun)
	{
		uart0_rx_buffer[uart0_rx_char_write] = UDR0;

        if(uart0_rx_buffer[uart0_rx_char_write] == '\n')
        {
            uart0_rx_lines++;
        }

		uart0_rx_char_write++;
		if(uart0_rx_char_write == UART0_RX_BUFFER_LENGHT)
		{
			uart0_rx_char_write = 0;
		}

		if(uart0_rx_char_write == uart0_rx_char_read)
		{
			uart0_rx_overrun = 1;
		}

		//Com active led
        uart0_active_timer = 5;
	}
    else
    {
        //Clear UDR0
        uart0_active_timer = UDR0;
    }
}



// -------------------------------------------------------------
// Zeichen ins Ausgangsregister schreiben
// -------------------------------------------------------------

inline void _UART0TxBuffer()
{
	if(!uart0_tx_empty)
	{
		uart0_tx_char_read++;

		if(uart0_tx_char_read == UART0_TX_BUFFER_LENGHT)
		{
			uart0_tx_char_read = 0;
		}

		if(uart0_tx_char_read == uart0_tx_char_write)
		{
			uart0_tx_empty = 1;
		}
		else
		{
			UDR0 = uart0_tx_buffer[uart0_tx_char_read];
		}
	}

#if (UART0_USE_RS485)
    if (uart0_tx_empty)
    {
#if (UART0_USE_RS485_DE)
        //disable TX_EN
        UART0_RS485_DE_PORT &= ~_BV(UART0_RS485_DE_PIN);
#endif

#if (USE_RX0_INTERRUPT && UART0_USE_RS485_2_WIRE)
        uart0_rx_blocked = 0;
	    UART0ClearRx();          //RX buffer leeren, ISR aktivieren
#endif
    }
#endif

    //Com active led
    uart0_active_timer = 5;
}
