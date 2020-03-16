/*
 * MOSFET_Source.c
 *
 * Created: 09.03.2020 09:39:25
 *  Author: schmidm
 */ 


#include "../main.h"	//Doppelpunkte um einen Ordner zurück zu gehen
#include <util/delay.h>
#include "../helper.h"

#include "MOSFET_Source.h"

#include "../ICs/AD5752.h"
#include "../ICs/LTC1864.h"


uint16_t mosfet_Source_Heat_Current_mA[8];
uint16_t mosfet_Source_Meas_Current_0mA1[8];
uint16_t mosfet_Source_Meas_Voltage_0V1[8];

//Heat-Voltage is adjusted by an external voltage-Source


/*
PinBelegung:
1. HP
2. MP
3. /ChipSelect DAC (Channel A: Heat_Current, Channel B: Meas_Current)
4. /ChipSelect DAC (Channel A: Meas_Voltage, Channel B: N.C.)
5. /ChipSelect ADC (1/8 of V_DS voltage)
6. NC

EEPROM:
Parameter1 = HeatCurrent in mA
Parameter2 = MeasCurrent in 0,1mA
Parameter3 = MeasVoltage in 0,1V

*/

void MOSFET_Source_Init(int slot_nr)
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
	DAC_AD5752_Range_and_PowerUp(Range_p5V, PowerUp_AB, &IO_PORT4, slot_nr-1);
	
	//Set DAC output
	MOSFET_Source_Set_Heat_Current(mosfet_Source_Heat_Current_mA[slot_nr-1], slot_nr);
	MOSFET_Source_Set_Meas_Current(mosfet_Source_Meas_Current_0mA1[slot_nr-1], slot_nr);
	MOSFET_Source_Set_Meas_Voltage(mosfet_Source_Meas_Voltage_0V1[slot_nr-1], slot_nr);
}

void MOSFET_Source_Variables_from_EEPROM(int slot_nr)
{
	mosfet_Source_Heat_Current_mA[slot_nr-1] = eeprom_read_word(&parameter1_eeprom[slot_nr-1]);
	mosfet_Source_Meas_Current_0mA1[slot_nr-1] = eeprom_read_word(&parameter2_eeprom[slot_nr-1]);
	mosfet_Source_Meas_Voltage_0V1[slot_nr-1] = eeprom_read_word(&parameter3_eeprom[slot_nr-1]);
}

void MOSFET_Source_Default_Values(int slot_nr)
{
	//Set default values (Local & EEPROM)
	mosfet_Source_Heat_Current_mA[slot_nr-1] = mosfet_source_Heat_current_default;
	mosfet_Source_Meas_Current_0mA1[slot_nr-1] = mosfet_source_Meas_current_default;
	mosfet_Source_Meas_Voltage_0V1[slot_nr-1] = mosfet_source_Meas_voltage_default;
	
	eeprom_write_word(&parameter1_eeprom[slot_nr-1], mosfet_Source_Heat_Current_mA[slot_nr-1]);
	eeprom_write_word(&parameter2_eeprom[slot_nr-1], mosfet_Source_Meas_Current_0mA1[slot_nr-1]);
	eeprom_write_word(&parameter3_eeprom[slot_nr-1], mosfet_Source_Meas_Voltage_0V1[slot_nr-1]);
}

void MOSFET_Source_Set_Heat_Current(uint16_t current_mA, int slot_nr)
{
	/*
	Input:
	-heatCurrent in 0,1mA
	-channel (Einschubkarten-Platz)
	-Heizquelle immer an ChipSelect [channel] & Kanal A
	-I_max = 5000
	*/
	
	//Binär Wert berechnen
	uint16_t binary_value = (((uint32_t) current_mA) * 0xffff) / 5000;
		
	//Senden
	DAC_AD5752_Set(binary_value, &IO_Port3, slot_nr-1, DAC_ADR_DAC_A);
}
void MOSFET_Source_Set_Meas_Current(uint16_t current_10th_mA, int slot_nr)
{
	/*
	Input:
	-measCurrent in 0,1mA
	-channel (Einschubkarten-Platz)
	-Heizquelle immer an ChipSelect [channel] & Kanal B
	-I_max = 25mA
	*/
	
	//Binär Wert berechnen
	uint16_t binary_value = (((uint32_t) current_10th_mA) * 0xffff) / 250;
	
	//Senden
	DAC_AD5752_Set(binary_value, &IO_Port3, slot_nr-1, DAC_ADR_DAC_B);	

}
void MOSFET_Source_Set_Meas_Voltage(uint16_t current_10th_V, int slot_nr)
{
	/*
	Input:
	-measCurrent in 0,1V
	-channel (Einschubkarten-Platz)
	-Heizquelle immer an ChipSelect2 [channel] & Kanal B
	-U_max = 20V
	*/
	
	//Binär Wert berechnen
	uint16_t binary_value = (((uint32_t) current_10th_V) * 0xffff) / 200;
		
	//Senden
	DAC_AD5752_Set(binary_value, &IO_PORT4, slot_nr-1, DAC_ADR_DAC_A);
}