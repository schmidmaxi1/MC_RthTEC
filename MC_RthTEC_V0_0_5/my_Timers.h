/*
 * myTimers.h
 *
 * Created: 20.03.2019 12:48:13
 *  Author: schmidm
 */ 

#ifndef _MYTIMERS_H_
#define _MYTIMERS_H_



//*******************************************************************
//								Includes
//*******************************************************************

#include <stdio.h>


//*******************************************************************
//						Compiler-Constants
//*******************************************************************

#define CounterPin          PORTB,5		//Output Compare Pin A from Timer 1 

//*******************************************************************
//								Variables
//*******************************************************************

//Interrupt Counters
volatile uint8_t interrupt_1ms;
volatile uint8_t interrupt_10hz;
volatile uint8_t interrupt_1hz;




//*******************************************************************
//								Functions
//*******************************************************************

void Init_Timer_1ms();
void Init_Timer_100us();
void Init_Counter_100us();

void Start_Timer_100us();
void Stop_Timer_100us();

void Setup_Counter_for_stdTTA();
void Setup_Counter_for_DPA_TTA();
void Setup_Counter_for_DPA_TTA_HighLevel();


#endif