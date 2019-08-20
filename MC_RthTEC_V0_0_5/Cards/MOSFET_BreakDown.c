/*
 * MOSFET_BreakDown.c
 *
 * Created: 27.06.2019 16:01:29
 *  Author: schmidm
 */ 

/*
 ** Includes
 */

#include "../main.h"	//Doppelpunkte um einen Ordner zurück zu gehen
#include <util/delay.h>
#include "../helper.h"

#include "MOSFET_BreakDown.h"

#include "../ICs/MCP23S08.h"

/*
PinBelegung:
1. HP	(N.C.)
2. MP	(N.C.)
3. /ChipSelect DAC
4. /ChipSelect ADC-->U_DS
5. /ChipSelect ADC-->I_DS
6. /ChipSelect IO-Expander

IO0: Relay0: DS --> COM
IO1: Relay1: DS --> GND
IO2: Relay2: DS --> SMU
IO3: Relay3: GS --> SMU
IO4: Relay4: GS --> DAC
IO5: Relay5: GS --> GND
IO6: N.C.
IO7: N.C.

*/


void MOSFET_BreakDown_Init(int slot_nr)
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
	
	//Register in MCP23S08:
	//All as Outputs
	//0 means OUT, 1 means IN
	IO_Expander_set_Register(register_IODIR, 0x00, &IO_PORT6, slot_nr-1);

	//All to LOW (Relais off)
	IO_Expander_set_Register(register_OLAT, 0x00, &IO_PORT6, slot_nr-1);

}

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
