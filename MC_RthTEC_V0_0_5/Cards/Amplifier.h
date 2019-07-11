/*
 * Amplifier.h
 *
 * Created: 11.07.2019 17:11:06
 *  Author: schmidm
 */ 


#define amplifier_gain_default					2;		//Factor 2
#define amplifier_offset_voltage_default		3000;		//3.0V


/*
 ** Variablen
 */

uint16_t amplifier_gain[8];
uint16_t amplifier_offset_voltage_mV[8];


/*
 ** Functions
 */

void Amplifier_Init(int slot_nr);
void Amplifier_Default_Values(int slot_nr);

void Amplifier_Variables_from_EEPROM(int slot_nr);

void Amplifier_Set_Gain(uint16_t gain, int slot_nr);
void Amplifier_Set_Offset_Voltage(uint16_t offset_voltage_in_mV, int slot_nr);