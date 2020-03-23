/*
 * Slot_Tester.c
 *
 * Created: 30.04.2019 12:31:12
 *  Author: schmidm
 */ 

//*******************************************************************
//								Includes
//*******************************************************************

#include "Slot_Tester.h"

#include "../Config.h" //Doppelpunkte um einen Ordner zurück zu gehen
#include "../helper.h"
#include "../globalVAR.h"
#include "../Serial_ReadWrite.h"

#include <util/delay.h>
#include <avr/wdt.h>

#include "../ICs/MCP23S08.h"

//*******************************************************************
//								Variables
//*******************************************************************

uint16_t slot_Tester_Display_number[8];

//*******************************************************************
//								Explanation
//*******************************************************************

/*
PinBelegung:
1. LED
2. LED
3. LED
4. LED
5. LED
6. LED + CS IO-Expander

EEPROM:
Parameter1 = Number in display
Parameter2 = not used
Parameter3 = not used

Don't forget to add a new Card to the functions:
-Init_All_Cards(char newCard_Type[], char oldCard_Type[])
-EEPROM_last_Values()

*/


//*******************************************************************
//						 Init & EEPROM
//*******************************************************************

void Slot_Tester_Init(int slot_nr)
{
	//Set all to 0
	_clear_bit(HP_Port, slot_nr - 1);
	_clear_bit(MP_Port, slot_nr - 1);
	_clear_bit(IO_PORT3, slot_nr - 1);
	_clear_bit(IO_PORT4, slot_nr - 1);
	_clear_bit(IO_PORT5, slot_nr - 1);
	_clear_bit(IO_PORT6, slot_nr - 1);
	//Set all as Output
	_set_out(HP_Port, slot_nr - 1);
	_set_out(MP_Port, slot_nr - 1);
	_set_out(IO_PORT3, slot_nr - 1);
	_set_out(IO_PORT4, slot_nr - 1);
	_set_out(IO_PORT5, slot_nr - 1);
	_set_out(IO_PORT6, slot_nr - 1);
	
	//Register in MCP23S08:
	//CS wieder auf 1, (sonst auf 0) und kurz warten
	_set_bit(IO_PORT6, slot_nr - 1);
	_delay_ms(1);
	//All as Outputs 
	IO_Expander_set_Register(register_IODIR, 0x00, &IO_PORT6, slot_nr-1);

	//All to high (LEDs off)
	IO_Expander_set_Register(register_OLAT, 0xFF, &IO_PORT6, slot_nr-1);
	//CS wieder auf 0
	_clear_bit(IO_PORT6, slot_nr - 1);
	
	//Display setzen
	_delay_ms(1);
	Slot_Tester_set_7Segment(slot_nr, slot_Tester_Display_number[slot_nr-1]);
}

void Slot_Tester_Variables_from_EEPROM(int slot_nr)
{
	slot_Tester_Display_number[slot_nr-1] = eeprom_read_word(&parameter1_eeprom[slot_nr-1]);
}

void Slot_Tester_Default_Values(int slot_nr)
{
	//Set default values (Local & EEPROM)
	slot_Tester_Display_number[slot_nr-1] = slot_nr;
		
	eeprom_write_word(&parameter1_eeprom[slot_nr-1], slot_Tester_Display_number[slot_nr-1]);
}


//*******************************************************************
//						 Setting - FCTs
//*******************************************************************					

void Slot_Tester_Gesamtablauf(int slot_nr)
{
	//Alle LEDs nacheinander durchschalten und 7-Segment von 1 bis 6 zählen
	
	//Nummer in 7 Segment schreiben (SPI In test)
	int old_number = Slot_Tester_get_7Segment(slot_nr);
		
	//Alle Ouptup LEDs werden nacheinander angesporchen
	//Um zu verhindern das man in Watchdog läuft wird der Counter jedesmal auf 0 gesetzt
	_set_bit(HP_Port, slot_nr - 1);
	Slot_Tester_set_7Segment(slot_nr, 1);
	_delay_ms(500);
	wdt_reset();
	_clear_bit(HP_Port, slot_nr - 1);
	_set_bit(MP_Port, slot_nr - 1);
	Slot_Tester_set_7Segment(slot_nr, 2);
	_delay_ms(500);
	wdt_reset();
	_set_bit(IO_PORT3, slot_nr - 1);
	_clear_bit(MP_Port, slot_nr - 1);
	Slot_Tester_set_7Segment(slot_nr, 3);
	_delay_ms(500);
	wdt_reset();
	_set_bit(IO_PORT4, slot_nr - 1);
	_clear_bit(IO_PORT3, slot_nr - 1);
	Slot_Tester_set_7Segment(slot_nr, 4);
	_delay_ms(500);
	wdt_reset();
	_set_bit(IO_PORT5, slot_nr - 1);
	_clear_bit(IO_PORT4, slot_nr - 1);
	Slot_Tester_set_7Segment(slot_nr, 5);
	_delay_ms(500);
	wdt_reset();
	_set_bit(IO_PORT6, slot_nr - 1);
	_clear_bit(IO_PORT5, slot_nr - 1);
	Slot_Tester_set_7Segment(slot_nr, 6);
	_delay_ms(500);
	wdt_reset();
	_clear_bit(IO_PORT6, slot_nr - 1);
	Slot_Tester_set_7Segment(slot_nr, old_number);
		
}
	
void Slot_Tester_set_7Segment(int slot_nr, int number)
{
	//CS wieder auf 1, (sonst auf 0) und kurz warten
	_set_bit(IO_PORT6, slot_nr - 1);
	_delay_ms(1);
	
	//nr. in 7-Segment
	uint8_t value = 0xFF;
	switch (number)
	{
		case 0:
			value = Segment_0;
			break;
		case 1:
			value = Segment_1;
			break;
		case 2:
			value = Segment_2;
			break;
		case 3:
			value = Segment_3;
			break;
		case 4:
			value = Segment_4;
			break;
		case 5:
			value = Segment_5;
			break;
		case 6:
			value = Segment_6;
			break;
		case 7:
		value = Segment_7;
		break;
		case 8:
			value = Segment_8;
			break;
		case 9:
			value = Segment_9;
			break;
		default:
			value = 0xFF;
			break;		
	}
		
	//Switch LEDs according to number (0 is ON)
	IO_Expander_set_Register(register_OLAT, value, &IO_PORT6, slot_nr - 1);
	
	//CS wieder auf 0
	_delay_ms(1);
	IO_Expander_set_Register(register_IODIR, 0x00, &IO_PORT6, slot_nr - 1);
	_delay_ms(1);
	_clear_bit(IO_PORT6, slot_nr - 1);
	
}


//*******************************************************************
//						 Getting - FCTs
//*******************************************************************

int Slot_Tester_get_7Segment(int slot_nr)
{
		//CS wieder auf 1, (sonst auf 0) und kurz warten
		_set_bit(IO_PORT6, slot_nr - 1);
		_delay_ms(1);

		uint8_t local = IO_Expander_get_Register(register_OLAT, &IO_PORT6, slot_nr - 1);
		int value = -1;
		
		switch (local)
		{
			case Segment_0:
				value = 0;
				break;
			case Segment_1:
				value = 1;
				break;
			case Segment_2:
				value = 2;
				break;
			case Segment_3:
				value = 3;
				break;
			case Segment_4:
				value = 4;
				break;
			case Segment_5:
				value = 5;
				break;
			case Segment_6:
				value = 6;
				break;
			case Segment_7:
			value = 7;
			break;
			case Segment_8:
				value = 8;
				break;
			case Segment_9:
				value = 9;
				break;
			default:
				value = -1;
				break;		
		}
			
		//CS wieder auf 0
		_delay_ms(1);
		_clear_bit(IO_PORT6, slot_nr - 1);
		
		return value;	
}
	

//*******************************************************************
//							  Terminal
//*******************************************************************

void Terminal_SET_SlotTester(char *myMessage)
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
		//4.99.1 Init
		case  _MK16('I', 'N'):
			if (myMessage[5] != '\n')
			{
				//to many signs
				TransmitStringLn("FORMAT ERR");
			}
			else
			{
				//Overwrite old settings
				card_Type[mySlotNr-1] = 'T';
				eeprom_write_block(card_Type, &card_Type_register_eeprom, 8);
				
				//Take default values
				Slot_Tester_Default_Values(mySlotNr);
				//Init
				Slot_Tester_Init(mySlotNr);
				
				//Answer
				TransmitString("SIN");
				TransmitInt(mySlotNr, 1);
				TransmitStringLn("T");
			}
			break;

		//4.99.2 Set digital number
		case  _MK16('D', 'N'):
			if (!ParseIntLn(&myMessage[6],4,&temp16))
			{
				//no number
				TransmitStringLn("FORMAT ERR");
			}
			else if (temp16 > 9 || temp16 < 0)
			{
				//0 to 9
				TransmitStringLn("NUMBER ERR");
			}
			else
			{	
				//Change display
				slot_Tester_Display_number[mySlotNr-1] = temp16;
				eeprom_write_word(&parameter1_eeprom[mySlotNr-1], temp16);
				Slot_Tester_set_7Segment(mySlotNr, temp16);
						
				//Answer
				TransmitString("SDN");
				TransmitInt(mySlotNr, 1);
				TransmitString("T = ");
				TransmitInt(temp16, 1);
				TransmitStringLn("");
				
			}
			break;		
			
		//4.99.3 Start test run (blinking)
		case _MK16('S','T'):
			if (myMessage[5] != '\n')
			{
				//To many Chars
				TransmitStringLn("FORMAT ERR");
			}
			else
			{				
				//Test
				Slot_Tester_Gesamtablauf(mySlotNr);
				
				//Answer
				TransmitString("Slot-Test finished: ");
				TransmitInt(mySlotNr, 1);
				TransmitStringLn("");
									

			}
			break;
			
		//Default --> Fehler
		default:
			TransmitStringLn("COMMAND ERR");
			break;
	
	}


}

void Terminal_GET_SlotTester(char *myMessage)
{
	//myMessage:	whole message
	//myCMD:		2 indicator Chars (Position 1 and 2)
	//mySlotNr:		Slot Number (Position 3)
	
	//Local variables
	uint16_t myCMD = _MK16(myMessage[1],myMessage[2]);
	int8_t mySlotNr = myMessage[3] - '0';

	switch(myCMD){
		
		//99.3.2 Offset Voltage
		case _MK16('D','N'):
			TransmitString("GDN");
			TransmitInt(mySlotNr , 1);
			TransmitString("T=");
			TransmitInt(slot_Tester_Display_number[mySlotNr - 1], 1);
			TransmitStringLn("");
			break;
			
		//Default --> Fehler
		default:
			TransmitStringLn("COMMAND ERR");
			break;
		
	}

}