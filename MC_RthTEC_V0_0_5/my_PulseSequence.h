/*
 * my_Pulses.h
 *
 * Created: 23.03.2020 20:46:51
 *  Author: schmidm
 */ 


#ifndef MY_PULSESEQUENCES_H_
#define MY_PULSESEQUENCES_H_

//*******************************************************************
//								Includes
//*******************************************************************

#include <stdio.h>


//*******************************************************************
//								Variables
//*******************************************************************

//Flags
uint8_t flag_std_TTA;
uint8_t flag_DPA_TTA;
uint8_t flag_HPP_TTA;				//Heat Pre Pulse


//*******************************************************************
//								Functions
//*******************************************************************

//Pulses
void PulseStop();

void PulseStart_stdTTA();

void PulseStart_Sensitivity();			//edit: Maxi 07112017

void PulseStart_DPA_TTA();				//edit: Maxi 26.09.2018

void PulseStart_DPA_TTA_HighStart();	//edit: Maxi 12.9.2018
void PulseStart_DPA_TTA_fromHPP();

void PulseStart_PrePulse();				//edit: Maxi 27.09.2018


#endif /* MY_PULSES_H_ */