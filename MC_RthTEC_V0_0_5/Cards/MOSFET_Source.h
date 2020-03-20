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
uint16_t mosfet_Source_Meas_Voltage_10mV[8];

//Heat-Voltage is adjusted by an external voltage-Source

/*
 ** Functions
 */

void MOSFET_Source_Init(int slot_nr);
void MOSFET_Source_Default_Values(int slot_nr);

void MOSFET_Source_Variables_from_EEPROM(int slot_nr);

void MOSFET_Source_Set_Heat_Current(uint16_t current_mA, int slot_nr);
void MOSFET_Source_Set_Meas_Current(uint16_t current_10th_mA, int slot_nr);
void MOSFET_Source_Set_Meas_Voltage(uint16_t voltage_10mV, int slot_nr);

void MOSFET_Source_sample_Heat(int slot_nr);
uint16_t MOSFET_Source_sample_Meas_receive_Heat(int slot_nr);
uint16_t MOSFET_Source_receive_Meas(int slot_nr);

int MOSFET_Source_Measure_Heat_Voltage_in_10mV();
int MOSFET_Source_Measure_Meas_Voltage_in_10mV();

void Terminal_SET_MOSFET_Source(char *myMessage);
void Terminal_GET_MOSFET_Source(char *myMessage);