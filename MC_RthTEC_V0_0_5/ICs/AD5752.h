/*
 * AD5752.h
 *
 * Created: 04.07.2019 15:49:30
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

//Read /Write
#define DAC_R           0b10000000

//Register
#define DAC_REG_DAC     0b00000000
#define DAC_REG_RANGE   0b00001000
#define DAC_REG_POWER   0b00010000
#define DAC_REG_CONTROL 0b00011000

//Channel ADR
#define DAC_ADR_DAC_A   0b00000000
#define DAC_ADR_DAC_B   0b00000010
#define DAC_ADR_DAC_AB  0b00000100


//Output Range:
#define Range_p5V		0b00000000
#define Range_p10V		0b00000001
#define Range_p10V8		0b00000010
#define Range_pm5V		0b00000011
#define Range_pm10V		0b00000100
#define Range_pm10V8	0b00000101

//Power Control
#define PowerUp_A		0b00000001
#define PowerUp_B		0b00000100
#define PowerUp_AB		0b00000101
#define PowerUp_NON		0b00000000


/*
 ** Functions
 */

void DAC_AD5752_Range_and_PowerUp(uint8_t range, uint8_t powerUP, volatile uint8_t *port, uint8_t pin);
void DAC_AD5752_Set(uint16_t val, volatile uint8_t *port, uint8_t pin, uint8_t ch);