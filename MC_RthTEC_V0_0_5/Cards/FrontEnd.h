/*
 * FrontEnd.h
 *
 * Created: 19.08.2019 15:39:32
 *  Author: schmidm
 */ 

//*******************************************************************
//								Includes
//*******************************************************************

#include "../Config.h"		//Doppelpunkte um einen Ordner zurück zu gehen
#include "../helper.h"
#include "../globalVAR.h"
#include "../Serial_ReadWrite.h"

#include "../ICs/AD5752.h"
#include "../ICs/MCP23S08.h"
#include "../ICs/LTC1864.h"

//*******************************************************************
//								Default
//*******************************************************************

#define frontEnd_gain_default					25;			//Factor 25
#define frontEnd_offset_voltage_default			3000;		//3.0V

//*******************************************************************
//								Variables
//*******************************************************************

uint16_t frontEnd_gain[8];
uint16_t frontEnd_offset_voltage_mV[8];


//*******************************************************************
//								Functions
//*******************************************************************

//Init
void FrontEnd_Init(int slot_nr);
void FrontEnd_Default_Values(int slot_nr);
void FrontEnd_Variables_from_EEPROM(int slot_nr);

//Set
void FrontEnd_Set_Gain(uint16_t gain, int slot_nr);
void FrontEnd_Set_Offset_Voltage(uint16_t offset_voltage_in_mV, int slot_nr);

//Terminal
void Terminal_SET_FrontEnd(char *myMessage);
void Terminal_GET_FrontEnd(char *myMessage);