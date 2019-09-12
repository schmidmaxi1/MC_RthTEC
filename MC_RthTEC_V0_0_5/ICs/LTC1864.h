/*
 * LTC1864.h
 *
 * Created: 20.08.2019 18:22:10
 *  Author: schmidm
 */ 

#include <stdint.h>

/*
 ** Compiler-Constants
 */

//Non necessary

/*
 ** Functions
 */

uint16_t LTC1864_getBIT_OneShot(volatile uint8_t *port, uint8_t pin);
uint16_t LTC1864_getBIT_LastSample_and_NewShot(volatile uint8_t *port, uint8_t pin);