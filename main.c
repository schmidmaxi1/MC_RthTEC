/* ----------------------------------------------------------
 *
 * Project:  LED Test system
 * Module:   Main
 *
 * Version:  see main.h
 *
 * Author:   Johannes Knauss 2017, mail@iet-chiemsee.de
 *
 * Clock:    16.000 MHz
 *
 *
 * ----------------------------------------------------------
 */



/*
 ** Includes
 */

#include "main.h"

#include "avr/pgmspace.h"

#include <util/delay.h>


/*
 ** Constants
 */

//Firmware Version
const char firmware_version[] PROGMEM = S(VER_START:FIRMWARE_VERSION:VER_END);



/*
 ** Variables
 */

//General
uint16_t firmware_code_eeprom EEMEM;
uint16_t heat_pulse_current_eeprom EEMEM;
uint32_t heat_pulse_length_eeprom EEMEM;
uint16_t measure_pulse_current_eeprom EEMEM;
uint32_t measure_pulse_length_eeprom EEMEM;
uint16_t measure_pulse_voltage_eeprom EEMEM;
uint16_t window_offset_eeprom EEMEM;



//Edit: Maxi 20.12.17
uint16_t offset_voltage_eeprom EEMEM;
uint16_t offset_voltage;

//For Deterministic pulses
uint16_t deterministic_pulse_length;
uint16_t deterministic_pulse_cycles;
uint16_t deterministic_pulse_length_eeprom EEMEM;
uint16_t deterministic_pulse_cycles_eeprom EEMEM;

//For Pulse Output Register
uint8_t pulse_output_register;
uint8_t pulse_output_register_eeprom EEMEM;

//Flags
uint8_t flag_standard_TTA;
uint8_t flag_deterministic_TTA;

/*
 ** Functions
 */ 

// -------------------------------------------------------------
// Main function
// -------------------------------------------------------------

int main()
{	
	Init();

    UART0ClearRx();
	

    //Main loop
	while (1)
	{
		
    
        //Watchdog
        wdt_reset();

        TerminalCheckCommand();


        //100Hz
        if (interrupt_1ms >= 10)
        {
            interrupt_1ms = 0;
            interrupt_10hz++;
        }


        //10Hz
        else if (interrupt_10hz >= 10)
        {
            interrupt_10hz = 0;
            interrupt_1hz++;

            //USB active led
            if (uart0_active_timer)
            {
                uart0_active_timer--;
                set_bit(LED_DATA);
            }
            else
            {
                clear_bit(LED_DATA);
            }


            //Enable
            if (current_source_enabled)
            {
                set_bit(LED_1A);
                set_bit(LED_1B);
            }
            else
            {
                set_bit(LED_1A);
                clear_bit(LED_1B);
            }
			
        }

        //1Hz
        else if (interrupt_1hz >= 10)
        {
            interrupt_1hz = 0;									
        }
    }
}

// -------------------------------------------------------------
// Initialize
// -------------------------------------------------------------

void Init()
{
    /************************************************************************/
    /*                             Watchdog                                 */
    /************************************************************************/
    wdt_enable(WDTO_1S);
    wdt_reset();
	
	
	/************************************************************************/
	/*                               Timer                                  */
	/************************************************************************/
	
    Init_Timer_1ms();

	Init_Timer_100us();

	Init_Counter_100us();
	
	//Timer 4 Anhalten und Counter Register reseten
	PulseStop();
	
	/************************************************************************/
	/*               I/O Pins konfigurieren                                 */
	/************************************************************************/
	
	Init_IO_Pins();
	
	/************************************************************************/
	/*               EEProm init, load factory defaults                     */
	/************************************************************************/

	//Werte werden nur angepasst, wenn sich FIRMWARE_Version geändert hat
	
    if (eeprom_read_word(&firmware_code_eeprom) != FIRMWARE_CODE)
    {

        //LEDs on
        set_bit(LED_1A);
        set_bit(LED_1B);
		set_bit(LED_DATA);
        set_bit(LED_Pulse);

        eeprom_write_word(&firmware_code_eeprom, FIRMWARE_CODE);
        
        eeprom_write_word(&heat_pulse_current_eeprom, 50);
        eeprom_write_word(&measure_pulse_current_eeprom, 50);
        eeprom_write_word(&window_offset_eeprom, 12000);
		
		//Edit: Maxi 20.12.17
		eeprom_write_word(&offset_voltage_eeprom, 3000);

        
        eeprom_write_dword(&heat_pulse_length_eeprom, 25);
        eeprom_write_dword(&measure_pulse_length_eeprom, 25);
		
		eeprom_write_word(&deterministic_pulse_length_eeprom, 10);
		eeprom_write_word(&deterministic_pulse_cycles_eeprom, 100);
		
		eeprom_write_byte(&pulse_output_register_eeprom, 0);

        
        //Watchdog
        wdt_reset();
    }

	//Allow interrupts ( set enable interrupt)
    sei();

	/************************************************************************/
	/*                                  Terminal                            */
	/************************************************************************/
	

    TerminalInit();
	
	/************************************************************************/
	/*                              Variables init                          */
	/************************************************************************/

	//Variablen aus mit Werten aus EEPROM initialisiern
    heat_pulse_current = eeprom_read_word(&heat_pulse_current_eeprom);
    measure_pulse_current = eeprom_read_word(&measure_pulse_current_eeprom);
    heat_pulse_length = eeprom_read_dword(&heat_pulse_length_eeprom);
    measure_pulse_length = eeprom_read_dword(&measure_pulse_length_eeprom);
    window_offset = eeprom_read_word(&window_offset_eeprom);
	offset_voltage = eeprom_read_word(&offset_voltage_eeprom);	
	deterministic_pulse_length = eeprom_read_word(&deterministic_pulse_length_eeprom);
	deterministic_pulse_cycles = eeprom_read_word(&deterministic_pulse_cycles_eeprom);
	pulse_output_register = eeprom_read_byte(&pulse_output_register_eeprom);

    current_source_enabled = 0;


    /************************************************************************/
    /*                            DAC                                       */
    /************************************************************************/
	
    DAC_Init();


#if DEBUG_ENABLE
    UART0TransmitStringLn(FIRMWARE_VERSION);
    UART0TransmitStringLn("");
#endif


    /************************************************************************/
    /*                         Startup LED sequence                         */
    /************************************************************************/
    //LEDs leuchten am Anfang (Lauflicht)
	Lauflicht();
	
}

void Init_IO_Pins(){
	
	//1. Signal LEDs
	clear_bit(LED_1A);
	clear_bit(LED_1B);
	clear_bit(LED_DATA);
	clear_bit(LED_Pulse);
	
	set_out(LED_1A);
	set_out(LED_1B);
	set_out(LED_Pulse);
	set_out(LED_DATA);
	
	//2. HP, MP & ChipSelect [alle]
	for(int i = 0; i < 7; i++){
		//Auf 0 setzen
		_clear_bit(HP_Port, i);
		_clear_bit(MP_Port, i);
		_set_bit(IO_Port3, i);
		_set_bit(IO_PORT4, i);
		//Als Output setzen
		_set_out(HP_Port, i);
		_set_out(MP_Port, i);
		_set_out(IO_Port3, i);
		_set_out(IO_PORT4, i);
	}
	
	//3. Alle anderen (für ADC_MOSFET)
	set_out(IO_PORT5_3);
	clear_bit(IO_PORT5_3);

   
	//4. Fail&Enable
	clear_bit(ENABLE);
	set_out(ENABLE);
	
	clear_bit(FAIL_OUT);
	set_out(FAIL_OUT);
	
	set_in(FAIL_IN);
}

void Lauflicht(){
	startup_wait_counter = 6;
		
	while (startup_wait_counter)
	{
		if (interrupt_1ms >= 10)
		{
			interrupt_1ms = 0;

			if (++interrupt_10hz == 10)
			{
				interrupt_10hz = 0;
				startup_wait_counter--;

				//Watchdog
				wdt_reset();
			}
		}

		if (startup_wait_counter == 6)
		{
			set_bit(LED_1A);
		}

		if (startup_wait_counter == 5)
		{
			clear_bit(LED_1A);
			set_bit(LED_1B);
		}

		if (startup_wait_counter == 4)
		{
			set_bit(LED_1A);
			set_bit(LED_1B);
		}

		if (startup_wait_counter == 3)
		{
			clear_bit(LED_1A);
			clear_bit(LED_1B);
			set_bit(LED_DATA);
		}

		if (startup_wait_counter == 2)
		{
			clear_bit(LED_DATA);
			set_bit(LED_Pulse);
		}

		if (startup_wait_counter == 1)
		{
			clear_bit(LED_Pulse);
		}
	}
}


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
	flag_standard_TTA = 1;
	flag_deterministic_TTA = 0;
	
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

void PulseStart_detTTA()
{
	//Stop Timer 1
	Stop_Timer_100us();
	
	// Set Flags
	flag_deterministic_TTA = 1;
	flag_standard_TTA = 0;
		
	//Setup for det TTA
	Setup_Counter_for_detTTA();
		
	//Status LED for Pulses
	set_bit(LED_Pulse);
			
	//Switch off all Heat pulses
	HP_Port = 0;
	MP_Port = 0;
	
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




