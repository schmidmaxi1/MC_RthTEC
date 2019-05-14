/*
 * LTC1864.h
 *
 * Created: 20.11.2018 16:02:56
 *  Author: schmidm
 */ 


/*
 ** Includes
 */

#include "usart_spi.h"

/*
 ** Compiler-Konstanten
 */

//Keine Notwendig


/*
 ** Funktionen
 */

uint32_t Measure_Voltage_MOSFET_with_DataAnalyse();

void Measure_Voltage_MOSFET_without_DataAnalyse();
uint32_t Measure_Voltage_MOSFET_get_Data();

uint32_t Measure_Voltage_LED();