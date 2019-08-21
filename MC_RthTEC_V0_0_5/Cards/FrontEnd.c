/*
 * FrontEnd.c
 *
 * Created: 19.08.2019 15:39:12
 *  Author: schmidm
 */


#include "../main.h"	//Doppelpunkte um einen Ordner zurück zu gehen
#include "../helper.h"


#include "FrontEnd.h"

#include "../ICs/AD5752.h"
#include "../ICs/MCP23S08.h"
#include "../ICs/LTC1864.h"


uint16_t frontEnd_gain[8];
uint16_t frontEnd_offset_voltage_mV[8];

/*
PinBelegung:
1. HP
2. MP
3. /ChipSelect DAC (Channel A: Offset_Voltage)
4. /ChipSelect IO-Expander
5. /ChipSelect ADC (Complete Range)
6. NC

EEPROM:
Parameter1 = Gain
Parameter2 = Offset_Voltage_in_mV
Parameter3 = not used

IO-Expander:
IO_0: (OUT) W0 (Window Select 0)
IO_1: (OUT) W1 (Window Select 0)
IO_2: (OUT) FlipFlop RST (LOW--> RST of FlipFlops)
IO_3: (IN)  WINDOWIII (at least on out of Range appeared)
IO_4: (IN)  OUT_Q_IC58 (last out of Range was too LOW)
IO_5: (IN)  OUT_Q_IC49 (last out of Range was too HIGH)
IO_6: (IN)  N$157 (not over Range right now)
IO_7: (IN)  N$148 (not under Ranger right now)

*/

//*******************************************************************
//						 Init & EEPROM
//*******************************************************************

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
	
	//IO Expander:
	//Set Direction (0:2 is OUT, 3:7 is IN)
	//0 means OUT, 1 means IN
	IO_Expander_set_Register(register_IODIR, 0b11111000, &IO_PORT4, slot_nr -1 );
	
	//All Output to Low, except FlipFlop RST
	IO_Expander_set_Register(register_OLAT, 0b00000100,  &IO_PORT4, slot_nr -1);
	
	
	//Set Offset and GAIN
	FrontEnd_Set_Gain(frontEnd_gain[slot_nr-1], slot_nr-1);		
	FrontEnd_Set_Offset_Voltage(frontEnd_offset_voltage_mV[slot_nr-1], slot_nr-1);			
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

//*******************************************************************
//						 Setting - FCTs
//*******************************************************************

void FrontEnd_Set_Gain(uint16_t gain, int slot_nr)
{
	//Reset bit for FlipFlops always HIGH
	switch(gain)
	{
		case 25:
			IO_Expander_set_Register(register_OLAT, 0b00000100,  &IO_PORT4, slot_nr -1);
			break;
		case 50:
			IO_Expander_set_Register(register_OLAT, 0b00000101,  &IO_PORT4, slot_nr -1);
			break;
		case 100:
			IO_Expander_set_Register(register_OLAT, 0b00000110,  &IO_PORT4, slot_nr -1);
			break;
		default:
			break;
	}	
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
//						Further - FCTs
//*******************************************************************

uint16_t FrontEnd_Get_Voltage_in_mV_FullRange(int slot_nr)
{
	//get binary value
	uint32_t temp = LTC1864_getBIT_OneShot(&IO_PORT5, slot_nr-1);
	
	//Convert (DAC Range: 0...2,5V / Pre-Divider: 4 --> Whole Range 0...10000mV)
	return (temp * 10000)>>16; 	
}

void FrontEnd_Reset_FlipFlops(int slot_nr)
{
	//Get current setup of IO-Expander (only the two gain bit necessary; last two)
	uint8_t temp = IO_Expander_get_Register(register_GPIO, &IO_PORT4, slot_nr -1 ) & 0x03;
	
	//Reset Bit (3 Position) to LOW 
	IO_Expander_set_Register(register_OLAT, temp, &IO_PORT4, slot_nr-1);
	//Reset Bit (3 Position) to HIGH
	IO_Expander_set_Register(register_OLAT, temp | 0x04, &IO_PORT4, slot_nr-1);	
}

//*******************************************************************
//							  Terminal
//*******************************************************************

void Terminal_SET_FrontEnd(char *myMessage)
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
			
			
		//2. Window Gain 
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
					frontEnd_gain[mySlotNr-1] = temp16;
					eeprom_write_word(&parameter1_eeprom[mySlotNr-1], temp16);
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

		case _MK16('R','F'):
			if (myMessage[5] != '\n')
			{
				//to many signs
				TransmitStringLn("FORMAT ERR");
			}
			else
			{
				FrontEnd_Reset_FlipFlops(mySlotNr);
				//Answer
				TransmitString("SRF");
				TransmitInt(mySlotNr, 1);
				TransmitStringLn("F");
			}
			break;
			
								
		//Default --> Fehler
		default:
			TransmitStringLn("COMMAND ERR");
			break;		
				
	}
}

void Terminal_GET_FrontEnd(char *myMessage)
{
	//myMessage:	whole message
	//myCMD:		2 indicator Chars (Position 1 and 2)
	//mySlotNr:		Slot Number (Position 3)
		
	//Local variables
	uint16_t myCMD = _MK16(myMessage[1],myMessage[2]);
	int8_t mySlotNr = myMessage[3] - '0';
	
	switch(myCMD){
							
		//1. Gain
		case _MK16('W','G'):
			TransmitString("GWG");
			TransmitInt(mySlotNr, 1);
			TransmitString("F=");
			TransmitInt(frontEnd_gain[mySlotNr-1], 1);
			TransmitStringLn(" W/O-Unit");
			break;
							
		//2. Offset
		case _MK16('W','O'):
			TransmitString("GWO");
			TransmitInt(mySlotNr, 1);
			TransmitString("F=");
			TransmitFloat(frontEnd_offset_voltage_mV[mySlotNr-1], 1, 3);
			TransmitStringLn(" V");
			break;
			
		//3. Status Info
		case _MK16('S','I'):
			TransmitString("GSI");
			TransmitInt(mySlotNr, 1);
			TransmitString("F=");
			TransmitByte(IO_Expander_get_Register(register_GPIO, &IO_PORT4, mySlotNr-1));
			TransmitStringLn("");
			break;			
			
		//4. Voltage at full range DAC
		case _MK16('V','F'):
			TransmitString("GVF");
			TransmitInt(mySlotNr, 1);
			TransmitString("F=");
			//Measure and return
			TransmitInt(FrontEnd_Get_Voltage_in_mV_FullRange(mySlotNr), 1);
			TransmitStringLn(" mV");
			break;
			
							
		//Default --> Fehler
		default:
			TransmitStringLn("COMMAND ERR");
			break;							
	}

}