/*
 * MOSFET_BreakDown.c
 *
 * Created: 27.06.2019 16:01:29
 *  Author: schmidm
 */ 

/*
 ** Includes
 */

#include "../Config.h" //Doppelpunkte um einen Ordner zurück zu gehen

#include "../main.h"	//Doppelpunkte um einen Ordner zurück zu gehen
#include <util/delay.h>
#include "../helper.h"

#include "MOSFET_BreakDown.h"

#include "../ICs/AD5752.h"
#include "../ICs/MCP23S08.h"
#include "../ICs/LTC1864.h"

uint16_t breakDown_V_GS_mV[8];

/*
PinBelegung:
1. HP	(N.C.)
2. MP	(N.C.)
3. /ChipSelect DAC
4. /ChipSelect ADC-->U_DS
5. /ChipSelect ADC-->I_DS
6. /ChipSelect IO-Expander

EEPROM:
Parameter1 = V_GS
Parameter2 = not used
Parameter3 = not used

IO0: Relay0: DS --> COM
IO1: Relay1: DS --> GND
IO2: Relay2: DS --> SMU
IO3: Relay3: GS --> SMU
IO4: Relay4: GS --> DAC
IO5: Relay5: GS --> GND
IO6: N.C.
IO7: N.C.

*/

//*******************************************************************
//						 Init & EEPROM
//*******************************************************************

void MOSFET_BreakDown_Init(int slot_nr)
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
	//Range: +10V
	//Only Channel A
	//DAC_AD5752_Range_and_PowerUp(Range_p5V, PowerUp_A, &IO_Port3, slot_nr-1);
	DAC_AD5752_Range_and_PowerUp(Range_pm5V, PowerUp_A, &IO_PORT3, slot_nr-1);
	
	//Register in MCP23S08:
	//All as Outputs
	//0 means OUT, 1 means IN
	IO_Expander_set_Register(register_IODIR, 0x00, &IO_PORT6, slot_nr-1);

	//All to LOW (Relays off)
	IO_Expander_set_Register(register_OLAT, 0x00, &IO_PORT6, slot_nr-1);
	
	//Set V_GS
	BreakDown_Set_V_GS_mV(breakDown_V_GS_mV[slot_nr-1], slot_nr-1);
}

void BreakDown_Variables_from_EEPROM(int slot_nr)
{
	breakDown_V_GS_mV[slot_nr-1] = eeprom_read_word(&parameter1_eeprom[slot_nr-1]);
}

void BreakDown_Default_Values(int slot_nr)
{
	//Set default values (Local & EEPROM)
	breakDown_V_GS_mV[slot_nr-1] = breakDown_V_GS_mV_default;
	
	eeprom_write_word(&parameter1_eeprom[slot_nr-1], breakDown_V_GS_mV[slot_nr-1]);
}


//*******************************************************************
//						 Relay - FCTs
//*******************************************************************

void B_Set_Relais_to_BreakDownTest(int slot_nr)
{
	//Switch of all relays
	IO_Expander_set_Register(register_OLAT, 0x00, &IO_PORT6, slot_nr-1);
	
	//Wait to be save that switched of
	_delay_ms(100);
	
	//Switch on relay 2 & 5
	//GS-->GND & DS-->SMU
	IO_Expander_set_Register(register_OLAT, 0x24, &IO_PORT6, slot_nr-1); 
}

void B_Set_Relais_to_Leakage_GS(int slot_nr)
{
	//Switch of all relays
	IO_Expander_set_Register(register_OLAT, 0x00, &IO_PORT6, slot_nr-1);
	
	//Wait to be save that switched of
	_delay_ms(100);
	
	//Switch on relay 1 & 3
	//GS-->SMU & DS-->GND
	IO_Expander_set_Register(register_OLAT, 0x0A, &IO_PORT6, slot_nr-1); 
}

void B_Set_Relais_to_Characteristic_Curve(int slot_nr)
{
	//Switch of all relays
	IO_Expander_set_Register(register_OLAT, 0x00, &IO_PORT6, slot_nr-1);
	
	//Wait to be save that switched of
	_delay_ms(100);
	
	//Switch on relay 0 & 4
	//GS-->DAC & DS-->COM
	IO_Expander_set_Register(register_OLAT, 0x11, &IO_PORT6, slot_nr-1); 
}

void B_Set_Relais_to_BodyDiode_Curve(int slot_nr)
{
	//Switch of all relays
	IO_Expander_set_Register(register_OLAT, 0x00, &IO_PORT6, slot_nr-1);
	
	//Wait to be save that switched of
	_delay_ms(100);
	
	//Switch on relay 2 & 4
	//GS-->DAC & DS-->SMU
	IO_Expander_set_Register(register_OLAT, 0x14, &IO_PORT6, slot_nr-1); 
}

void B_Set_Relais_all_off(int slot_nr)
{
	//Switch of all relays
	IO_Expander_set_Register(register_OLAT, 0x00, &IO_PORT6, slot_nr-1);
	
	//Wait to be save that switched of
	_delay_ms(100);	
}

//*******************************************************************
//						 Setting - FCTs
//*******************************************************************

void BreakDown_Set_V_GS_mV(int16_t V_GS_mV, int slot_nr)
{
			/*
	Input:
	-VoltageVoltag in mV
	-channel (Slot_Number)
	-Offset always an ChipSelect [channel] & Channel A
	-Output is form -20000mV to 20000mV
	*/
	
	//Calculate binary value
	uint16_t binary_value = (((int32_t)V_GS_mV) * 0xffff) / 40000;
	V_GS_mV = binary_value;
	//Send
	DAC_AD5752_Set(binary_value, &IO_PORT3, slot_nr-1, DAC_ADR_DAC_A);		
}

//*******************************************************************
//						 Getting - FCTs
//*******************************************************************

int B_Get_V_DS_in_mV(int slot_nr)
{
	//get binary value
	uint32_t temp = LTC1864_getBIT_OneShot(&IO_PORT4, slot_nr-1);
	
	//Convert (DAC Range: 0...2,5V / Pre-Divider: 16 --> Whole Range 0...40000mV)
	return (temp * 40000)>>16;
}

int B_Get_I_DS_in_mA(int slot_nr)
{
	//get binary value
	uint32_t temp = LTC1864_getBIT_OneShot(&IO_PORT5, slot_nr-1);
	
	//Convert (DAC Range: 0...2,5V / R=50mOhm, Gain = 25 --> 0...2A)
	return (temp * 2000)>>16;
}

//*******************************************************************
//							  Terminal
//*******************************************************************

void Terminal_SET_BreakDown(char *myMessage)
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
				MOSFET_BreakDown_Init(mySlotNr);
			
				//Answer
				TransmitString("SIN");
				TransmitInt(mySlotNr, 1);
				TransmitStringLn("B");
			}
			break;
		
		//2. Relays for Breakdown
		case _MK16('R','B'):
			if (myMessage[6] == '1' && myMessage[7] == '\n')
			{
				//Switch Relays
				B_Set_Relais_to_BreakDownTest(mySlotNr);

				//Answer
				TransmitString("SRB");
				TransmitInt(mySlotNr, 1);
				TransmitStringLn("B=1");
			}
			else if (myMessage[6] == '0' && myMessage[7] == '\n')
			{
				//Switch Relays
				B_Set_Relais_all_off(mySlotNr);

				//Answer
				TransmitString("SRB");
				TransmitInt(mySlotNr, 1);
				TransmitStringLn("B=0");
			}
			else
			{
				//no number
				TransmitStringLn("FORMAT ERR");
			}
			break;
		
		//3. Relays for Leakage
		case _MK16('R','L'):
			if (myMessage[6] == '1' && myMessage[7] == '\n')
			{
				//Switch Relays
				B_Set_Relais_to_Leakage_GS(mySlotNr);

				//Answer
				TransmitString("SRL");
				TransmitInt(mySlotNr, 1);
				TransmitStringLn("B=1");
			}
			else if (myMessage[6] == '0' && myMessage[7] == '\n')
			{
				//Switch Relays off
				B_Set_Relais_all_off(mySlotNr);

				//Answer
				TransmitString("SRL");
				TransmitInt(mySlotNr, 1);
				TransmitStringLn("B=0");
			}
			else
			{
				//no number
				TransmitStringLn("FORMAT ERR");
			}
			break;
		
		//4. Relays for Characteristic Curve
		case _MK16('R','C'):
			if (myMessage[6] == '1' && myMessage[7] == '\n')
			{
				//Switch Relays
				B_Set_Relais_to_Characteristic_Curve(mySlotNr);

				//Answer
				TransmitString("SRC");
				TransmitInt(mySlotNr, 1);
				TransmitStringLn("B=1");
			}
			else if (myMessage[6] == '0' && myMessage[7] == '\n')
			{
				//Switch Relays off
				B_Set_Relais_all_off(mySlotNr);

				//Answer
				TransmitString("SRC");
				TransmitInt(mySlotNr, 1);
				TransmitStringLn("B=0");
			}
			else
			{
				//no number
				TransmitStringLn("FORMAT ERR");
			}
			break;
		
		//5. Relays for BodyDiode
		case _MK16('R','D'):
			if (myMessage[6] == '1' && myMessage[7] == '\n')
			{
				//Switch Relays
				B_Set_Relais_to_BodyDiode_Curve(mySlotNr);

				//Answer
				TransmitString("SRD");
				TransmitInt(mySlotNr, 1);
				TransmitStringLn("B=1");
			}
			else if (myMessage[6] == '0' && myMessage[7] == '\n')
			{
				//Switch Relays off
				B_Set_Relais_all_off(mySlotNr);

				//Answer
				TransmitString("SRD");
				TransmitInt(mySlotNr, 1);
				TransmitStringLn("B=0");
			}
			else
			{
				//no number
				TransmitStringLn("FORMAT ERR");
			}
			break;
			
		//6. Set V_GS voltage
		case _MK16('V', 'G'):
			if (!ParseIntLn(&myMessage[6],5,&temp16))
			{
				//no number
				TransmitStringLn("FORMAT ERR");
			}
			else if (temp16 > 20000 || temp16 < -20000)
			{
				//0 bis 10 V
				TransmitStringLn("NUMBER ERR");
			}
			else
			{
				TransmitString("SVG");
				TransmitInt(mySlotNr, 1);
				TransmitString("B=");
				TransmitFloat(temp16,1, 3);
				TransmitStringLn(" V");

				if (breakDown_V_GS_mV[mySlotNr-1] != temp16)
				{
					breakDown_V_GS_mV[mySlotNr-1] = temp16;
					eeprom_write_word(&parameter1_eeprom[mySlotNr-1], temp16);
					BreakDown_Set_V_GS_mV(breakDown_V_GS_mV[mySlotNr-1], mySlotNr);
				}
			}
			break;
			
		default:
			TransmitStringLn("COMMAND ERR");
			break;
		
	}
}

void Terminal_GET_BreakDown(char *myMessage)
{
	//myMessage:	whole message
	//myCMD:		2 indicator Chars (Position 1 and 2)
	//mySlotNr:		Slot Number (Position 3)
		
	//Local variables
	uint16_t myCMD = _MK16(myMessage[1],myMessage[2]);
	int8_t mySlotNr = myMessage[3] - '0';
	
	switch(myCMD){
							
		//1. Voltage GS (set)
		case _MK16('V','G'):
			TransmitString("GVG");
			TransmitInt(mySlotNr, 1);
			TransmitString("B=");
			TransmitInt(breakDown_V_GS_mV[mySlotNr-1], 1);
			TransmitStringLn(" V");
			break;
			
		//2. Voltage DS (measured)
		case _MK16('V','D'):
			TransmitString("GVD");
			TransmitInt(mySlotNr, 1);
			TransmitString("B=");
			TransmitInt(B_Get_V_DS_in_mV(mySlotNr), 1);
			TransmitStringLn(" mV");
			break;			

		//3. Current DS (measured)
		case _MK16('C','D'):
		TransmitString("GCD");
		TransmitInt(mySlotNr, 1);
		TransmitString("B=");
		TransmitInt(B_Get_I_DS_in_mA(mySlotNr), 1);
		TransmitStringLn(" mA");
		break;
												
		//Default --> Fehler
		default:
			TransmitStringLn("COMMAND ERR");
			break;							
	}

}