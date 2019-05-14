/* ----------------------------------------------------------
 *
 * Project:  LED Test system
 * Module:   DAC, AD5752R
 *
 * Author:   Johannes Knauss 2017, mail@iet-chiemsee.de
 *
 *
 * ----------------------------------------------------------
 */



/*
 ** Includes
 */

#include "usart_spi.h"


/*
 ** Compiler-Konstanten
 */

/*
#define DAC_1       1   //DAC I
#define DAC_2       2   //DAC Window
*/

#define DAC_SDO     PORTB,3
#define DAC_SDI     PORTB,2
#define DAC_SCK     PORTB,1

#define DAC_CLR		PORTE,4
#define DAC_LDAC	PORTE,1

/*
//DAC auf Stromquelle
#define DAC_1_SYNC      PORTA,3
#define DAC_1_SYNC_PORT PORTA
#define DAC_1_SYNC_PIN  3
#define DAC_1_CLR       PORTA,4
#define DAC_1_LDAC      PORTA,5

//DAC auf Offset-Karte
#define DAC_2_SYNC  PORTC,5
#define DAC_2_SYNC_PORT PORTC
#define DAC_2_SYNC_PIN  5
#define DAC_2_CLR   PORTA,1
#define DAC_2_LDAC  PORTA,2
*/

#define DAC_R           0b10000000
#define DAC_REG_DAC     0b00000000
#define DAC_REG_RANGE   0b00001000
#define DAC_REG_POWER   0b00010000
#define DAC_REG_CONTROL 0b00011000

#define DAC_ADR_DAC_A   0b00000000
#define DAC_ADR_DAC_B   0b00000010
#define DAC_ADR_DAC_AB  0b00000100



/*
 ** Funktionen
 */

void DAC_Init();
//void DAC_Set(uint16_t val, uint8_t dac, uint8_t ch);
void DAC_Set(uint16_t val, volatile uint8_t *port, uint8_t pin, uint8_t ch);

void LEDSource_set_Heat_Current(uint16_t heatCurrent, uint8_t slot);
void LEDSource_set_Meas_Current(uint16_t measCurrent, uint8_t slot);

void MOSFETSource_set_Heat_Current(uint16_t heatCurrent, uint8_t slot);
void MOSFETSource_set_Meas_Current(uint16_t measCurrent, uint8_t slot);
void MOSFETSource_set_Meas_Voltage(uint16_t measVoltage, uint8_t slot);

void FrontEnd_set_Offset_Voltage(uint16_t offsetVoltage, uint8_t slot);

