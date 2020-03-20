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
void TransmitByte_Reverse(uint8_t byte);

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
									
				//3. Boost-Heat current Source				
				case 'H':
					TransmitStringLn("Function not realized yet!");			
					break;
													
				//4. Amplifier
				#pragma region SettingsFrontend
				
				case 'A':
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
								Amplifier_Init(slotNr);
							
								//Answer
								TransmitString("SIN");
								TransmitInt(slotNr, 1);
								TransmitStringLn("A");
							}
							break;
						
						//2. Window Gain (Fixed to 2)
						case _MK16('W','G'):
						
							if (!ParseIntLn(&string[6],4,&temp16))
							{
								//no number
								TransmitStringLn("FORMAT ERR");
							}
							else if (temp16 > 100 || temp16 < 1)
							{
								//Kp mit der versterkung
								TransmitStringLn("NUMBER ERR");
							}
							else
							{
								TransmitString("SWG");
								TransmitInt(slotNr, 1);
								TransmitString("A=");
								//TransmitInt(temp16, 1);
								TransmitInt(2, 1);
								TransmitStringLn(" W/O-Unit");

								if (amplifier_gain[slotNr-1] != temp16)
								{
									amplifier_gain[slotNr-1] = 2;
									eeprom_write_word(&parameter1_eeprom[slotNr-1], 2);
									Amplifier_Set_Gain(amplifier_gain[slotNr-1], slotNr);
								}
							}
							break;
						//3. Set Window Offset Voltage
						case _MK16('W','O'):
						
							if (!ParseIntLn(&string[6],4,&temp16))
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
								TransmitInt(slotNr, 1);
								TransmitString("A=");
								TransmitFloat(temp16,1, 3);
								TransmitStringLn(" V");

								if (amplifier_offset_voltage_mV[slotNr-1] != temp16)
								{
									amplifier_offset_voltage_mV[slotNr-1] = temp16;
									eeprom_write_word(&parameter2_eeprom[slotNr-1], temp16);
									Amplifier_Set_Offset_Voltage(amplifier_offset_voltage_mV[slotNr-1], slotNr);
								}
							}
							break;
							
						//Default --> Fehler
						default:
							TransmitStringLn("COMMAND ERR");
							break;
					}
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
				
				//6. BreakDownVoltage Test								
				case 'B':
					Terminal_SET_BreakDown(string);
					break;
												
				//7. Heller Frontend (F)								
				case 'F':
					Terminal_SET_FrontEnd(string);
					break;
												
				//Default --> Fehler
				default:
					TransmitStringLn("COMMAND ERR");
					break;				
			}
			
		#pragma endregion Special_Set_Commands		
					
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
			
			
			
				//GHV, get heat pulse voltage
				case _MK16('H','V'):
															
					TransmitString("GHV=");
					TransmitLong(heat_pulse_voltage,1);
					TransmitStringLn(" mV");
					break;
			
				//GMV, get measure pulse voltage
				case _MK16('M','V'):
												
					TransmitString("GMV=");
					TransmitLong(measure_pulse_voltage,1);
					TransmitStringLn(" mV");
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
								
				#pragma endregion SettingsMOSFET-Source
					
				//4.3 Amplifier
				#pragma region Amplifier
				
				case 'A':
					switch(cmd){
					
						//2. Gain
						case _MK16('W','G'):
							TransmitString("GWG");
							TransmitInt(slotNr, 1);
							TransmitString("A=");
							TransmitInt(amplifier_gain[slotNr-1], 1);
							TransmitStringLn(" W/O-Unit");
							break;
					
						//3. Set Meas Current
						case _MK16('W','O'):
							TransmitString("GWO");
							TransmitInt(slotNr, 1);
							TransmitString("A=");
							TransmitFloat(amplifier_offset_voltage_mV[slotNr-1], 1, 3);
							TransmitStringLn(" V");
							break;
					
						//Default --> Fehler
						default:
							TransmitStringLn("COMMAND ERR");
							break;
					
					}
				break;
				
				#pragma endregion Amplifier
								
				
				//6. BreakDown (B)
				case 'B':
					Terminal_GET_BreakDown(string);
					break;
				
				//7. Heller FrontEnd (F)	
				case 'F':
					Terminal_GET_FrontEnd(string);
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
	uint8_t temp = 0;
	for(int i = 0; i < 8 ; i++)
	{
		if(string[i] - '0' == 0)
		{
			//Nix zu tun
		}
		else if(string[i] - '0' == 1)
		{
			temp = temp + (1<<i);
		}
		else
		{
			return 0;
		}
	}
	
	*value = temp;
	return 1;
	
}

/*
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
*/

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

void TransmitByte_Reverse(uint8_t byte)
{
	char str[] = "00000000";
	
	for(int i = 0; i < 8; i++)
	{
		if(byte & (1 << i)){
			str[i] = '1';
		}
		else{
			str[i] = '0';
		}
	}
	

	//send
	TransmitString(str);
}


#pragma endregion Fuctions_Help



#pragma GCC diagnostic pop