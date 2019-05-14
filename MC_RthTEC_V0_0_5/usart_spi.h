/* ----------------------------------------------------------
 *
 * USART SPI mode functions
 *
 * Author:  Johannes Knauss 2017, mail@iet-chiemsee.de
 *
 *
 * ----------------------------------------------------------
 */


#ifndef _USART_SPI_
#define _USART_SPI_



/*
**	Compiler-Konstanten
*/

#ifndef F_CPU
  #define F_CPU         16000000UL			    //System clock in Hz - Definition as unsigned long (!)
#endif

#ifndef USART_SPI_BAUD
  #define USART_SPI_BAUD    10000UL		        //USART Baudrate
#endif


//Register berechnen
#define USART_SPI_UBRR_VALUE        ((F_CPU+USART_SPI_BAUD*800)/(USART_SPI_BAUD*1600)-1)        //Runden
#define USART_SPI_BAUD_REAL         (F_CPU/(16*(USART_SPI_UBRR_VALUE+1)))               //Reale Baudrate
#define USART_SPI_BAUD_ERROR        ((USART_SPI_BAUD_REAL*10)/USART_SPI_BAUD) 		    //Fehler
 
#if ((USART_SPI_BAUD_ERROR<950) || (USART_SPI_BAUD_ERROR>1050))
	#error Fehler der Baudrate > 5%! 
#elif ((USART_SPI_BAUD_ERROR<960) || (USART_SPI_BAUD_ERROR>1040))
	#error Fehler der Baudrate > 4%! 
#elif ((USART_SPI_BAUD_ERROR<970) || (USART_SPI_BAUD_ERROR>1030))
	#warning Fehler der Baudrate > 3%! 
#endif

#define USART_SPI_RX_BUFFER_LENGHT      64	    //RX-Buffer
#define USART_SPI_TX_BUFFER_LENGHT      64		//TX-Buffer

#define USART_SPI_USE_RX_INTERRUPT      1  		//RXIE enable
#define USART_SPI_USE_TX_INTERRUPT      1  		//TXIE enable

#define USART_SPI_RX        PORTB,3				//MISO
#define USART_SPI_TX        PORTB,2				//MOSI
#define USART_SPI_XCK       PORTB,1				//SCK
#define USART_SPI_SS        PORTB,0				//SS



/*
**	GLOBALE Variablen
*/

char usart_spi_rx[USART_SPI_RX_BUFFER_LENGHT];



/*
**	GLOBALE Funktionen
*/

void	USART_SPI_Init();
void    USART_SPI_InitBaudrate(uint16_t baudrate);
void	USART_SPI_SetCS(volatile uint8_t *csr, uint8_t csp, uint8_t neg, uint8_t len);

void    USART_SPI_DoRxTx();
uint8_t	USART_SPI_ReceiveLine();
uint8_t USART_SPI_ReceiveBinaryCommand();
uint8_t	USART_SPI_ReceiveLen(uint8_t len);
uint8_t	USART_SPI_RXWaiting();
uint8_t USART_SPI_TXWaiting();
uint8_t	USART_SPI_ClearRx();

uint8_t	USART_SPI_TxByte(char byte);
void	USART_SPI_TxBytes(char *pointer, uint8_t len);
void    USART_SPI_ToggleCS(uint8_t set);

//Edit: Maxi
void USART_SPI_RxBytes(char *pointer, uint8_t lengeht);

#endif
