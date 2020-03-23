/*
 * globalVAR.c
 *
 * Created: 20.03.2020 13:12:33
 *  Author: schmidm
 */ 

#include "globalVAR.h"

//*******************************************************************
//							EEPROM
//*******************************************************************

//General
uint16_t firmware_code_eeprom EEMEM;

//Slot-Settings: Active and Type
uint8_t pulse_output_register_eeprom EEMEM;
char card_Type_register_eeprom[8] EEMEM;

//Slot-Parameters
uint16_t parameter1_eeprom[8] EEMEM;
uint16_t parameter2_eeprom[8] EEMEM;
uint16_t parameter3_eeprom[8] EEMEM;

//Pulse Settings (Time):
uint32_t heat_pulse_length_eeprom EEMEM;
uint32_t measure_pulse_length_eeprom EEMEM;

uint16_t deterministic_pulse_length_eeprom EEMEM;
uint16_t deterministic_pulse_cycles_eeprom EEMEM;

//*******************************************************************
//							Variables
//*******************************************************************


//Measurment_storage for every slot (19.03.2020, Maxi)
//Dependent on Card-Type if necessary
//Global for storage save
uint16_t measured_binary_heat[8];
uint16_t measured_binary_meas[8];

//For Pulse Output Register
uint8_t pulse_output_register;
char card_Type[8];

uint32_t heat_pulse_length;
uint32_t measure_pulse_length;

//Edit: Maxi 26.09.2018
uint16_t deterministic_pulse_length;
uint16_t deterministic_pulse_cycles;

uint8_t current_source_enabled;