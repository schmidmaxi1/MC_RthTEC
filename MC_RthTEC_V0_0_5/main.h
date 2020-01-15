#ifndef _MAIN_H_
#define _MAIN_H_



/*
 ** User settings
 */

#define F_CPU               16000000UL          //Systemtakt in Hz - Definition als unsigned long (!)


/*
 ** Compiler constants
 */

#define FIRMWARE_VER_R      V0.0.5
#define FIRMWARE_CODE       0x0005

#define CONTROLLER_TYPE_R   RthTEC_mC_Unit

#define FIRMWARE_VERSION    S(CONTROLLER_TYPE_R FIRMWARE_VER_R)
#define CONTROLLER_TYPE     S(CONTROLLER_TYPE_R)

#define DEBUG_ENABLE        0
#define VT100_ENABLE        0

/*
 ** Pin-Setup for MCU-Bord V_2
 */

//1. Signal LEDs
#define LED_1A              PORTG,3			//green
#define LED_1B              PORTG,2			//red
#define LED_DATA            PORTG,0			//green --> Data-Transfer
#define LED_Pulse           PORTG,1			//red	--> Pulse in use


//2. Fail & Enable	
#define FAIL_IN             PORTB,7
#define FAIL_OUT            PORTB,6
#define ENABLE              PORTE,2


//3. Heat-Pulse-Pins (IO x.1) [for up to 8 Slots]
//All on Port H
#define HP_Port	PORTH

#define HP_1	PORTH,0
#define HP_2	PORTH,1
#define HP_3	PORTH,2
#define HP_4	PORTH,3
#define HP_5	PORTH,4
#define HP_6	PORTH,5
#define HP_7	PORTH,6
#define HP_8	PORTH,7

//4. Meas-Pulse-Pins (IO x.2)[for up to 8 Slots]
//All on Port K
#define MP_Port	PORTK

#define MP_1	PORTK,0
#define MP_2	PORTK,1
#define MP_3	PORTK,2
#define MP_4	PORTK,3
#define MP_5	PORTK,4
#define MP_6	PORTK,5
#define MP_7	PORTK,6
#define MP_8	PORTK,7

//5. ChipSelct-Pins (IO x.3)[for up to 8 Slots]
//All on Port A
#define IO_Port3		PORTA

#define IO_PORT3_1			PORTA,0
#define IO_PORT3_2			PORTA,1
#define IO_PORT3_3			PORTA,2
#define IO_PORT3_4			PORTA,3
#define IO_PORT3_5			PORTA,4
#define IO_PORT3_6			PORTA,5
#define IO_PORT3_7			PORTA,6
#define IO_PORT3_8			PORTA,7

//6. ChipSelct-Pins (IO x.4)[for up to 8 Slots]
//All on Port J
#define IO_PORT4			PORTJ

#define IO_PORT4_1			PORTJ,0
#define IO_PORT4_2			PORTJ,1
#define IO_PORT4_3			PORTJ,2
#define IO_PORT4_4			PORTJ,3
#define IO_PORT4_5			PORTJ,4
#define IO_PORT4_6			PORTJ,5
#define IO_PORT4_7			PORTJ,6
#define IO_PORT4_8			PORTJ,7

//7. ChipSelct-Pins (IO x.5)[for up to 8 Slots]
//All on Port D
#define IO_PORT5			PORTD

#define IO_PORT5_1			PORTD,0
#define IO_PORT5_2			PORTD,1
#define IO_PORT5_3			PORTD,2
#define IO_PORT5_4			PORTD,3
#define IO_PORT5_5			PORTD,4
#define IO_PORT5_6			PORTD,5
#define IO_PORT5_7			PORTD,6
#define IO_PORT5_8			PORTD,7


//8. ChipSelct-Pins (IO x.6)[for up to 8 Slots]
//All on Port D
#define IO_PORT6			PORTC

#define IO_PORT6_1			PORTC,0
#define IO_PORT6_2			PORTC,1
#define IO_PORT6_3			PORTC,2
#define IO_PORT6_4			PORTC,3
#define IO_PORT6_5			PORTC,4
#define IO_PORT6_6			PORTC,5
#define IO_PORT6_7			PORTC,6
#define IO_PORT6_8			PORTC,7



/*
 ** Includes
 */

#include "helper.h"
#include "avr/io.h"
#include "avr/interrupt.h"
#include "avr/eeprom.h"
#include "avr/wdt.h"
#include "stdlib.h"
#include "stdio.h"
#include "avr/power.h"

#include "uart0.h"
#include "ad5752.h"
#include "LTC1864.h"
#include "my_Timers.h"

#include "Cards/Slot_Tester.h"
#include "Cards/MOSFET_BreakDown.h"
#include "Cards/LED_Source.h"
#include "Cards/Amplifier.h"
#include "Cards/FrontEnd.h"

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
