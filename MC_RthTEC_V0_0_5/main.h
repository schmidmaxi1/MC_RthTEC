#ifndef _MAIN_H_
#define _MAIN_H_


//*******************************************************************
//								Includes
//*******************************************************************

#include <stdio.h>


//*******************************************************************
//								Variables
//*******************************************************************

//Interface
uint8_t terminal_timeout;


//*******************************************************************
//								Functions
//*******************************************************************

//Main - Loop
void Init();

//Inits
void Init_IO_Pins();
void Lauflicht();
void EEPROM_default_Values();
void EEPROM_last_Values();
void Init_All_Cards(char newCard_Type[], char oldCard_Type[]);

//Terminal
void TerminalInit();
void TerminalWaitCount();
void TerminalCheckCommand();
void TerminalParseCommand(char *string);



#endif
