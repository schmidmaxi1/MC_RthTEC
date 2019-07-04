/*
 * LED_Source.c
 *
 * Created: 04.07.2019 15:37:01
 *  Author: schmidm
 */ 


#include "../main.h"	//Doppelpunkte um einen Ordner zur�ck zu gehen
#include <util/delay.h>
#include "../helper.h"

#include "LED_Source.h"

#include "../ICs/AD5752.h"

/*
PinBelegung:
1. HP
2. MP
3. /ChipSelect DAC
4. NC
5. NC
6. NC

IO0: Relay0: DS --> COM
IO1: Relay1: DS --> GND
IO2: Relay2: DS --> SMU
IO3: Relay3: GS --> SMU
IO4: Relay4: GS --> DAC
IO5: Relay5: GS --> GND
IO6: N.C.
IO7: N.C.

*/


void LED_Source_Init(int slot_nr)
{
	//Set HP&MP to LOW, and all CS to HIGH
	_clear_bit(HP_Port, slot_nr - 1);
	_clear_bit(MP_Port, slot_nr - 1);
	_set_bit(IO_Port3, slot_nr - 1);
	_set_bit(IO_PORT4, slot_nr - 1);
	_set_bit(IO_PORT5, slot_nr - 1);
	_set_bit(IO_PORT6, slot_nr - 1);
	//Set all as Output
	_set_out(HP_Port, slot_nr - 1);
	_set_out(MP_Port, slot_nr - 1);
	_set_out(IO_Port3, slot_nr - 1);
	_set_out(IO_PORT4, slot_nr - 1);
	_set_out(IO_PORT5, slot_nr - 1);
	_set_out(IO_PORT6, slot_nr - 1);
	

	//ADC initialization and set to 0V
	//Range: +5V
	//Both Channels on
	DAC_AD5752_Range_and_PowerUp(Range_p5V, PowerUp_AB, &IO_Port3, slot_nr-1);	
}


void LED_Source_Set_Heat_Current(uint16_t current_mA, int slot_nr)
{
	//Bin�r Wert berechnen
	uint16_t binary_value = (((uint32_t) current_mA) * 0xffff) / 1500;
	
	//Senden
	DAC_AD5752_Set(binary_value, &IO_Port3, slot_nr-1, DAC_ADR_DAC_A);
	
}
void LED_Source_Set_Meas_Current(uint16_t current_10th_mA, int slot_nr)
{
		/*
	Input:
	-measCurrent in 0,1mA
	-channel (Einschubkarten-Platz)
	-Heizquelle immer an ChipSelect [channel] & Kanal B
	*/
	
	//Bin�r Wert berechnen
	uint16_t binary_value = (((uint32_t) current_10th_mA) * 0xffff) / 250;
	
	//Senden
	DAC_AD5752_Set(binary_value, &IO_Port3, slot_nr-1, DAC_ADR_DAC_B);
	
}