/* ----------------------------------------------------------
 *
 * Project:  LED Test system
 * Module:   Terminal
 *
 * Author:   Johannes Knauss 2017, mail@iet-chiemsee.de
 *
 *
 * ----------------------------------------------------------
 */



/*
 ** Includes
 */

#include <string.h>

#include "main.h"



/*
 ** Variables
 */

uint8_t len;
char *string;



/*
 ** Functions
 */

uint8_t ParseIntLn(char *string, uint8_t digits, int16_t *num);
uint8_t ParseLongLn(char *string, uint8_t digits, int32_t *num);
uint8_t ParseBool(char *string, uint8_t *value);
uint8_t ParseByte(char *string, uint8_t *value);
void TransmitByte(uint8_t byte);

//This line is used to suppress the warnings for code Folding
// It is ignored by the compiler
//At the end of the Code it must be closed
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"



//////////////////////////////////////////////////////////////////////////////////
//									Initialization								//
//////////////////////////////////////////////////////////////////////////////////

void TerminalInit()
{
    //UART (USB)
    UART0Init();

    len = 0;

#if VT100_ENABLE
    TransmitString("\033[?25l\033[0;0H\033[37;44m\033[2J");

    TransmitString("\033[4m");
    TransmitString(FIRMWARE_VERSION);
    TransmitString("\033[24m");
#endif

}


//////////////////////////////////////////////////////////////////////////////////
//								Check for Command								//
//////////////////////////////////////////////////////////////////////////////////

void TerminalCheckCommand()
{
    //USB specific
    if ((len = UART0ReceiveLine()) > 0)
    {
		string = uart0_rx;
    }

    //Common commands
    if (len > 0)
    {
        TerminalParseCommand(string);
    }
}

//////////////////////////////////////////////////////////////////////////////////
//							Search witch Command								//
//////////////////////////////////////////////////////////////////////////////////

void TerminalParseCommand(char *string)
{
	uint16_t cmd = _MK16(string[1],string[2]);
	int8_t slotNr = 0;
	uint8_t temp8 = 0;
	int16_t temp16 = 0;
	int32_t temp32 = 0;
		
	//***************************************************************************
	//									RESET
	//***************************************************************************
	
	#pragma region Reset
	
	if (string[0] == 'R')
	{
		//If RST,'reset' command
		if (string[1] == 'S' && string[2] == 'T' && string[3] == '\n')
		{
			current_source_enabled = 0;
				
			TransmitStringLn("reboot");

			wdt_enable(WDTO_15MS);
			while(1);
		}

		//If RST,'reset' command with factory defaults
		else if (string[1] == 'S' && string[2] == 'T' && string[3] == ' ' && string[4] == '1' && string[5] == '\n')
		{
			current_source_enabled = 0;

			eeprom_write_word(&firmware_code_eeprom, 0xFFFF);
				
			TransmitStringLn("factory reset");

			wdt_enable(WDTO_15MS);
			while(1);
		}
	}
	
	#pragma endregion Reset
		
	//***************************************************************************
	//									SET
	//***************************************************************************

	#pragma region Set

	else if (string[0] == 'S')
	{
		//Must executed here, later no chance, before not safe
		slotNr = string[3] - '0';
	
		//*****************************************************************************
		//Set-Commands, that apply for all 19''-Cards (Definition: Space @ 3 character)
		//*****************************************************************************
		if(string[3] == ' '){
			
			//Check the second two characters
			switch (cmd){
			
				//1. Enable************************************************************
				#pragma region 1.Enable		
				case _MK16('E','N'):
				
					if (string[4] == '1' && string[5] == '\n')
					{
						current_source_enabled = 1;
						set_bit(ENABLE);

						TransmitStringLn("SEN=1");
					}
					else if (string[4] == '0' && string[5] == '\n')
					{
						current_source_enabled = 0;
						PulseStop();
						clear_bit(ENABLE);

						TransmitStringLn("SEN=0");
					}
					else
					{
						//no number
						TransmitStringLn("FORMAT ERR");
					}
					break;
					
				#pragma endregion 1.Enable
			
				//2. Start pulses******************************************************
				#pragma region 2.PulseStarten
				//2.1 Start Standard TTA			
				case _MK16('P','S'):
				
					if (string[4] == '1' && string[5] == '\n')
					{
						PulseStart_stdTTA();

						TransmitStringLn("SPS=1");
					}
					else if (string[4] == '0' && string[5] == '\n')
					{
						PulseStop();

						TransmitStringLn("SPS=0");
					}
					else
					{
						//no number
						TransmitStringLn("FORMAT ERR");
					}
					break;
					
				//2.2 Start Sensitivity
				case _MK16('S','S'):
					
					if (string[4] == '1' && string[5] == '\n')
					{
						PulseStart_Sensitivity();

						TransmitStringLn("SSS=1");
					}
					else if (string[4] == '0' && string[5] == '\n')
					{
						PulseStop();

						TransmitStringLn("SSS=0");
					}
					else
					{
						//no number
						TransmitStringLn("FORMAT ERR");
					}
					break;
					
				//2.3 Start deterministic Pulse
				case _MK16('D','S'):
					
					if (string[4] == '1' && string[5] == '\n')
					{
						TransmitStringLn("SDS=1");
						
						PulseStart_detTTA();
					}
					else if (string[4] == '0' && string[5] == '\n')
					{
						PulseStop();

						TransmitStringLn("SDS=0");
					}
					else
					{
						//no number
						TransmitStringLn("FORMAT ERR");
					}
					break;
					
				//2.4 Pre-pulse to get V(Sense, 25°C)
				case _MK16('M','S'):
									
					if (string[4] == '1' && string[5] == '\n')
					{
						TransmitStringLn("SMS=1");
							
						PulseStart_PrePulse();
					}
					else if (string[4] == '0' && string[5] == '\n')
					{
						PulseStop();

						TransmitStringLn("SMS=0");
					}
					else
					{
						//no number
						TransmitStringLn("FORMAT ERR");
					}
					break;
					
				#pragma endregion 2.PulseStarten
			
				//3. Puls settings*****************************************************
				#pragma region 3.Puls-settings
				//3.1 Set Heat Pulse Length
				case _MK16('H','P'):
			
					if (!ParseLongLn(&string[4],6,&temp32))
					{
						//no number
						TransmitStringLn("FORMAT ERR");
					}
					else if ((temp32 > 120000 || temp32 < 25) && temp32 != 0)
					{
						//new length not between 25ms and 120s
						TransmitStringLn("NUMBER ERR");
					}
					else
					{
						TransmitString("SHP=");
						TransmitLong(temp32, 1);
						TransmitStringLn(" ms");

						heat_pulse_length = temp32;
						eeprom_write_dword(&heat_pulse_length_eeprom,temp32);
					}
					break;

				//3.2 Set Measurement Pulse Length
				case _MK16('M','P'):
			
					if (!ParseLongLn(&string[4],6,&temp32))
					{
						//no number
						TransmitStringLn("FORMAT ERR");
					}
					else if ((temp32 > 120000 || temp32 < 25) && temp32 != 0)
					{
						//new length not between 25ms and 120s
						TransmitStringLn("NUMBER ERR");
					}
					else
					{
						TransmitString("SMP=");
						TransmitLong(temp32, 1);
						TransmitStringLn(" ms");

						measure_pulse_length = temp32;
						eeprom_write_dword(&measure_pulse_length_eeprom,temp32);
					}
					break;
					
				//3.3 Set Number of deterministic pulses
				case _MK16('N','D'):
									
					if (!ParseIntLn(&string[4],6,&temp16)) 
					{
						//no number
						TransmitStringLn("FORMAT ERR");
					}
					else if ((temp16 > 1000 || temp16 < 1))
					{
						//new length not between 25ms and 120s
						TransmitStringLn("NUMBER ERR");
					}
					else
					{
						TransmitString("SND=");
						TransmitInt(temp16, 1);
						TransmitStringLn(" ");

						deterministic_pulse_cycles = temp16;
						eeprom_write_word(&deterministic_pulse_cycles_eeprom,temp16);	
										
					}
					break;
					
				//3.4 Set Length of deterministic pulses
				case _MK16('T','D'):
									
					if (!ParseIntLn(&string[4],6,&temp16))
					{
						//no number
						TransmitStringLn("FORMAT ERR");
					}
					else if ((temp16 > 1000 || temp16 < 1))
					{
						//new length not between 25ms and 120s
						TransmitStringLn("NUMBER ERR");
					}
					else
					{
						TransmitString("STD=");
						TransmitFloat(temp16, 1, 1);
						TransmitStringLn(" ms");

						deterministic_pulse_length = temp16;
						eeprom_write_word(&deterministic_pulse_length_eeprom,temp16);
					}
					break;

				//3.5 Set Pulse Register	
				case _MK16('P','R'):
								
					if (!ParseByte(&string[4], &temp8))
					{
						//no 8 bit 
						TransmitStringLn("FORMAT ERR");
					}
					else
					{
						TransmitString("SPR=");
						TransmitByte(temp8);
						TransmitStringLn("");

						pulse_output_register = temp8;
						eeprom_write_byte(&pulse_output_register_eeprom,temp8);
					}
					break;

								
				#pragma endregion 3.Puls-settings
												
				//Default --> Fehler***************************************************	
				default:
					TransmitStringLn("COMMAND ERR");
					break;				
			}						
		}
		
		//*****************************************************************************
		//Set-Commands, that apply ONE 19''-Cards (Definition: Number @ 3 character)
		//*****************************************************************************
						
		else if (slotNr >= 0 && slotNr < 8){
			
			//*****************************************************************************
			//Differentiation between card-Types
			//*****************************************************************************
			
			switch(string[4]){
				
				//1. LED-Heat-Meas current Source
				#pragma region SettingsLED-Source
				
				case 'L':					
					switch(cmd){
							
						//1. Initialization
						case _MK16('I','N'):
							if (string[5] != '\n')
							{
								//to many signs
								TransmitStringLn("FORMAT ERR");
							}
							else
							{
								LED_Source_Init(slotNr);
												
								//Answer
								TransmitString("SIN");
								TransmitInt(slotNr, 1);
								TransmitStringLn("L");
							}
							break;
						//2. Set Heat Current	
						case _MK16('H','C'):
								
							if (!ParseIntLn(&string[6],4,&temp16))
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
								TransmitString("SHC LED=");
								TransmitInt(temp16, 1);
								TransmitStringLn(" mA");

								if (heat_pulse_current != temp16)
								{
									heat_pulse_current = temp16;
									eeprom_write_word(&heat_pulse_current_eeprom,temp16);

									LED_Source_Set_Heat_Current(heat_pulse_current, slotNr);
									//LEDSource_set_Heat_Current(heat_pulse_current, slotNr);
								}						
							}
							break;
	
						//3. Set Meas Current
						case _MK16('M','C'):
						
							if (!ParseIntLn(&string[6],3,&temp16))
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
								TransmitString("SMC LED=");
								TransmitFloat(temp16, 1, 1);
								TransmitStringLn(" mA");

								if (measure_pulse_current != temp16)
								{
									measure_pulse_current = temp16;
									eeprom_write_word(&measure_pulse_current_eeprom,temp16);
									
									LED_Source_Set_Meas_Current(measure_pulse_current, slotNr);
									//LEDSource_set_Meas_Current(measure_pulse_current, slotNr);
								}
								
							}
							break;
						
						//Default --> Fehler
						default:
						TransmitStringLn("COMMAND ERR");
						break;
						
						}			
					break;
					
				#pragma endregion SettingsLED-Source
					
				//2. MOSFET-Heat-Meas current Source
				#pragma region SettingsMOSFET-Source
				
				case 'M':
									
					switch(cmd){
					
						//1. Set Heat Current
						case _MK16('H','C'):						
							if (!ParseIntLn(&string[6],4,&temp16))
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

								if (heat_pulse_current != temp16)
								{
									heat_pulse_current = temp16;
									eeprom_write_word(&heat_pulse_current_eeprom,temp16);

									MOSFETSource_set_Heat_Current(heat_pulse_current, slotNr);
									//DAC_Set((((uint32_t) heat_pulse_current) * 0xffff) / 1500, DAC_1, DAC_ADR_DAC_A);
								}
							}
							break;
						
						//2. Set Meas Current
						case _MK16('M','C'):						
														if (!ParseIntLn(&string[6],3,&temp16))
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

								if (measure_pulse_current != temp16)
								{
									measure_pulse_current = temp16;
									eeprom_write_word(&measure_pulse_current_eeprom,temp16);
								
									MOSFETSource_set_Meas_Current(measure_pulse_current, slotNr);
									//DAC_Set((((uint32_t) measure_pulse_current) * 0xffff) / 250, DAC_1, DAC_ADR_DAC_B);
								}
							
							}
							
							break;
							
						//3. Set Meas Voltage
						case _MK16('M','V'):
							if (!ParseIntLn(&string[6],4,&temp16))
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

								if (measure_pulse_voltage != temp16)
								{
									measure_pulse_voltage = temp16;
									eeprom_write_word(&measure_pulse_voltage_eeprom,temp16);
									
																						
									MOSFETSource_set_Meas_Voltage(temp16, slotNr);
									//DAC_Set((((uint32_t) measure_pulse_current) * 0xffff) / 250, DAC_1, DAC_ADR_DAC_B);
								}

																					
							}
																				
							break;
						//Default --> Fehler
						default:
							TransmitStringLn("COMMAND ERR");
							break;	
							
					}
					break;
					
					#pragma endregion SettingsMOSFET-Source
				
				//3. Boost-Heat current Source
				#pragma region SettingsPowerDiode-Source
				
				case 'H':
					TransmitStringLn("Function not realized yet!");			
					break;
					
				#pragma endregion SettingsPowerDiode-Source
								
				//4. Amplifier
				#pragma region SettingsFrontend
				
				case 'A':
					switch(cmd){
						
						//1. Set Heat Current
						case _MK16('W','O'):
						
						if (!ParseIntLn(&string[6],4,&temp16))
						{
							//no number
							TransmitStringLn("FORMAT ERR");
						}
						else if (temp16 > 10000 || temp16 < 0)
						{
							//new current limit not between 0 and 1.5A
							TransmitStringLn("NUMBER ERR");
						}
						else
						{
							TransmitString("SWO Amp=");
							TransmitInt(temp16, 1);
							TransmitStringLn(" mV");
							
							offset_voltage = temp16;
							eeprom_write_word(&offset_voltage_eeprom,temp16);


							if (offset_voltage != temp16)
							{
								offset_voltage = temp16;
								eeprom_write_word(&offset_voltage_eeprom,temp16);

								FrontEnd_set_Offset_Voltage(offset_voltage, slotNr);
							}
						}
						break;
					}						
					break;
					
					#pragma endregion SettingsFrontend
					
				//5. SlotTester
				#pragma region Setting_SlotTester
					
				case 'T':
					switch(cmd){
						
						//1. Set Heat Current
						case _MK16('S','T'):						
							if (string[5] != '\n')
							{
								//zuviele Zeichen
								TransmitStringLn("FORMAT ERR");
							}
							else
							{
								//Antrowrt
								TransmitString("SST Slot-Test: ");
								TransmitInt(slotNr, 1);
								TransmitStringLn("");
							
								//Test
								Slot_Tester_Init(slotNr);
								Slot_Tester_Gesamtablauf(slotNr);								
							}
							break;
					}
					break;
					
				#pragma endregion Setting_SlotTester
				
				//4. BreakDownVoltage Test
				#pragma region BreakDownCard
								
				case 'B':
				switch(cmd){
									
					//1. Initialization
					case _MK16('I','N'):
					if (string[5] != '\n')
					{
						//to many signs
						TransmitStringLn("FORMAT ERR");
					}
					else
					{
						MOSFET_BreakDown_Init(slotNr);
						
						//Answer
						TransmitString("SIN");
						TransmitInt(slotNr, 1);
						TransmitStringLn("B");									
					}
					break;
					
					//2. Relays for Breakdown
					case _MK16('R','B'):						
						if (string[6] == '1' && string[7] == '\n')
						{
							//Switch Relays
							B_Set_Relais_to_BreakDownTest(slotNr);

							//Answer
							TransmitString("SRB");
							TransmitInt(slotNr, 1);
							TransmitStringLn("B=1");
						}
						else if (string[6] == '0' && string[7] == '\n')
						{
							//Switch Relays
							B_Set_Relais_all_off(slotNr);

							//Answer
							TransmitString("SRB");
							TransmitInt(slotNr, 1);
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
						if (string[6] == '1' && string[7] == '\n')
						{
							//Switch Relays
							B_Set_Relais_to_Leakage_GS(slotNr);

							//Answer
							TransmitString("SRL");
							TransmitInt(slotNr, 1);
							TransmitStringLn("B=1");
						}
						else if (string[6] == '0' && string[7] == '\n')
						{
							//Switch Relays off
							B_Set_Relais_all_off(slotNr);

							//Answer
							TransmitString("SRL");
							TransmitInt(slotNr, 1);
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
						if (string[6] == '1' && string[7] == '\n')
						{
							//Switch Relays
							B_Set_Relais_to_Characteristic_Curve(slotNr);

							//Answer
							TransmitString("SRC");
							TransmitInt(slotNr, 1);
							TransmitStringLn("B=1");
						}
						else if (string[6] == '0' && string[7] == '\n')
						{
							//Switch Relays off
							B_Set_Relais_all_off(slotNr);

							//Answer
							TransmitString("SRC");
							TransmitInt(slotNr, 1);
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
						if (string[6] == '1' && string[7] == '\n')
						{
							//Switch Relays
							B_Set_Relais_to_BodyDiode_Curve(slotNr);

							//Answer
							TransmitString("SRD");
							TransmitInt(slotNr, 1);
							TransmitStringLn("B=1");
						}
						else if (string[6] == '0' && string[7] == '\n')
						{
							//Switch Relays off
							B_Set_Relais_all_off(slotNr);

							//Answer
							TransmitString("SRD");
							TransmitInt(slotNr, 1);
							TransmitStringLn("B=0");
						}
						else
						{
							//no number
							TransmitStringLn("FORMAT ERR");
						}
						break;
				}
				break;
								
				#pragma endregion BreakDown
				
				//Default --> Fehler
				default:
					TransmitStringLn("COMMAND ERR");
					break;
				
			}					
		}
		
		else{
			
			//Set Command is not known
			TransmitStringLn("COMMAND ERR");
		}
						
	}
	
	#pragma endregion Set
					
	//***************************************************************************
	//									GET
	//***************************************************************************
	
	#pragma region Get
		
	else if (string[0] == 'G' && string[3] == '\n')
	{
		switch (cmd)
		{
			//GEN, get enable status
			case _MK16('E','N'):
			
			TransmitString("GEN=");
			TransmitInt(current_source_enabled, 1);
			TransmitStringLn("");
			break;

			//GHC, get heat pulse current
			case _MK16('H','C'):
			
			TransmitString("GHC=");
			TransmitInt(heat_pulse_current, 1);
			TransmitStringLn(" mA");
			break;

			//GMC, get measure pulse current
			case _MK16('M','C'):
			
			TransmitString("GMC=");
			TransmitFloat(measure_pulse_current, 1, 1);
			TransmitStringLn(" mA");
			break;
			
			//GHV, get heat pulse voltage
			case _MK16('H','V'):
															
			TransmitString("GHV=");
			TransmitLong(heat_pulse_voltage,1);
			TransmitStringLn(" mV");
			break;
			
			//GMV, get heasure pulse voltage
			case _MK16('M','V'):
												
			TransmitString("GMV=");
			TransmitLong(measure_pulse_voltage,1);
			TransmitStringLn(" mV");
			break;

			//GHP, get heat pulse length
			case _MK16('H','P'):
			
			TransmitString("GHP=");
			TransmitLong(heat_pulse_length, 1);
			TransmitStringLn(" ms");
			break;

			//GMP, get measure pulse length
			case _MK16('M','P'):
			
			TransmitString("GMP=");
			TransmitInt(measure_pulse_length, 1);
			TransmitStringLn(" ms");
			break;

			//GFW, get firmware version
			case _MK16('F','W'):
			
			TransmitString("FW=");
			TransmitStringLn(FIRMWARE_VERSION);
			break;
			
			//GFW, get firmware version
			case _MK16('I','D'):
			
			TransmitString("ID=");
			TransmitStringLn("RthTEC TTA-Equipment V1_0");
			break;

			//GPA, get parameters
			case _MK16('P','A'):
			
			TransmitString("PARAMS=");

			TransmitInt(current_source_enabled, 1);

			TransmitString(",");
			TransmitInt(heat_pulse_current, 1);

			TransmitString(",");
			TransmitFloat(measure_pulse_current, 1, 1);

			TransmitString(",");
			TransmitLong(heat_pulse_length, 1);

			TransmitString(",");
			TransmitInt(measure_pulse_length, 1);

			TransmitStringLn("");
			break;

			default:
			TransmitStringLn("COMMAND ERR");
		}
	}
	
	#pragma endregion Get

	//***************************************************************************
	//								 ERROR
	//***************************************************************************
	else
	{
		TransmitStringLn("COMMAND ERR");
	}
	
}

//////////////////////////////////////////////////////////////////////////////////
//									Help - Functions							//
//////////////////////////////////////////////////////////////////////////////////

#pragma region Fuctions_Help

// -------------------------------------------------------------
// Zeile als Zahl lesen
// -------------------------------------------------------------

uint8_t ParseIntLn(char *string, uint8_t digits, int16_t *num)
{
    uint8_t neg = 0;
    uint8_t i = 0;
    uint8_t temp[6];

    *num = 0;

    //negative number
    if (*string == '-')
    {
        neg = 1;
        string++;
    }

	while ( (temp[i] = (*(string++) - '0')) < 10)
    {
        i++;
    }

    if (*(string-1) == '\n' && i <= digits)
    {
        for (uint8_t j = i;j > 0; j--)
        {
            *num *= 10;
            *num += temp[i-j];
        }

        if (neg)
        {
            *num = - *num;
        }

        return i;
    }

    return 0;
}

// -------------------------------------------------------------
// Zeile als Zahl lesen
// -------------------------------------------------------------

uint8_t ParseLongLn(char *string, uint8_t digits, int32_t *num)
{
    uint8_t neg = 0;
    uint8_t i = 0;
    uint8_t temp[11];

    *num = 0;

    //negative number
    if (*string == '-')
    {
        neg = 1;
        string++;
    }

	while ( (temp[i] = (*(string++) - '0')) < 10)
    {
        i++;
    }

    if (*(string-1) == '\n' && i <= digits)
    {
        for (uint8_t j = i;j > 0; j--)
        {
            *num *= 10;
            *num += temp[i-j];
        }

        if (neg)
        {
            *num = - *num;
        }

        return i;
    }

    return 0;
}

// -------------------------------------------------------------
// String als Boolean lesen
// -------------------------------------------------------------

uint8_t ParseBool(char *string, uint8_t *value)
{
    if (*string == '0')
    {
        *value = 0;

        return 1;
    }

    else if (*string == '1')
    {
        *value = 1;

        return 1;
    }

    return 0;
}

// -------------------------------------------------------------
// String als Byte lesen
// -------------------------------------------------------------

uint8_t ParseByte(char *string, uint8_t *value)
{
	uint8_t temp[10];
	uint8_t bit = 0;
	uint8_t i = 0;
	uint8_t j = 0;
	
	//Write in Tempo Array
	while ( (temp[i] = (*(string++) - '0')) < 10 && i < 9)
    {
        i++;
    }
	i--;
	
	if(i != 7)
	{
		return 0;
	}
	
	while (j <= i) 
	{
		if (temp[j]  == 0)
		{
			 bit = 0;
		}
		
		else if (temp[j]  == 1)
		{
			bit = 1;
		}
		else
		{
			return 0;
		}
		
		*value += bit << (i-j);
		j++;
	}
	
	return 1;
}

// -------------------------------------------------------------
// String auf Interface ausgeben
// -------------------------------------------------------------

void TransmitString(char *string)
{
    UART0TransmitString(string);
}

// -------------------------------------------------------------
// String & CRLF auf Interface ausgeben
// -------------------------------------------------------------

void TransmitStringLn(char *string)
{
    UART0TransmitStringLn(string);
}

// -------------------------------------------------------------
// Zahl dezimal auf Interface ausgeben (ASCII)
// -------------------------------------------------------------

void TransmitInt(int16_t i, uint8_t digits)
{
	char str[6], temp[6];

    // minus sign
	if(i < 0)
	{
		strcpy(temp,"-");
        i = -i;
	}
	else
	{
		strcpy(temp,"");
	}

	itoa(i,str,10);
	
	strcat(temp,str);
    strcpy(str,temp);


    //format with leading spaces
	if(digits > 0)
	{
		while(strlen(str) < digits)
		{
			strcpy(temp," ");
			strcat(temp,str);
			strcpy(str,temp);
		}
	}
    

    //send
	TransmitString(str);
}

// -------------------------------------------------------------
// Zahl dezimal auf Interface ausgeben (ASCII)
// -------------------------------------------------------------

void TransmitInt0(int16_t i, uint8_t digits)
{
	char str[6], temp[6];
    uint8_t neg = 0;

    // minus sign
	if(i < 0)
	{
        neg = 1;
        i = -i;

        if(digits) digits--;
	}

	itoa(i,str,10);


    //format with leading zeros
	if(digits)
	{
		while(strlen(str) < digits)
		{
			strcpy(temp,"0");
			strcat(temp,str);
			strcpy(str,temp);
		}
	}

    // minus sign
	if(neg)
	{
		strcpy(temp,"-");
		strcat(temp,str);
		strcpy(str,temp);
	}
    

    //send
	TransmitString(str);
}

// -------------------------------------------------------------
// Zahl dezimal auf Interface ausgeben (ASCII)
// -------------------------------------------------------------

void TransmitLong(int32_t i, uint8_t digits)
{
	char str[11], temp[11];

    // minus sign
	if(i < 0)
	{
		strcpy(temp,"-");
        i = -i;
	}
	else
	{
		strcpy(temp,"");
	}

	ltoa(i,str,10);
	
	strcat(temp,str);
    strcpy(str,temp);


    //format with leading spaces
	if(digits > 0)
	{
		while(strlen(str) < digits)
		{
			strcpy(temp," ");
			strcat(temp,str);
			strcpy(str,temp);
		}
	}
    

    //send
	TransmitString(str);
}

// -------------------------------------------------------------
// Zahl als Dezimalbruch auf Interface ausgeben (ASCII)
// -------------------------------------------------------------

void TransmitFloat(int16_t i, uint8_t digits, uint8_t div)
{
	char str[10], temp[10];
	uint16_t pot = 1;
	uint8_t co = div;

	while(div > 0)
	{
		pot *= 10;
		div--;
	}

	itoa(abs(i)%pot,str,10);

	while(strlen(str) < co)
	{
		strcpy(temp,"0");
		strcat(temp,str);
		strcpy(str,temp);
	}

	itoa(abs(i)/pot,temp,10);

	strcat(temp,".");
	strcat(temp,str);


    // minus sign
	if(i < 0)
	{
		strcpy(str,"-");
	}
	else
	{
		strcpy(str,"");
	}
	
	strcat(str,temp);


    //format with leading spaces
	if(digits > 0)
	{
		while(strlen(str) < digits)
		{
			strcpy(temp," ");
			strcat(temp,str);
			strcpy(str,temp);
		}
	}
    

    //send
	TransmitString(str);
}

// -------------------------------------------------------------
// Zahl als Dezimalbruch auf Interface ausgeben (ASCII)
// -------------------------------------------------------------

void TransmitByte(uint8_t byte)
{
	char str[] = "00000000";
	
	for(int i = 7; i >= 0; i--)
	{
		if(byte & (1 << i)){
			str[7-i] = '1';
		}
		else{
			str[7-i] = '0';		
		}
	}
	

	//send
	TransmitString(str);
}


#pragma endregion Fuctions_Help



#pragma GCC diagnostic pop