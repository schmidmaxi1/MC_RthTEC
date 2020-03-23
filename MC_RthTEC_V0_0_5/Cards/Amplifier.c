/*
 * Amplifier.c
 *
 * Created: 11.07.2019 17:10:52
 *  Author: schmidm
 */ 

//*******************************************************************
//								Includes
//*******************************************************************

#include "Amplifier.h"

#include "../Config.h"		//Doppelpunkte um einen Ordner zurück zu gehen
#include "../helper.h"
#include "../globalVAR.h"
#include "../Serial_ReadWrite.h"

#include "../ICs/AD5752.h"

//*******************************************************************
//								Variables
//*******************************************************************

uint16_t amplifier_gain[8];
uint16_t amplifier_offset_voltage_mV[8];

//*******************************************************************
//								Explanation
//*******************************************************************

/*
PinBelegung:
1. HP
2. MP
3. /ChipSelect DAC (Channel A: Offset_Voltage)
4. NC
5. NC
6. NC

EEPROM:
Parameter1 = Gain
Parameter2 = Offset_Voltage_in_mV
Parameter3 = not used

*/

//*******************************************************************
//						 Init & EEPROM
//*******************************************************************

void Amplifier_Init(int slot_nr)
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
	DAC_AD5752_Range_and_PowerUp(Range_p5V, PowerUp_A, &IO_PORT3, slot_nr-1);	
	
	//Set DAC output
	Amplifier_Set_Gain(amplifier_gain[slot_nr-1], slot_nr);		
	Amplifier_Set_Offset_Voltage(amplifier_offset_voltage_mV[slot_nr-1], slot_nr);	
}

void Amplifier_Variables_from_EEPROM(int slot_nr)
{		
	amplifier_gain[slot_nr-1] = eeprom_read_word(&parameter1_eeprom[slot_nr-1]);
	amplifier_offset_voltage_mV[slot_nr-1] = eeprom_read_word(&parameter2_eeprom[slot_nr-1]);
}

void Amplifier_Default_Values(int slot_nr)
{
	//Set default values (Local & EEPROM)
	amplifier_gain[slot_nr-1] = amplifier_gain_default	;
	amplifier_offset_voltage_mV[slot_nr-1] = amplifier_offset_voltage_default;
	
	eeprom_write_word(&parameter1_eeprom[slot_nr-1], amplifier_gain[slot_nr-1]);
	eeprom_write_word(&parameter2_eeprom[slot_nr-1], amplifier_offset_voltage_mV[slot_nr-1]);
}

//*******************************************************************
//						 Setting - FCTs
//*******************************************************************

void Amplifier_Set_Gain(uint16_t gain, int slot_nr)
{
	//Is fixed to 2	
}

void Amplifier_Set_Offset_Voltage(uint16_t voltage_in_mV, int slot_nr)
{
		/*
	Input:
	-measCurrent in mV
	-channel (Slot-Number)
	-Offset always at ChipSelect [channel] & Chanel A
	-Adjustable from 0V ... 5V
	-Additional OpAmp with Gain 2
	*/
	
	//Calculate binary value
	uint16_t binary_value = (((uint32_t) voltage_in_mV) * 0xffff) / 5000 / 2;
	
	//Send
	DAC_AD5752_Set(binary_value, &IO_PORT3, slot_nr-1, DAC_ADR_DAC_A);
	
}

//*******************************************************************
//							  Terminal
//*******************************************************************

void Terminal_SET_Amplifier(char *myMessage)
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
		//4.3.1 Initialization
		case _MK16('I','N'):
			if (myMessage[5] != '\n')
			{
				//to many signs
				TransmitStringLn("FORMAT ERR");
			}
			else
			{
				//Overwrite old settings
				card_Type[mySlotNr-1] = 'A';
				eeprom_write_block(card_Type, &card_Type_register_eeprom, 8);
								
				//Take default values
				Amplifier_Default_Values(mySlotNr);
				//Init
				Amplifier_Init(mySlotNr);
							
				//Answer
				TransmitString("SIN");
				TransmitInt(mySlotNr, 1);
				TransmitStringLn("A");
			}
			break;
		
		//4.3.2 Set Window Offset Voltage
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
				TransmitString("A=");
				TransmitFloat(temp16,1, 3);
				TransmitStringLn(" V");

				if (amplifier_offset_voltage_mV[mySlotNr-1] != temp16)
				{
					amplifier_offset_voltage_mV[mySlotNr-1] = temp16;
					eeprom_write_word(&parameter2_eeprom[mySlotNr-1], temp16);
					Amplifier_Set_Offset_Voltage(amplifier_offset_voltage_mV[mySlotNr-1], mySlotNr);
				}
			}
			break;	
						
		//4.3.3 Window Gain (Fixed to 2)
		case _MK16('W','G'):					
			if (!ParseIntLn(&myMessage[6],4,&temp16))
			{
				//no number
				TransmitStringLn("FORMAT ERR");
			}
			else if (temp16 > 2 || temp16 < 2)
			{
				//Kp mit der versterkung
				TransmitStringLn("NUMBER ERR");
			}
			else
			{
				TransmitString("SWG");
				TransmitInt(mySlotNr, 1);
				TransmitString("A=");
				//TransmitInt(temp16, 1);
				TransmitInt(2, 1);
				TransmitStringLn(" W/O-Unit");

				if (amplifier_gain[mySlotNr-1] != temp16)
				{
					amplifier_gain[mySlotNr-1] = 2;
					eeprom_write_word(&parameter1_eeprom[mySlotNr-1], 2);
					Amplifier_Set_Gain(amplifier_gain[mySlotNr-1], mySlotNr);
				}
			}
		break;
								
		//Default --> Fehler
		default:
			TransmitStringLn("COMMAND ERR");
			break;
		
	}
		
}

void Terminal_GET_Amplifier(char *myMessage)
{
	//myMessage:	whole message
	//myCMD:		2 indicator Chars (Position 1 and 2)
	//mySlotNr:		Slot Number (Position 3)
		
	//Local variables
	uint16_t myCMD = _MK16(myMessage[1],myMessage[2]);
	int8_t mySlotNr = myMessage[3] - '0';
		
	switch(myCMD){
		
		//4.3.2 Offset Voltage
		case _MK16('W','O'):
			TransmitString("GWO");
			TransmitInt(mySlotNr , 1);
			TransmitString("A=");
			TransmitFloat(amplifier_offset_voltage_mV[mySlotNr -1], 1, 3);
			TransmitStringLn(" V");
			break;
		
		//4.3.3 Gain
		case _MK16('W','G'):
			TransmitString("GWG");
			TransmitInt(mySlotNr , 1);
			TransmitString("A=");
			TransmitInt(amplifier_gain[mySlotNr -1], 1);
			TransmitStringLn(" W/O-Unit");
			break;
								
								
		//Default --> Fehler
		default:
			TransmitStringLn("COMMAND ERR");
			break;
		
	}
}