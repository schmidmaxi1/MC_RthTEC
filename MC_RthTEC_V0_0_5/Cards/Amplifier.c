/*
 * Amplifier.c
 *
 * Created: 11.07.2019 17:10:52
 *  Author: schmidm
 */ 

#include "../Config.h" //Doppelpunkte um einen Ordner zurück zu gehen

#include "../main.h"	//Doppelpunkte um einen Ordner zurück zu gehen
#include <util/delay.h>
#include "../helper.h"

#include "Amplifier.h"

#include "../ICs/AD5752.h"


uint16_t amplifier_gain[8];
uint16_t amplifier_offset_voltage_mV[8];

/*
PinBelegung:
1. HP
2. MP
3. /ChipSelect DAC (Channel A: Offset_Voltage)
4. NC
5. NC
6. NC

EEPROM:
Parameter1 = Gain
Parameter2 = Offset_Voltage_in_mV
Parameter3 = not used

*/

void Amplifier_Init(int slot_nr)
{
	//Set HP&MP to LOW, and all CS to HIGH
	_clear_bit(HP_Port, slot_nr - 1);
	_clear_bit(MP_Port, slot_nr - 1);
	_set_bit(IO_PORT3, slot_nr - 1);
	_set_bit(IO_PORT4, slot_nr - 1);
	_set_bit(IO_PORT5, slot_nr - 1);
	_set_bit(IO_PORT6, slot_nr - 1);
	//Set all as Output
	_set_out(HP_Port, slot_nr - 1);
	_set_out(MP_Port, slot_nr - 1);
	_set_out(IO_PORT3, slot_nr - 1);
	_set_out(IO_PORT4, slot_nr - 1);
	_set_out(IO_PORT5, slot_nr - 1);
	_set_out(IO_PORT6, slot_nr - 1);
	

	//ADC initialization
	//Range: +5V
	//Both Channels on
	DAC_AD5752_Range_and_PowerUp(Range_p5V, PowerUp_A, &IO_PORT3, slot_nr-1);	
	
	//Set DAC output
	Amplifier_Set_Gain(amplifier_gain[slot_nr-1], slot_nr);		
	Amplifier_Set_Offset_Voltage(amplifier_offset_voltage_mV[slot_nr-1], slot_nr);	
}

void Amplifier_Variables_from_EEPROM(int slot_nr)
{		
	amplifier_gain[slot_nr-1] = eeprom_read_word(&parameter1_eeprom[slot_nr-1]);
	amplifier_offset_voltage_mV[slot_nr-1] = eeprom_read_word(&parameter2_eeprom[slot_nr-1]);
}

void Amplifier_Default_Values(int slot_nr)
{
	//Set default values (Local & EEPROM)
	amplifier_gain[slot_nr-1] = amplifier_gain_default	;
	amplifier_offset_voltage_mV[slot_nr-1] = amplifier_offset_voltage_default;
	
	eeprom_write_word(&parameter1_eeprom[slot_nr-1], amplifier_gain[slot_nr-1]);
	eeprom_write_word(&parameter2_eeprom[slot_nr-1], amplifier_offset_voltage_mV[slot_nr-1]);
}


void Amplifier_Set_Gain(uint16_t gain, int slot_nr)
{
	//Is fixed to 2	
}
void Amplifier_Set_Offset_Voltage(uint16_t voltage_in_mV, int slot_nr)
{
		/*
	Input:
	-measCurrent in mV
	-channel (Slot-Number)
	-Offset always at ChipSelect [channel] & Chanel A
	-Adjustable from 0V ... 5V
	-Additional OpAmp with Gain 2
	*/
	
	//Calculate binary value
	uint16_t binary_value = (((uint32_t) voltage_in_mV) * 0xffff) / 5000 / 2;
	
	//Send
	DAC_AD5752_Set(binary_value, &IO_PORT3, slot_nr-1, DAC_ADR_DAC_A);
	
}