/*
 * LED_Source.c
 *
 * Created: 04.07.2019 15:37:01
 *  Author: schmidm
 */ 


#include "../main.h"	//Doppelpunkte um einen Ordner zurück zu gehen
#include <util/delay.h>
#include "../helper.h"

#include "LED_Source.h"

#include "../ICs/AD5752.h"

uint16_t led_Source_Heat_Current_mA[8];
uint16_t led_Source_Meas_Current_0mA1[8];

/*
PinBelegung:
1. HP
2. MP
3. /ChipSelect DAC
4. NC
5. NC
6. NC

EEPROM:
Parameter1 = HeatCurrent in mA
Parameter2 = MeasCurrent in 0,1mA
Parameter3 = not used

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
	

	//ADC initialization
	//Range: +5V
	//Both Channels on
	DAC_AD5752_Range_and_PowerUp(Range_p5V, PowerUp_AB, &IO_Port3, slot_nr-1);	
	
	//Set DAC output
	LED_Source_Set_Heat_Current(led_Source_Heat_Current_mA[slot_nr-1], slot_nr);		
	LED_Source_Set_Meas_Current(led_Source_Meas_Current_0mA1[slot_nr-1], slot_nr);	
}

void LED_Source_Variables_from_EEPROM(int slot_nr)
{		
	led_Source_Heat_Current_mA[slot_nr-1] = eeprom_read_word(&parameter1_eeprom[slot_nr-1]);
	led_Source_Meas_Current_0mA1[slot_nr-1] = eeprom_read_word(&parameter2_eeprom[slot_nr-1]);
}

void LED_Source_Default_Values(int slot_nr)
{
	//Set default values (Local & EEPROM)
	led_Source_Heat_Current_mA[slot_nr-1] = led_source_Heat_current_default;
	led_Source_Meas_Current_0mA1[slot_nr-1] = led_source_Meas_current_default;
	
	eeprom_write_word(&parameter1_eeprom[slot_nr-1], led_Source_Heat_Current_mA[slot_nr-1]);
	eeprom_write_word(&parameter2_eeprom[slot_nr-1], led_Source_Meas_Current_0mA1[slot_nr-1]);
}


void LED_Source_Set_Heat_Current(uint16_t current_mA, int slot_nr)
{
	//Binär Wert berechnen
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
	
	//Binär Wert berechnen
	uint16_t binary_value = (((uint32_t) current_10th_mA) * 0xffff) / 250;
	
	//Senden
	DAC_AD5752_Set(binary_value, &IO_Port3, slot_nr-1, DAC_ADR_DAC_B);
	
}