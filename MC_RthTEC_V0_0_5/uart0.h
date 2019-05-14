/* ----------------------------------------------------------
 *
 * USART0 Functions
 *
 * Written by Johannes Knauss 2016, mail@iet-chiemsee.de
 *
 *
 * ----------------------------------------------------------
 */


#ifndef _UART0_
#define _UART0_



/*
**	Compiler-Konstanten
*/

#ifndef F_CPU
  #define F_CPU         16000000UL						//Systemtakt in Hz - Definition als unsigned long (!)
#endif

#ifndef UART0_BAUD
  #define UART0_BAUD    576UL					        //UART Baudrate
#endif


//Register berechnen
#define UART0_UBRR_VALUE        ((F_CPU+UART0_BAUD*800)/(UART0_BAUD*1600)-1)        //Runden
#define UART0_BAUD_REAL         (F_CPU/(16*(UART0_UBRR_VALUE+1)))               //Reale Baudrate
#define UART0_BAUD_ERROR        ((UART0_BAUD_REAL*10)/UART0_BAUD) 		    //Fehler
 
#if ((UART0_BAUD_ERROR<950) || (UART0_BAUD_ERROR>1050))
	#error Fehler der Baudrate > 5%! 
#elif ((UART0_BAUD_ERROR<960) || (UART0_BAUD_ERROR>1040))
	#error Fehler der Baudrate > 4%! 
#elif ((UART0_BAUD_ERROR<970) || (UART0_BAUD_ERROR>1030))
	#warning Fehler der Baudrate > 3%! 
#endif

#define UART0_RX_BUFFER_LENGHT    128			//RX-Buffer
#define UART0_TX_BUFFER_LENGHT    128			//TX-Buffer

#define USE_RX0_INTERRUPT   1  					//RXIE enable
#define USE_TX0_INTERRUPT   1  					//TXIE enable



/*
**	Variablen
*/

char uart0_rx[UART0_RX_BUFFER_LENGHT +1];

uint8_t uart0_active_timer;



/*
**	Funktionen
*/

void	UART0Init();
void    UART0InitBaudrate(uint16_t baudrate);

void    UART0DoRxTx();
uint8_t	UART0ReceiveLine();
uint8_t	UART0ReceiveLen(uint8_t len);
uint8_t	UART0CharsWaiting();
uint8_t UART0SendWaiting();
uint8_t	UART0ClearRx();

uint8_t	UART0TxByte(char byte);
void 	UART0TransmitString(char *string);
void 	UART0TransmitStringLn(char *string);
void	UART0TransmitBytes(char *pointer, uint8_t len);



#endif
