/*
 * MOSFET_BreakDown.h
 *
 * Created: 27.06.2019 16:01:51
 *  Author: schmidm
 */ 


/*
 ** Compiler-Constants
 */


/*
 ** Functions
 */

void MOSFET_BreakDown_Init(int slot_nr);


void B_Set_Relais_to_BreakDownTest(int slot_nr);
void B_Set_Relais_to_Leakage_GS(int slot_nr);
void B_Set_Relais_to_Characteristic_Curve(int slot_nr);
void B_Set_Relais_to_BodyDiode_Curve(int slot_nr);
void B_Set_Relais_all_off(int slot_nr);


void B_Set_DAC_Voltage(int voltage_in_mV, int slot_nr);

int B_Get_V_DS_in_mV(int slot_nr);
int B_Get_I_DS_in_mA(int slot_nr);
