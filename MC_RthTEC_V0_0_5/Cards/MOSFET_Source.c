/*
 * MOSFET_Source.c
 *
 * Created: 09.03.2020 09:39:25
 *  Author: schmidm
 */ 



#include "../main.h"	//Doppelpunkte um einen Ordner zurück zu gehen



#include "MOSFET_Source.h"


uint16_t mosfet_Source_Heat_Current_mA[8];
uint16_t mosfet_Source_Meas_Current_0mA1[8];
uint16_t mosfet_Source_Meas_Voltage_10mV[8];
//Heat-Voltage is adjusted by an external voltage-Source

/*
PinBelegung:
1. HP
2. MP
3. /ChipSelect DAC (Channel A: Heat_Current, Channel B: Meas_Current)
4. /ChipSelect DAC (Channel A: Meas_Voltage, Channel B: N.C.)
5. /ChipSelect ADC (1/8 of V_DS voltage)
6. NC

EEPROM:
Parameter1 = HeatCurrent in mA
Parameter2 = MeasCurrent in 0,1mA
Parameter3 = MeasVoltage in 0,1V

Don't forget to add a new Card to the functions:
-Init_All_Cards(char newCard_Type[], char oldCard_Type[])
-EEPROM_last_Values()

*/

//*******************************************************************
//						 Init & EEPROM
//*******************************************************************

void MOSFET_Source_Init(int slot_nr)
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
	DAC_AD5752_Range_and_PowerUp(Range_p5V, PowerUp_AB, &IO_PORT4, slot_nr-1);
	
	//Set DAC output
	MOSFET_Source_Set_Heat_Current(mosfet_Source_Heat_Current_mA[slot_nr-1], slot_nr);
	MOSFET_Source_Set_Meas_Current(mosfet_Source_Meas_Current_0mA1[slot_nr-1], slot_nr);
	MOSFET_Source_Set_Meas_Voltage(mosfet_Source_Meas_Voltage_10mV[slot_nr-1], slot_nr);
}

void MOSFET_Source_Variables_from_EEPROM(int slot_nr)
{
	mosfet_Source_Heat_Current_mA[slot_nr-1] = eeprom_read_word(&parameter1_eeprom[slot_nr-1]);
	mosfet_Source_Meas_Current_0mA1[slot_nr-1] = eeprom_read_word(&parameter2_eeprom[slot_nr-1]);
	mosfet_Source_Meas_Voltage_10mV[slot_nr-1] = eeprom_read_word(&parameter3_eeprom[slot_nr-1]);
}

void MOSFET_Source_Default_Values(int slot_nr)
{
	//Set default values (Local & EEPROM)
	mosfet_Source_Heat_Current_mA[slot_nr-1] = mosfet_source_Heat_current_default;
	mosfet_Source_Meas_Current_0mA1[slot_nr-1] = mosfet_source_Meas_current_default;
	mosfet_Source_Meas_Voltage_10mV[slot_nr-1] = mosfet_source_Meas_voltage_default;
	
	eeprom_write_word(&parameter1_eeprom[slot_nr-1], mosfet_Source_Heat_Current_mA[slot_nr-1]);
	eeprom_write_word(&parameter2_eeprom[slot_nr-1], mosfet_Source_Meas_Current_0mA1[slot_nr-1]);
	eeprom_write_word(&parameter3_eeprom[slot_nr-1], mosfet_Source_Meas_Voltage_10mV[slot_nr-1]);
}

//*******************************************************************
//						 Setting - FCTs
//*******************************************************************

void MOSFET_Source_Set_Heat_Current(uint16_t current_mA, int slot_nr)
{
	/*
	Input:
	-heatCurrent in 1mA
	-channel (Einschubkarten-Platz)
	-Heizquelle immer an ChipSelect [channel] & Kanal A
	-I_max = 5000
	*/
	
	//Binär Wert berechnen
	uint16_t binary_value = (((uint32_t) current_mA) * 0xffff) / 5000;
		
	//Senden
	DAC_AD5752_Set(binary_value, &IO_PORT3, slot_nr-1, DAC_ADR_DAC_A);
}

void MOSFET_Source_Set_Meas_Current(uint16_t current_10th_mA, int slot_nr)
{
	/*
	Input:
	-measCurrent in 0,1mA
	-channel (Einschubkarten-Platz)
	-Heizquelle immer an ChipSelect [channel] & Kanal B
	-I_max = 25mA
	*/
	
	//Binär Wert berechnen
	uint16_t binary_value = (((uint32_t) current_10th_mA) * 0xffff) / 250;
	
	//Senden
	DAC_AD5752_Set(binary_value, &IO_PORT3, slot_nr-1, DAC_ADR_DAC_B);	

}

void MOSFET_Source_Set_Meas_Voltage(uint16_t voltage_10mV, int slot_nr)
{
	/*
	Input:
	-measCurrent in 0,1V
	-channel (Einschubkarten-Platz)
	-Heizquelle immer an ChipSelect2 [channel] & Kanal B
	-U_max = 20V
	*/
	
	//Binär Wert berechnen (URef = 0,33 * U_A- 1,61V)
	volatile uint16_t binary_value = ((((uint32_t)voltage_10mV * 10) / 3 - 1610) * 0xffff) / 5000;
		
	//Senden
	DAC_AD5752_Set(binary_value, &IO_PORT4, slot_nr-1, DAC_ADR_DAC_A);
}

//*******************************************************************
//						 Getting - FCTs
//*******************************************************************

int MOSFET_Source_Get_measured_Heat_Voltage_in_mV(int slot_nr)
{
	//V_in_ADC_max = 2.5V; TP = 1:11
	return (((uint32_t) measured_binary_heat[slot_nr-1]) * 2500 * 11) / 0xffff;
}

int MOSFET_Source_Get_measured_Meas_Voltage_in_mV(int slot_nr)
{
	//V_in_ADC_max = 2.5V; TP = 1:11
	return (((uint32_t) measured_binary_meas[slot_nr-1]) * 2500 * 11) / 0xffff;
}

//*******************************************************************
//					 Measure in Timer - FCTs
//*******************************************************************

void MOSFET_Source_sample_Heat(int slot_nr)
{
	//Samples the ADC but not collects the data
	LTC1864_getBIT_LastSample_and_NewShot(&IO_PORT5, slot_nr-1);
}

uint16_t MOSFET_Source_sample_Meas_receive_Heat(int slot_nr)
{
	//Samples the ADC but not collects the data
	return LTC1864_getBIT_LastSample_and_NewShot(&IO_PORT5, slot_nr-1);
}

uint16_t MOSFET_Source_receive_Meas(int slot_nr)
{
	//Samples the ADC but not collects the data
	return LTC1864_getBIT_LastSample_and_NewShot(&IO_PORT5, slot_nr-1);
}


//*******************************************************************
//							  Terminal
//*******************************************************************

void Terminal_SET_MOSFET_Source(char *myMessage)
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
		//4.2.1 Initialization
		case _MK16('I','N'):
			if (myMessage[5] != '\n')
			{
				//to many signs
				TransmitStringLn("FORMAT ERR");
			}
			else
			{
				MOSFET_Source_Init(mySlotNr);
					
				//Answer
				TransmitString("SIN");
				TransmitInt(mySlotNr, 1);
				TransmitStringLn("M");
			}
			break;
	
		//4.2.2 Set Heat Current
		case _MK16('H','C'):
			if (!ParseIntLn(&myMessage[6],4,&temp16))
			{
				//no number
				TransmitStringLn("FORMAT ERR");
			}
			else if (temp16 > 5000 || temp16 < 0)
			{
				//new current limit not between 0 and 1.5A
				TransmitStringLn("NUMBER ERR");
			}
			else
			{
				TransmitString("SHC MOSFET=");
				TransmitInt(temp16, 1);
				TransmitStringLn(" mA");

				if (mosfet_Source_Heat_Current_mA[mySlotNr-1] != temp16)
				{
					mosfet_Source_Heat_Current_mA[mySlotNr-1] = temp16;
					eeprom_write_word(&parameter1_eeprom[mySlotNr-1],temp16);

					MOSFET_Source_Set_Heat_Current(mosfet_Source_Heat_Current_mA[mySlotNr-1], mySlotNr);
					//DAC_Set((((uint32_t) mosfet_Source_Heat_Current_mA[mySlotNr-1]) * 0xffff) / 1500, DAC_1, DAC_ADR_DAC_A);
				}
			}
			break;
	
		//4.2.3 Set Meas Current
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
				TransmitString("SMC MOSFET=");
				TransmitFloat(temp16, 1, 1);
				TransmitStringLn(" mA");

				if (mosfet_Source_Meas_Current_0mA1[mySlotNr-1] != temp16)
				{
					mosfet_Source_Meas_Current_0mA1[mySlotNr-1] = temp16;
					eeprom_write_word(&parameter2_eeprom[mySlotNr-1],temp16);
			
					MOSFET_Source_Set_Meas_Current(mosfet_Source_Meas_Current_0mA1[mySlotNr-1], mySlotNr);
					//DAC_Set((((uint32_t) measure_pulse_current) * 0xffff) / 250, DAC_1, DAC_ADR_DAC_B);
				}
		
			}
	
			break;
	
		//4.2.4 Set Meas Voltage
		case _MK16('M','V'):
			if (!ParseIntLn(&myMessage[6],4,&temp16))
			{
				//no number
				TransmitStringLn("FORMAT ERR");
			}
			else if (temp16 > 2000 || temp16 < 100)
			{
				//new current limit not between 5 and 25mA
				TransmitStringLn("NUMBER ERR");
			}
			else
			{
				TransmitString("SMV MOSFET=");
				TransmitFloat(temp16, 1, 2);
				TransmitStringLn(" V");

				if (mosfet_Source_Meas_Voltage_10mV[mySlotNr-1] != temp16)
				{
					mosfet_Source_Meas_Voltage_10mV[mySlotNr-1] = temp16;
					eeprom_write_word(&parameter3_eeprom[mySlotNr-1],temp16);
			
			
					MOSFET_Source_Set_Meas_Voltage(mosfet_Source_Meas_Voltage_10mV[mySlotNr-1], mySlotNr);
					//DAC_Set((((uint32_t) measure_pulse_current) * 0xffff) / 250, DAC_1, DAC_ADR_DAC_B);
				}

		
			}
	
			break;

		//Default --> Fehler
		default:
			TransmitStringLn("COMMAND ERR");
			break;
	
	}


}

void Terminal_GET_MOSFET_Source(char *myMessage)
{
	//myMessage:	whole message
	//myCMD:		2 indicator Chars (Position 1 and 2)
	//mySlotNr:		Slot Number (Position 3)
	
	//Local variables
	uint16_t myCMD = _MK16(myMessage[1],myMessage[2]);
	int8_t mySlotNr = myMessage[3] - '0';

	switch(myCMD){
						
		//4.2.2 Get adjusted Heat Current
		case _MK16('H','C'):
			TransmitString("GHC");
			TransmitInt(mySlotNr, 1);
			TransmitString("M=");
			TransmitInt(mosfet_Source_Heat_Current_mA[mySlotNr-1], 1);
			TransmitStringLn(" mA");
			break;
						
		//4.2.3 Get adjusted Meas Current
		case _MK16('M','C'):
			TransmitString("GMC");
			TransmitInt(mySlotNr, 1);
			TransmitString("M=");
			TransmitFloat(mosfet_Source_Meas_Current_0mA1[mySlotNr-1], 1, 1);
			TransmitStringLn(" mA");
			break;
			
		//4.2.4 Get adjusted Meas Voltage
		case _MK16('M','V'):
			TransmitString("GMV");
			TransmitInt(mySlotNr, 1);
			TransmitString("M=");
			TransmitFloat(mosfet_Source_Meas_Voltage_10mV[mySlotNr-1], 2, 2);
			TransmitStringLn(" V");
			break;
			
		//4.2.5 Get measured Heat Voltage
		case _MK16('H','M'):
			TransmitString("GHM");
			TransmitInt(mySlotNr, 1);
			TransmitString("M=");
			TransmitFloat(MOSFET_Source_Get_measured_Heat_Voltage_in_mV(mySlotNr), 3, 3);
			TransmitStringLn(" V");
			break;
			
		//4.2.6 Get measured Meas Voltage
		case _MK16('M','M'):
			TransmitString("GMM");
			TransmitInt(mySlotNr, 1);
			TransmitString("M=");
			TransmitFloat(MOSFET_Source_Get_measured_Meas_Voltage_in_mV(mySlotNr), 3, 3);
			TransmitStringLn(" V");
			break;
						
		//Default --> Fehler
		default:
			TransmitStringLn("COMMAND ERR");
			break;
						
	}

}