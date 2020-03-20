/*
 * LED_Source.h
 *
 * Created: 04.07.2019 15:37:14
 *  Author: schmidm
 */ 


/*
 ** Compiler-Constants
 */

#define led_source_Heat_current_default		500;		//500mA
#define led_source_Meas_current_default		200;		//20.0mA


/*
 ** Variablen
 */

uint16_t led_Source_Heat_Current_mA[8];
uint16_t led_Source_Meas_Current_0mA1[8];


/*
 ** Functions
 */

void LED_Source_Init(int slot_nr);
void LED_Source_Default_Values(int slot_nr);
void LED_Source_Variables_from_EEPROM(int slot_nr);

void LED_Source_Set_Heat_Current(uint16_t current_mA, int slot_nr);
void LED_Source_Set_Meas_Current(uint16_t current_10th_mA, int slot_nr);

void Terminal_SET_LED_Source(char *myMessage);
void Terminal_GET_LED_Source(char *myMessage);