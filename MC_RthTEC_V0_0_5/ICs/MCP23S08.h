/*
 * MCP23S08.h
 *
 * Created: 30.04.2019 15:25:23
 *  Author: schmidm
 */ 


/*
 ** Includes
 */

//#include "../usart_spi.h"
#include <stdint.h>
#include "../usart_spi.h"

/*
 ** Compiler-Konstanten
 */

#define IO_Expander_adresse	0b01000000	
//5bits definiert: 01000
//2bits über Pins: 00
//1bit für Read/Write

#define register_IODIR		0x00		//IO- Direction
#define register_IPOL		0x01		//Input Polarity
#define register_GPINTEN	0x02		//Interupt on Change Control
#define register_DEFVAL		0x03		//Default on Change (Interupt)
#define register_INTCON		0x04		//Interupt Controll
#define register_IOCON		0x05		//Configuration Register
#define register_GPPU		0x06		//Pull Up Resistior Config
#define register_INTF		0x07		//Interrupt Flag
#define register_INTCAP		0x08		//Interrupt Capture
#define register_GPIO		0x09		//Value
#define register_OLAT		0x0A		//Output Value


/*
 ** Funktionen
 */

void IO_Expander_Init();

void IO_Expander_set_Register(uint8_t reg, uint8_t value, volatile uint8_t *port, uint8_t pin);
uint8_t IO_Expander_get_Register(uint8_t reg, volatile uint8_t *port, uint8_t pin);

