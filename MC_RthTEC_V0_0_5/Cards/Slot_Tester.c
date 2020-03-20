/*
 * Slot_Tester.c
 *
 * Created: 30.04.2019 12:31:12
 *  Author: schmidm
 */ 

/*
 ** Includes
 */

#include "../Config.h" //Doppelpunkte um einen Ordner zurück zu gehen

#include "../main.h"	//Doppelpunkte um einen Ordner zurück zu gehen
#include <util/delay.h>
#include "../helper.h"

#include "Slot_Tester.h"

#include "../ICs/MCP23S08.h"


/*
 ** Compiler-Constants
 */

/*
 ** Variables
 */

/*
 ** Functions
 */

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
}

void Slot_Tester_Gesamtablauf(int slot_nr)
{
	//Nummer in 7 Segment schreiben (SPI In test)
	Slot_Tester_set_7Segment(slot_nr);
	//Nummer in 7 Segment lesen (SPI Out test)
	if(Slot_Tester_get_7Segment(slot_nr) == slot_nr)
	{
		//Lauflicht starten
		Slot_Tester_IO_Lauflicht(slot_nr);
	}
	
}
	
void Slot_Tester_set_7Segment(int slot_nr)
{
	//CS wieder auf 1, (sonst auf 0) und kurz warten
	_set_bit(IO_PORT6, slot_nr - 1);
	_delay_ms(1);
	
	//nr. in 7-Segment
	uint8_t value = 0xFF;
	switch (slot_nr)
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
	
void Slot_Tester_IO_Lauflicht(int slot_nr)
{
	//Alle Ouptup LEDs werden nacheinander angesporchen
	//Um zu verhindern das man in Watchdog läuft wird der Counter jedesmal auf 0 gesetzt	
	_set_bit(HP_Port, slot_nr - 1);
	_delay_ms(500);
	wdt_reset();	
	_clear_bit(HP_Port, slot_nr - 1);
	_set_bit(MP_Port, slot_nr - 1);
	_delay_ms(500);	
	wdt_reset();
	_set_bit(IO_PORT3, slot_nr - 1);
	_clear_bit(MP_Port, slot_nr - 1);
	_delay_ms(500);
	wdt_reset();
	_set_bit(IO_PORT4, slot_nr - 1);
	_clear_bit(IO_PORT3, slot_nr - 1);
	_delay_ms(500);
	wdt_reset();
	_set_bit(IO_PORT5, slot_nr - 1);
	_clear_bit(IO_PORT4, slot_nr - 1);
	_delay_ms(500);
	wdt_reset();
	_set_bit(IO_PORT6, slot_nr - 1);
	_clear_bit(IO_PORT5, slot_nr - 1);
	_delay_ms(500);
	wdt_reset();
	_clear_bit(IO_PORT6, slot_nr - 1);	
		
}
