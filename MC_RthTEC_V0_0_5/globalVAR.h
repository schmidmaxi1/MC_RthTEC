/*
 * globalVAR.h
 *
 * Created: 20.03.2020 13:04:46
 *  Author: schmidm
 */ 


#ifndef GLOBALVAR_H_
#define GLOBALVAR_H_

#include "avr/eeprom.h"

//external makes the variable visible for all .c files, knowing that is decleard

//*******************************************************************
//							EEPROM
//*******************************************************************

//General
extern uint16_t firmware_code_eeprom EEMEM;

//Slot-Settings: Active and Type
extern uint8_t pulse_output_register_eeprom EEMEM;
extern char card_Type_register_eeprom[8] EEMEM;

//Slot-Parameters
extern uint16_t parameter1_eeprom[8] EEMEM;
extern uint16_t parameter2_eeprom[8] EEMEM;
extern uint16_t parameter3_eeprom[8] EEMEM;

//Pulse Settings (Time):
extern uint32_t heat_pulse_length_eeprom EEMEM;
extern uint32_t measure_pulse_length_eeprom EEMEM;

extern uint16_t deterministic_pulse_length_eeprom EEMEM;
extern uint16_t deterministic_pulse_cycles_eeprom EEMEM;

//*******************************************************************
//							Variables
//*******************************************************************

//Measurment_storage for every slot (19.03.2020, Maxi)
//Dependent on Card-Type if necessary
//Global for storage save
extern uint16_t measured_binary_heat[8];
extern uint16_t measured_binary_meas[8];

//For Pulse Output Register
extern uint8_t pulse_output_register;
extern char card_Type[8];

extern uint32_t heat_pulse_length;
extern uint32_t measure_pulse_length;
extern uint16_t deterministic_pulse_length;
extern uint16_t deterministic_pulse_cycles;

extern uint8_t current_source_enabled;





#endif /* GLOBALVAR_H_ */