/*
 * myTimers.h
 *
 * Created: 20.03.2019 12:48:13
 *  Author: schmidm
 */ 


/*
 ** Includes
 */

//No needed

/*
 ** Compiler-Constants
 */

//Flags
uint8_t flag_std_TTA;
uint8_t flag_DPA_TTA;
uint8_t flag_HPP_TTA;	//Heat Pre Pulse

/*
 ** Compiler-Constants
 */

#define CounterPin          PORTB,5		//Output Compare Pin A from Timer 1 

/*
 ** Functions
 */


void Init_Timer_1ms();

void Init_Timer_100us();
void Start_Timer_100us();
void Stop_Timer_100us();

void Init_Counter_100us();
void Setup_Counter_for_stdTTA();
void Setup_Counter_for_DPA_TTA();
void Setup_Counter_for_DPA_TTA_HighLevel();