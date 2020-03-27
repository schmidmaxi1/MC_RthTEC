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



//*******************************************************************
//								Includes
//*******************************************************************

#include "main.h"			//Main und Terminal nutzen gleichen Header

#include "Config.h" 
#include "helper.h"
#include "globalVAR.h"
#include "Serial_ReadWrite.h"

#include "my_PulseSequence.h"

#include <string.h>
#include <avr/wdt.h>

#include "Cards/All_Cards.h"



//*******************************************************************
//						Variables (local)
//*******************************************************************

uint8_t len;
char *string;



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
		
		#pragma region General_Set_Commands
		
		if(string[3] == ' '){
			
			//Check the second two characters
			switch (cmd){
				
				//0.2 Info************************************************************
				#pragma region 0.2 Info
				
				//SHW, get hardware version (one one times possible)
				case _MK16('H','V'):
					//Check for Format: V[x].[x]
					if (string[4] == 'V' && string[6] == '.' && string[8] == '\n')
					{
						//Read current setting
						char temp_array[4];
						temp_array[4] = '\0';	//no clue why to force this. Else string is longer
						eeprom_read_block(temp_array, &hardware_version_eeprom ,4);
						
						//Check if HW Version already written
						if (temp_array[0] == 'V' && temp_array[2] == '.')
						{
							TransmitString("SHV=");
							TransmitString(temp_array);
							TransmitStringLn(" can't be overwritten");
						}
						else
						{
							//Get substring form message
							memcpy(temp_array, &string[4], 4);
							
							//Write to EEPROM
							eeprom_write_block(temp_array, &hardware_version_eeprom, 4);
							
							//Answer
							TransmitString("SHV=");
							TransmitString(temp_array);
							TransmitStringLn("");						
						}						
					}
					else
					{
						//no number
						TransmitStringLn("FORMAT ERR");
					}
					break;
				
				
				//SMD, set manufacturing date (one one times possible)
				case _MK16('M','D'):
					//Check for Format: CW[WW]-[YYYY]
					if (string[4] == 'C' && string[5] == 'W' && string[8] == '-' && string[13] == '\n')
					{
						//Read current setting
						char temp_array[9];
						temp_array[9] = '\0';	//no clue why to force this. Else string is longer
						eeprom_read_block(temp_array, &manufacturind_date_eeprom ,9);
					
						//Check if HW Version already written
						if (temp_array[0] == 'C' && temp_array[1] == 'W' && temp_array[4] == '-')
						{
							TransmitString("SMD=");
							TransmitString(temp_array);
							TransmitStringLn(" can't be overwritten");
						}
						else
						{
							//Get substring form message
							memcpy(temp_array, &string[4], 9);
						
							//Write to EEPROM
							eeprom_write_block(temp_array, &manufacturind_date_eeprom ,9);
						
							//Answer
							TransmitString("SMD=");
							TransmitString(temp_array);
							TransmitStringLn("");
						}
					}
					else
					{
						//no number
						TransmitStringLn("FORMAT ERR");
					}
					break;				
								
				#pragma endregion 0.2 Info
			
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
						PulseStart_DPA_TTA();
						//PulseStart_DPA_TTA_HighStart();
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
						TransmitStringLn(" pulses");

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
				
					//!!!Reihenfolge wird getauscht!!!
					if(strlen(string) != 13)
					{
						//Not 8 characters
						TransmitStringLn("FORMAT ERR");
					}	
					else if (!ParseByte(&string[4], &temp8))
					{
						//Conversion did not work
						TransmitStringLn("FORMAT ERR");
					}
					else
					{
						TransmitString("SPR=");
						TransmitByte_Reverse(temp8);
						TransmitStringLn("");

						pulse_output_register = temp8;
						eeprom_write_byte(&pulse_output_register_eeprom,temp8);
					}
					break;
								
				#pragma endregion 3.Puls-settings
				
				//4.0 Set Card types***************************************************
				#pragma region 4.SlotSettings
				case _MK16('C','T'):
				
					if(strlen(string) != 13)
					{
						//Not 8 characters
						TransmitStringLn("FORMAT ERR");
					}					
					else
					{
						//Save old setting temporary
						char old_card_Type[8];
						strcpy(old_card_Type, card_Type);
						
						//Get the substring and write it to card_Type and EEPROM
						memcpy(card_Type, &string[4], 8);
						eeprom_write_block(card_Type, &card_Type_register_eeprom, 8);
						
						//If there were changes --> Card initialization
						Init_All_Cards(card_Type, old_card_Type);
							
						//Answer
						TransmitString("SCT=");
						TransmitString(card_Type);
						TransmitStringLn("");
					}
				
					break;
					
				#pragma endregion 4.SlotSettings	
														
				//Default --> Fehler***************************************************	
				default:
					TransmitStringLn("COMMAND ERR");
					break;				
			}						
		}
		
		#pragma endregion General_Set_Commands
		
		//*****************************************************************************
		//Set-Commands, that apply ONE 19''-Cards (Definition: Number @ 3 character)
		//*****************************************************************************
		
		#pragma region Special_Set_Commands
						
		else if (slotNr >= 0 && slotNr < 8){
			
			//*****************************************************************************
			//Differentiation between card-Types
			//*****************************************************************************
			
			switch(string[4]){
				
				//4.1 LED-Heat-Meas current Source				
				case 'L':	
					Terminal_SET_LED_Source(string);
					break;
									
				//4.2 MOSFET-Heat-Meas current Source				
				case 'M':
					Terminal_SET_MOSFET_Source(string);
					break;
				
				//4.3.1 Heller Front-End (F)
				case 'F':
					Terminal_SET_FrontEnd(string);
					break;				
													
				//4.3.2 Amplifier			
				case 'A':
					Terminal_SET_Amplifier(string);
					break;
					
				//4.4 BreakDownVoltage Test
				case 'B':
					Terminal_SET_BreakDown(string);
					break;
				
				//4.99 SlotTester
				case 'T':
					Terminal_SET_SlotTester(string);
					break;
																	
				//Default --> Fehler
				default:
					TransmitStringLn("COMMAND ERR");
					break;				
			}
								
		}
		
		#pragma endregion Special_Set_Commands	
		
		//*****************************************************************************
		//Error
		//*****************************************************************************		
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
			
	else if (string[0] == 'G')
	{
		//Must executed here, later no chance, before not safe
		slotNr = string[3] - '0';
		
		//*****************************************************************************
		//Get-Commands, that apply for all 19''-Cards (Definition: Space @ 3 character)
		//*****************************************************************************
		
		#pragma region Gerneral_Get
		
		if(string[3] == '\n'){
		
			switch (cmd)
			{
				//0.2 Info........................................................................
				//GFW, get firmware version
				case _MK16('I','D'):
					TransmitString("GID=");
					TransmitStringLn("RthTEC TTA-Equipment V1_0");
					break;
					
				//GFW, get firmware version
				case _MK16('S','V'):
					TransmitString("GSV=");
					TransmitStringLn(SOFTWARE_VERSION);
					break;
					
				//GFW, get firmware version
				case _MK16('H','V'):;
					char temp_array[4];
					temp_array[4] = '\0';	//no clue why to force this. Else string is longer
					eeprom_read_block(temp_array, &hardware_version_eeprom ,4);					
					TransmitString("GHV=");
					TransmitStringLn(temp_array);
					break;
					
				//GFW, get firmware version
				case _MK16('M','D'):;
					char temp_array2[9];
					temp_array2[9] = '\0';	//no clue why to force this. Else string is longer
					eeprom_read_block(temp_array2, &manufacturind_date_eeprom ,9);
					TransmitString("GMD=");
					TransmitStringLn(temp_array2);
					break;
								
								
				//1.1 GEN, get enable status......................................................
				case _MK16('E','N'):			
					TransmitString("GEN=");
					TransmitInt(current_source_enabled, 1);
					TransmitStringLn("");
					break;
				
				//3.1 GHP, get heat pulse length.................................................
				case _MK16('H','P'):							
					TransmitString("GHP=");
					TransmitLong(heat_pulse_length, 1);
					TransmitStringLn(" ms");
					break;

				//3.2 GMP, get measure pulse length
				case _MK16('M','P'):							
					TransmitString("GMP=");
					TransmitInt(measure_pulse_length, 1);
					TransmitStringLn(" ms");
					break;
				
				//3.3 GND, get deterministic pulse count
				case _MK16('N','D'):			
					TransmitString("GND=");
					TransmitInt(deterministic_pulse_cycles, 1);
					TransmitStringLn(" pulses");
					break;
			
				//3.4 GTD, get deterministic pulse length
				case _MK16('T','D'):
					TransmitString("GTD=");
					TransmitFloat(deterministic_pulse_length, 1, 1);
					TransmitStringLn(" ms");
					break;
				
				//3.5, get Slot pulse info
				case _MK16('P','R'):
					TransmitString("GPR=");
					TransmitByte_Reverse(pulse_output_register);
					TransmitStringLn("");
					break;
				
				//4.1, get Slot Card Type
				case _MK16('C','T'):
					TransmitString("GCT=");
					TransmitString(card_Type);
					TransmitStringLn("");
					break;
							



				default:
				TransmitStringLn("COMMAND ERR");
			}
		}
		
		#pragma endregion Gerneral_Get
		
		//*****************************************************************************
		//Get-Commands, that apply ONE 19''-Cards (Definition: Number @ 3 character)
		//*****************************************************************************
		
		#pragma region Special_Get
		
		else if (slotNr >= 0 && slotNr < 8){
						
			switch(string[4]){
				
				//4.1 LED-Source				
				case 'L':
					Terminal_GET_LED_Source(string);
					break;
								
				//4.2 MOSFET heat-meas current source								
				case 'M':
					Terminal_GET_MOSFET_Source(string);
					break;	
						
				//4.3.1 Heller FrontEnd (F)
				case 'F':
					Terminal_GET_FrontEnd(string);
					break;					
					
				//4.3.2 Amplifier
				case 'A':
					Terminal_GET_Amplifier(string);
					break;
															
				//4.4 BreakDown (B)
				case 'B':
					Terminal_GET_BreakDown(string);
					break;
					
				//4.99 Slot-Tester
				case 'T':
					Terminal_GET_SlotTester(string);
					break;
							
				//Default --> Fehler
				default:
					TransmitStringLn("COMMAND ERR");
					break;
			}
		}
		
		#pragma endregion Special_Get
		
		//*****************************************************************************
		//Error
		//*****************************************************************************
		
		else{
			//Set Command is not known
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





#pragma GCC diagnostic pop