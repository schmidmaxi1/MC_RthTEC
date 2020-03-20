/*
 * LED_Source.c
 *
 * Created: 04.07.2019 15:37:01
 *  Author: schmidm
 */ 

#include "../Config.h" //Doppelpunkte um einen Ordner zurück zu gehen
#include "../helper.h"

#include "../main.h"	//Doppelpunkte um einen Ordner zurück zu gehen
#include <util/delay.h>


#include "LED_Source.h"
#include "../ICs/AD5752.h"

uint16_t led_Source_Heat_Current_mA[8];
uint16_t led_Source_Meas_Current_0mA1[8];

/*
PinBelegung:
1. HP
2. MP
3. /ChipSelect DAC (Channel A: Heat_Current, Channel B: Meas_Current)
4. NC
5. NC
6. NC

EEPROM:
Parameter1 = HeatCurrent in mA
Parameter2 = MeasCurrent in 0,1mA
Parameter3 = not used

Don't forget to add a new Card to the functions:
-Init_All_Cards(char newCard_Type[], char oldCard_Type[])
-EEPROM_last_Values()

*/

//*******************************************************************
//						 Init & EEPROM
//*******************************************************************

void LED_Source_Init(int slot_nr)
{
	//Set HP&MP to LOW, and all CS to HIGH
	_clear_bit(HP_Port, slot_nr - 1);
	_clear_bit(MP_Port, slot_nr - 1);
	_set_bit(IO_PORT3, slot_nr - 1);
	_set_bit(IO_PORT4, slot_nr - 1);
	_set_bit(IO_PORT5, slot_nr - 1);
	_set_bit(IO_PORT6, slot_nr - 1);
	//Set all as Output
	_set_out(HP_Port, slot_nr - 1);
	_set_out(MP_Port, slot_nr - 1);
	_set_out(IO_PORT3, slot_nr - 1);
	_set_out(IO_PORT4, slot_nr - 1);
	_set_out(IO_PORT5, slot_nr - 1);
	_set_out(IO_PORT6, slot_nr - 1);
	

	//ADC initialization
	//Range: +5V
	//Both Channels on
	DAC_AD5752_Range_and_PowerUp(Range_p5V, PowerUp_AB, &IO_PORT3, slot_nr-1);	
	
	//Set DAC output
	LED_Source_Set_Heat_Current(led_Source_Heat_Current_mA[slot_nr-1], slot_nr);		
	LED_Source_Set_Meas_Current(led_Source_Meas_Current_0mA1[slot_nr-1], slot_nr);	
}

void LED_Source_Variables_from_EEPROM(int slot_nr)
{		
	led_Source_Heat_Current_mA[slot_nr-1] = eeprom_read_word(&parameter1_eeprom[slot_nr-1]);
	led_Source_Meas_Current_0mA1[slot_nr-1] = eeprom_read_word(&parameter2_eeprom[slot_nr-1]);
}

void LED_Source_Default_Values(int slot_nr)
{
	//Set default values (Local & EEPROM)
	led_Source_Heat_Current_mA[slot_nr-1] = led_source_Heat_current_default;
	led_Source_Meas_Current_0mA1[slot_nr-1] = led_source_Meas_current_default;
	
	eeprom_write_word(&parameter1_eeprom[slot_nr-1], led_Source_Heat_Current_mA[slot_nr-1]);
	eeprom_write_word(&parameter2_eeprom[slot_nr-1], led_Source_Meas_Current_0mA1[slot_nr-1]);
}

//*******************************************************************
//						 Setting - FCTs
//*******************************************************************

void LED_Source_Set_Heat_Current(uint16_t current_mA, int slot_nr)
{
	//Binär Wert berechnen
	volatile uint16_t binary_value = (((uint32_t) current_mA) * 0xffff) / 1500;
	
	//Senden
	DAC_AD5752_Set(binary_value, &IO_PORT3, slot_nr-1, DAC_ADR_DAC_A);
	
}

void LED_Source_Set_Meas_Current(uint16_t current_10th_mA, int slot_nr)
{
		/*
	Input:
	-measCurrent in 0,1mA
	-channel (Einschubkarten-Platz)
	-Heizquelle immer an ChipSelect [channel] & Kanal B
	*/
	
	//Binär Wert berechnen
	uint16_t binary_value = (((uint32_t) current_10th_mA) * 0xffff) / 250;
	
	//Senden
	DAC_AD5752_Set(binary_value, &IO_PORT3, slot_nr-1, DAC_ADR_DAC_B);
	
}

//*******************************************************************
//						 Getting - FCTs
//*******************************************************************

//Non necessary

//*******************************************************************
//							  Terminal
//*******************************************************************

void Terminal_SET_LED_Source(char *myMessage)
{
	//myMessage:	whole message
	//myCMD:		2 indicator Chars (Position 1 and 2)
	//mySlotNr:		Slot Number (Position 3)
	
	//Local variables
	uint16_t myCMD = _MK16(myMessage[1],myMessage[2]);
	int8_t mySlotNr = myMessage[3] - '0';
	int16_t temp16 = 0;

	//Dependent on indicator Chars --> Command
	switch(myCMD)
	{
		//4.1 Initialization
		case _MK16('I','N'):
			if (myMessage[5] != '\n')
			{
				//to many signs
				TransmitStringLn("FORMAT ERR");
			}
			else
			{
				LED_Source_Init(mySlotNr);
			
				//Answer
				TransmitString("SIN");
				TransmitInt(mySlotNr, 1);
				TransmitStringLn("L");
			}
			break;
		
		
		//4.2 Set Heat Current
		case _MK16('H','C'):
			if (!ParseIntLn(&myMessage[6],4,&temp16))
			{
				//no number
				TransmitStringLn("FORMAT ERR");
			}
			else if (temp16 > 1500 || temp16 < 0)
			{
				//new current limit not between 0 and 1.5A
				TransmitStringLn("NUMBER ERR");
			}
			else
			{
				TransmitString("SHC");
				TransmitInt(mySlotNr, 1);
				TransmitString("L=");
				TransmitInt(temp16, 1);
				TransmitStringLn(" mA");

				if (led_Source_Heat_Current_mA[mySlotNr-1] != temp16)
				{
					led_Source_Heat_Current_mA[mySlotNr-1] = temp16;
					eeprom_write_word(&parameter1_eeprom[mySlotNr-1], temp16);
					LED_Source_Set_Heat_Current(led_Source_Heat_Current_mA[mySlotNr-1], mySlotNr);
				}
			}
			break;
		
		//4.3 Set Meas Current
		case _MK16('M','C'):
			if (!ParseIntLn(&myMessage[6],3,&temp16))
			{
				//no number
				TransmitStringLn("FORMAT ERR");
			}
			else if (temp16 > 250 || temp16 < 50)
			{
				//new current limit not between 5 and 25mA
				TransmitStringLn("NUMBER ERR");
			}
			else
			{
				TransmitString("SMC");
				TransmitInt(mySlotNr, 1);
				TransmitString("L=");
				TransmitFloat(temp16, 1, 1);
				TransmitStringLn(" mA");

				if (led_Source_Meas_Current_0mA1[mySlotNr-1] != temp16)
				{
					led_Source_Meas_Current_0mA1[mySlotNr-1] = temp16;
					eeprom_write_word(&parameter2_eeprom[mySlotNr-1],temp16);
					LED_Source_Set_Meas_Current(led_Source_Meas_Current_0mA1[mySlotNr-1], mySlotNr);
				}
			
			}
			break;
		
		//Default --> Fehler
		default:
			TransmitStringLn("COMMAND ERR");
			break;		
		
	}
}

void Terminal_GET_LED_Source(char *myMessage)
{
	//myMessage:	whole message
	//myCMD:		2 indicator Chars (Position 1 and 2)
	//mySlotNr:		Slot Number (Position 3)
	
	//Local variables
	uint16_t myCMD = _MK16(myMessage[1],myMessage[2]);
	int8_t mySlotNr = myMessage[3] - '0';


	switch(myCMD){
		//4.1.1 Init is not necessary
		
		//4.1.2 Set Heat Current
		case _MK16('H','C'):
			TransmitString("GHC");
			TransmitInt(mySlotNr, 1);
			TransmitString("L=");
			TransmitInt(led_Source_Heat_Current_mA[mySlotNr-1], 1);
			TransmitStringLn(" mA");
			break;
								
		//4.1.3 Set Meas Current
		case _MK16('M','C'):
			TransmitString("GMC");
			TransmitInt(mySlotNr, 1);
			TransmitString("L=");
			TransmitFloat(led_Source_Meas_Current_0mA1[mySlotNr-1], 1, 1);
			TransmitStringLn(" mA");
			break;
								
		//Default --> Fehler
		default:
			TransmitStringLn("COMMAND ERR");
			break;		
	}		
}