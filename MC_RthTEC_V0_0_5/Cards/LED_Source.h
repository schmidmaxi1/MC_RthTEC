/*
 * LED_Source.h
 *
 * Created: 04.07.2019 15:37:14
 *  Author: schmidm
 */ 


/*
 ** Compiler-Constants
 */


/*
 ** Functions
 */

void LED_Source_Init(int slot_nr);

void LED_Source_Set_Heat_Current(uint16_t current_mA, int slot_nr);
void LED_Source_Set_Meas_Current(uint16_t current_10th_mA, int slot_nr);