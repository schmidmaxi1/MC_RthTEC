/*
 * FrontEnd.h
 *
 * Created: 19.08.2019 15:39:32
 *  Author: schmidm
 */ 


#define frontEnd_gain_default					25;		//Factor 2
#define frontEnd_offset_voltage_default			3000;		//3.0V


/*
 ** Variablen
 */

uint16_t frontEnd_gain[8];
uint16_t frontEnd_offset_voltage_mV[8];


/*
 ** Functions
 */

void FrontEnd_Init(int slot_nr);
void FrontEnd_Default_Values(int slot_nr);

void FrontEnd_Variables_from_EEPROM(int slot_nr);

void FrontEnd_Set_Gain(uint16_t gain, int slot_nr);
void FrontEnd_Set_Offset_Voltage(uint16_t offset_voltage_in_mV, int slot_nr);