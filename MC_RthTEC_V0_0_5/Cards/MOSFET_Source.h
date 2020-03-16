/*
 * MOSFET_Source.h
 *
 * Created: 09.03.2020 09:39:43
 *  Author: schmidm
 */ 


/*
 ** Compiler-Constants
 */

#define mosfet_source_Heat_current_default		500;		//500  mA
#define mosfet_source_Meas_current_default		200;		//20.0 mA
#define mosfet_source_Meas_voltage_default		100;		//10.0 V

/*
 ** Variablen
 */

uint16_t mosfet_Source_Heat_Current_mA[8];
uint16_t mosfet_Source_Meas_Current_0mA1[8];
uint16_t mosfet_Source_Meas_Voltage_0V1[8];

//Heat-Voltage is adjusted by an external voltage-Source

/*
 ** Functions
 */

void MOSFET_Source_Init(int slot_nr);
void MOSFET_Source_Default_Values(int slot_nr);

void MOSFET_Source_Variables_from_EEPROM(int slot_nr);

void MOSFET_Source_Set_Heat_Current(uint16_t current_mA, int slot_nr);
void MOSFET_Source_Set_Meas_Current(uint16_t current_10th_mA, int slot_nr);
void MOSFET_Source_Set_Meas_Voltage(uint16_t current_10th_V, int slot_nr);