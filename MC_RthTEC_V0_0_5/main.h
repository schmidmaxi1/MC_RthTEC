#ifndef _MAIN_H_
#define _MAIN_H_







/*
 ** Includes
 */


#include "avr/io.h"
#include "avr/interrupt.h"
#include "avr/eeprom.h"
#include "avr/wdt.h"
#include "stdlib.h"
#include "stdio.h"
#include "avr/power.h"

#include "Config.h" //Doppelpunkte um einen Ordner zurück zu gehen
#include "helper.h"



#include "uart0.h"
//#include "ad5752.h"
#include "usart_spi.h"
#include "my_Timers.h"

#include "Cards/Slot_Tester.h"
#include "Cards/MOSFET_BreakDown.h"
#include "Cards/LED_Source.h"
#include "Cards/Amplifier.h"
#include "Cards/FrontEnd.h"
#include "Cards/MOSFET_Source.h"

#include "ICs/MCP23S08.h"


/*
 ** Variables
 */

//General
extern uint16_t firmware_code_eeprom EEMEM;
volatile uint8_t interrupt_1ms;
uint8_t interrupt_10hz;
uint8_t interrupt_1hz;
volatile uint8_t startup_wait_counter;
uint8_t debug_send;

//Interface
uint8_t terminal_timeout;

//Controller
uint8_t current_source_enabled;
uint8_t led_hp;
uint8_t led_mp;

uint16_t heat_pulse_current;
uint32_t heat_pulse_length;
uint16_t measure_pulse_current;
uint32_t measure_pulse_length;

uint16_t measure_pulse_voltage;

//Edit: Maxi 20.12.17
uint16_t offset_voltage;


extern uint16_t heat_pulse_current_eeprom EEMEM;
extern uint32_t heat_pulse_length_eeprom EEMEM;
extern uint16_t measure_pulse_current_eeprom EEMEM;
extern uint32_t measure_pulse_length_eeprom EEMEM;

extern uint16_t measure_pulse_voltage_eeprom EEMEM;

//Edit: Maxi 20.12.17
extern uint16_t offset_voltage_eeprom EEMEM;

int16_t window_offset;

extern uint16_t window_offset_eeprom EEMEM;


//Edit: Maxi 26.09.2018
uint16_t deterministic_pulse_length;
uint16_t deterministic_pulse_cycles;

extern uint16_t deterministic_pulse_length_eeprom EEMEM;
extern uint16_t deterministic_pulse_cycles_eeprom EEMEM;

//For Pulse Output Register
uint8_t pulse_output_register;
extern uint8_t pulse_output_register_eeprom EEMEM;

char card_Type[8];
extern char card_Type_register_eeprom[8] EEMEM;

//EEPROM Register neu (08.07.2018, Maxi)
extern uint16_t parameter1_eeprom[8] EEMEM;
extern uint16_t parameter2_eeprom[8] EEMEM;
extern uint16_t parameter3_eeprom[8] EEMEM;

//Measurment_storage for every slot (19.03.2020, Maxi)
//Dependent on Card-Type if necessary
//Global for storage save
uint16_t measured_binary_heat[8];
uint16_t measured_binary_meas[8];

//Flags
uint8_t flag_std_TTA;
uint8_t flag_DPA_TTA;
uint8_t flag_HPP_TTA;

//Edit: Maxi 20.11.2018
uint32_t heat_pulse_voltage;
uint32_t measure_pules_voltage;


/*
 ** Functions
 */

//Main
void Init();
void PulseStart_stdTTA();
void PulseStop();

void PulseStart_Sensitivity();		//edit: Maxi 07112017

void Init_IO_Pins();
void Lauflicht();
void EEPROM_default_Values();
void EEPROM_last_Values();
void Init_All_Cards(char newCard_Type[], char oldCard_Type[]);

void PulseStart_DPA_TTA();	//edit: Maxi 26.09.2018

void PulseStart_DPA_TTA_HighStart();			//edit: Maxi 12.9.2018
void PulseStart_DPA_TTA_fromHPP();

void PulseStart_PrePulse();			//edit: Maxi 27.09.2018

//Terminal
void TerminalInit();
void TerminalWaitCount();
void TerminalCheckCommand();
void TerminalParseCommand(char *string);
void TransmitAll();
void TransmitString(char *string);
void TransmitStringLn(char *string);
void TransmitInt(int16_t i, uint8_t digits);
void TransmitLong(int32_t i, uint8_t digits);
void TransmitInt0(int16_t i, uint8_t digits);
void TransmitFloat(int16_t i, uint8_t digits, uint8_t div);

extern uint8_t ParseIntLn(char *string, uint8_t digits, int16_t *num);



#endif
