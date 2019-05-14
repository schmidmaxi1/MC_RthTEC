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
//#include "../usart_spi.h"

/*
 ** Compiler-Konstanten
 */

#define IO_Expander_adresse	0b01000000	
//5bits definiert: 01000
//2bits �ber Pins: 00
//1bit f�r Read/Write

#define register_IODIR		0x00
#define register_IPOL		0x01
#define register_GPINTEN	0x02
#define register_DEFVAL		0x03
#define register_INTCON		0x04
#define register_IOCON		0x05
#define register_GPPU		0x06
#define register_INTF		0x07
#define register_INTCAP		0x08
#define register_GPIO		0x09
#define register_OLAT		0x0A


/*
 ** Funktionen
 */

void IO_Expander_Init();

void IO_Expander_set_Register(uint8_t reg, uint8_t value, volatile uint8_t *port, uint8_t pin);
uint8_t IO_Expander_get_Register(uint8_t reg, volatile uint8_t *port, uint8_t pin);

