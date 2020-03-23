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
//#include "globalVAR.h"



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

volatile uint8_t interrupt_1ms;
uint8_t interrupt_10hz;
uint8_t interrupt_1hz;
volatile uint8_t startup_wait_counter;
uint8_t debug_send;

//Interface
uint8_t terminal_timeout;





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


//Pulses
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
