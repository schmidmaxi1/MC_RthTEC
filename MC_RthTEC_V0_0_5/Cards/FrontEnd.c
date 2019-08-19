/*
 * FrontEnd.c
 *
 * Created: 19.08.2019 15:39:12
 *  Author: schmidm
 */


#include "../main.h"	//Doppelpunkte um einen Ordner zurück zu gehen
#include <util/delay.h>
#include "../helper.h"

#include "FrontEnd.h"

#include "../ICs/AD5752.h"
#include "../ICs/MCP23S08.h"


uint16_t frontEnd_gain[8];
uint16_t frontEnd_offset_voltage_mV[8];

/*
PinBelegung:
1. HP
2. MP
3. /ChipSelect DAC (Channel A: Offset_Voltage)
4. /ChipSelect IO-Expander
5. /ChipSelect ADC (Grob)
6. NC

EEPROM:
Parameter1 = Gain
Parameter2 = Offset_Voltage_in_mV
Parameter3 = not used

*/

void FrontEnd_Init(int slot_nr)
{
	//Set HP&MP to LOW, and all CS to HIGH
	_clear_bit(HP_Port, slot_nr - 1);
	_clear_bit(MP_Port, slot_nr - 1);
	_set_bit(IO_Port3, slot_nr - 1);
	_set_bit(IO_PORT4, slot_nr - 1);
	_set_bit(IO_PORT5, slot_nr - 1);
	_set_bit(IO_PORT6, slot_nr - 1);
	//Set all as Output
	_set_out(HP_Port, slot_nr - 1);
	_set_out(MP_Port, slot_nr - 1);
	_set_out(IO_Port3, slot_nr - 1);
	_set_out(IO_PORT4, slot_nr - 1);
	_set_out(IO_PORT5, slot_nr - 1);
	_set_out(IO_PORT6, slot_nr - 1);
	

	//ADC initialization
	//Range: +10V
	//Only Channel A
	//DAC_AD5752_Range_and_PowerUp(Range_p5V, PowerUp_A, &IO_Port3, slot_nr-1);	
	DAC_AD5752_Range_and_PowerUp(Range_p10V, PowerUp_A, &IO_Port3, slot_nr-1);	
	
	//Set DAC output
	FrontEnd_Set_Gain(frontEnd_gain[slot_nr-1], slot_nr);		
	FrontEnd_Set_Offset_Voltage(frontEnd_offset_voltage_mV[slot_nr-1], slot_nr);	
}

void FrontEnd_Variables_from_EEPROM(int slot_nr)
{		
	frontEnd_gain[slot_nr-1] = eeprom_read_word(&parameter1_eeprom[slot_nr-1]);
	frontEnd_offset_voltage_mV[slot_nr-1] = eeprom_read_word(&parameter2_eeprom[slot_nr-1]);
}

void FrontEnd_Default_Values(int slot_nr)
{
	//Set default values (Local & EEPROM)
	frontEnd_gain[slot_nr-1] = frontEnd_gain_default;
	frontEnd_offset_voltage_mV[slot_nr-1] = frontEnd_offset_voltage_default;
	
	eeprom_write_word(&parameter1_eeprom[slot_nr-1], frontEnd_gain[slot_nr-1]);
	eeprom_write_word(&parameter2_eeprom[slot_nr-1], frontEnd_offset_voltage_mV[slot_nr-1]);
}


void FrontEnd_Set_Gain(uint16_t gain, int slot_nr)
{
	//Is fixed to 2		
}
void FrontEnd_Set_Offset_Voltage(uint16_t voltage_in_mV, int slot_nr)
{
		/*
	Input:
	-offSet Voltag in mV
	-channel (Einschubkarten-Platz)
	-Offset immer an ChipSelect [channel] & Kanal A
	*/
	
	//Binär Wert berechnen
	uint16_t binary_value = (((uint32_t) voltage_in_mV) * 0xffff) / 10000;
	
	//Senden
	DAC_AD5752_Set(binary_value, &IO_Port3, slot_nr-1, DAC_ADR_DAC_A);	
} 


//*******************************************************************
//							  Terminal
//*******************************************************************

void Terminal_SET_FrontEND(char *myMessage)
{
	//myCMD:		2 indicator Chars
	//myMessage:	whole message
	//mySlotNr:		Slot Number
	
	//Local variables
	uint16_t myCMD = _MK16(myMessage[1],myMessage[2]);
	int8_t mySlotNr = myMessage[3] - '0';
	int16_t temp16 = 0;
	
	//Dependent on indicator Chars --> Command
	switch(myCMD)
	{
		//1. Initialization
		case _MK16('I','N'):
			if (myMessage[5] != '\n')
			{
				//to many signs
				TransmitStringLn("FORMAT ERR");
			}
			else
			{
				FrontEnd_Init(mySlotNr);									
				//Answer
				TransmitString("SIN");
				TransmitInt(mySlotNr, 1);
				TransmitStringLn("F");
			}
			break;
			
			
		//2. Window Gain (Fixed to 2)
		case _MK16('W','G'):
						
			if (!ParseIntLn(&myMessage[6],4,&temp16))
			{
				//no number
				TransmitStringLn("FORMAT ERR");
			}
			else if ( !(temp16 == 100 || temp16 == 50 || temp16 == 25) )
			{
				//Wrong Value
				TransmitStringLn("NUMBER ERR");
			}
			else
			{
				TransmitString("SWG");
				TransmitInt(mySlotNr, 1);
				TransmitString("F=");
				TransmitInt(temp16, 1);
				TransmitStringLn(" W/O-Unit");

				if (frontEnd_gain[mySlotNr-1] != temp16)
				{
					frontEnd_gain[mySlotNr-1] = 2;
					eeprom_write_word(&parameter1_eeprom[mySlotNr-1], 2);
					FrontEnd_Set_Gain(frontEnd_gain[mySlotNr-1], mySlotNr);
				}
			}
			break;
			
			
		//3. Set Window Offset Voltage
		case _MK16('W','O'):
						
			if (!ParseIntLn(&myMessage[6],4,&temp16))
			{
				//no number
				TransmitStringLn("FORMAT ERR");
			}
			else if (temp16 > 10000 || temp16 < 0)
			{
				//0 bis 10 V
				TransmitStringLn("NUMBER ERR");
			}
			else
			{
				TransmitString("SWO");
				TransmitInt(mySlotNr, 1);
				TransmitString("F=");
				TransmitFloat(temp16,1, 3);
				TransmitStringLn(" V");

				if (frontEnd_offset_voltage_mV[mySlotNr-1] != temp16)
				{
					frontEnd_offset_voltage_mV[mySlotNr-1] = temp16;
					eeprom_write_word(&parameter2_eeprom[mySlotNr-1], temp16);
					FrontEnd_Set_Offset_Voltage(frontEnd_offset_voltage_mV[mySlotNr-1], mySlotNr);
				}
			}
			break;
						
		//Default --> Fehler
		default:
			TransmitStringLn("COMMAND ERR");
			break;		
				
	}
}