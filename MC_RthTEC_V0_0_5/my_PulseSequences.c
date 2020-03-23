/*
 * my_Pulses.c
 *
 * Created: 23.03.2020 20:47:16
 *  Author: schmidm
 */ 


//*******************************************************************
//							Includes
//*******************************************************************

#include "my_PulseSequence.h"

#include "Config.h"
#include "helper.h"
#include "globalVAR.h"

#include <util/delay.h>

#include "my_Timers.h"

//*******************************************************************
//							Variables
//*******************************************************************

//Flags
uint8_t flag_std_TTA;
uint8_t flag_DPA_TTA;
uint8_t flag_HPP_TTA;				//Heat Pre Pulse


//*******************************************************************
//							Functions
//*******************************************************************

// -------------------------------------------------------------
// Stop all Pulses									  (complete)
// -------------------------------------------------------------

void PulseStop()
{
	//Timer 100us stop
	Stop_Timer_100us();

	//Clear all Pulses
	MP_Port = 0;
	HP_Port = 0;
	
	//Status LED for Pulses
	clear_bit(LED_Pulse);
}

// -------------------------------------------------------------
// stdTTA-Pulse start								  (complete)
// -------------------------------------------------------------

void PulseStart_stdTTA()
{
	//Stop Timer 1
	Stop_Timer_100us();
	
	//Set flags
	flag_std_TTA = 1;
	flag_DPA_TTA = 0;
	flag_HPP_TTA = 0;
	
	//Reset Timer 5 and change Compare Values
	Setup_Counter_for_stdTTA();
	
	//Status LED for Pulses
	set_bit(LED_Pulse);
	
	//HP and MP set
	if(heat_pulse_length > 0 )
	{
		HP_Port = pulse_output_register;
	}
	MP_Port = pulse_output_register;
	
	//Start Timer 100us
	Start_Timer_100us();
	
}

// -------------------------------------------------------------
// Sensitivity start								  (complete)
// -------------------------------------------------------------

void PulseStart_Sensitivity()
{
	//Status LED for Pulses
	set_bit(LED_Pulse);
	
	//Ports set
	MP_Port = pulse_output_register;
}

// -------------------------------------------------------------
// Deterministic Pulses (edit Maxi 26.09.2018)		  (complete)
// -------------------------------------------------------------

void PulseStart_DPA_TTA()
{
	//Stop Timer 1
	Stop_Timer_100us();
	
	// Set Flags
	flag_DPA_TTA = 1;
	flag_std_TTA = 0;
	flag_HPP_TTA = 0;
	
	//Setup for det TTA
	Setup_Counter_for_DPA_TTA();
	
	//Status LED for Pulses
	set_bit(LED_Pulse);
	
	//Switch off all Heat pulses
	HP_Port = 0;
	MP_Port = 0;
	
	//Start Timer 1
	Start_Timer_100us();
}

// -------------------------------------------------------------
// Deterministic Pulses with Heat PrePulse (edit Maxi 12.09.2019)
//													  ()
// -------------------------------------------------------------

void PulseStart_DPA_TTA_HighStart()
{
	//Stop Timer 1
	Stop_Timer_100us();
	
	// Set Flags
	flag_DPA_TTA = 0;
	flag_std_TTA = 0;
	flag_HPP_TTA = 1;
	
	//Setup for HeatPrePulse
	Setup_Counter_for_DPA_TTA_HighLevel();
	
	//Status LED for Pulses
	set_bit(LED_Pulse);
	
	//Switch ON all pulses
	HP_Port = pulse_output_register;
	MP_Port = pulse_output_register;
	
	//Start Timer 1
	Start_Timer_100us();
}

void PulseStart_DPA_TTA_fromHPP()
{
	//Stop Timer 1
	Stop_Timer_100us();
	
	// Set Flags
	flag_DPA_TTA = 1;
	flag_std_TTA = 0;
	flag_HPP_TTA = 0;
	
	//Setup for det TTA
	Setup_Counter_for_DPA_TTA();
	
	//Status LED for Pulses
	set_bit(LED_Pulse);
	
	//Switch off all Heat pulses (don't switch off here)
	//HP_Port = 0;
	//MP_Port = 0;
	
	//Start Timer 1
	Start_Timer_100us();
}

// -------------------------------------------------------------
// PrePulse (edit Maxi 27.09.2018)					  (complete)
// -------------------------------------------------------------

void PulseStart_PrePulse()
{
	//Stop all Pulses
	PulseStop();

	//Start Pulse
	MP_Port = pulse_output_register;
	
	//Wait
	_delay_ms(20);
	
	//Stop all Pulses
	PulseStop();
}


