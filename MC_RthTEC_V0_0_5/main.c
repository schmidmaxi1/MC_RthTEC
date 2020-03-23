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


//Moved to Global var
//uint16_t firmware_code_eeprom EEMEM;
//uint32_t measure_pulse_length_eeprom EEMEM;
//uint32_t heat_pulse_length_eeprom EEMEM;
//uint16_t deterministic_pulse_length_eeprom EEMEM;
//uint16_t deterministic_pulse_cycles_eeprom EEMEM;
//uint8_t pulse_output_register_eeprom EEMEM;
//uint8_t pulse_output_register;
//char card_Type[8];
//char card_Type_register_eeprom[8] EEMEM;
//uint16_t deterministic_pulse_length;
//uint16_t deterministic_pulse_cycles;




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
	/*                        General Settings                              */
	/************************************************************************/
	
	//Clock Divider auf 1
	clock_prescale_set(clock_div_1);
	
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
		
		EEPROM_default_Values();
      
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
	EEPROM_last_Values();

    current_source_enabled = 0;
	
	/************************************************************************/
	/*                              SPI										*/
	/************************************************************************/
		
	USART_SPI_InitBaudrate(125);
	
	/************************************************************************/
	/*                              Init Cards                              */
	/************************************************************************/
	
	
	//Is compared to 00...0 to initialize all used cards
	//"!" is used to avoid overwriting epromm with default values
	char init_card_types[8] = "!!!!!!!!"; 
	Init_All_Cards(card_Type, init_card_types);


    /************************************************************************/
    /*                            DAC     (alt)                             */
    /************************************************************************/
	
    //DAC_Init();


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
	
	//2. HP, MP & ChipSelect [alle] (wahrscheinlich nicht notwendig)
	for(int i = 0; i < 7; i++){
		//Auf 0 setzen
		_clear_bit(HP_Port, i);
		_clear_bit(MP_Port, i);
		_set_bit(IO_PORT3, i);
		_set_bit(IO_PORT4, i);
		//Als Output setzen
		_set_out(HP_Port, i);
		_set_out(MP_Port, i);
		_set_out(IO_PORT3, i);
		_set_out(IO_PORT4, i);
	}
	
	//3. ADC (Latch und Clear) Beide auf HIGH damit gleich ausgegeben
	set_bit(DAC_CLR);
	set_bit(DAC_LDAC);
	set_out(DAC_CLR);
	set_out(DAC_LDAC);
	
   
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

void EEPROM_default_Values()
{
	eeprom_write_word(&firmware_code_eeprom, FIRMWARE_CODE);
	
	
	eeprom_write_dword(&heat_pulse_length_eeprom, 25);
	eeprom_write_dword(&measure_pulse_length_eeprom, 25);
	
	eeprom_write_word(&deterministic_pulse_length_eeprom, 10);
	eeprom_write_word(&deterministic_pulse_cycles_eeprom, 100);
	
	eeprom_write_byte(&pulse_output_register_eeprom, 0);
	
	char init_card_types[8] = "00000000"; 
	eeprom_write_block(&card_Type_register_eeprom, init_card_types ,8);	
	
	//Default values of the single cards are first used if a card is initialized
}

void EEPROM_last_Values()
{
	heat_pulse_length = eeprom_read_dword(&heat_pulse_length_eeprom);
	measure_pulse_length = eeprom_read_dword(&measure_pulse_length_eeprom);
	
	deterministic_pulse_length = eeprom_read_word(&deterministic_pulse_length_eeprom);
	deterministic_pulse_cycles = eeprom_read_word(&deterministic_pulse_cycles_eeprom);
	
	pulse_output_register = eeprom_read_byte(&pulse_output_register_eeprom);
	eeprom_read_block(card_Type, &card_Type_register_eeprom ,8);
	
	//Je nach SlotBelegung die Parameter übernehmen
	for(int i = 0; i < 8; i++)
	{
		switch(card_Type[i])
		{
			case '0':
				break;
				
			case 'A':
				Amplifier_Variables_from_EEPROM(i+1);
				break;
			
			case 'L':
				LED_Source_Variables_from_EEPROM(i+1);
				break;

			case 'M':
				MOSFET_Source_Variables_from_EEPROM(i+1);
				break;
				
			case 'F':
				FrontEnd_Variables_from_EEPROM(i+1);
				break;	
				
			case 'B':
				BreakDown_Variables_from_EEPROM(i+1);
				break;		
				
			case 'T':
				Slot_Tester_Variables_from_EEPROM(i+1);
				break;
		}
	}
	
}

void Init_All_Cards(char newCard_Type[], char oldCard_Type[])
{
	//Check all characters
	for(int i = 0; i< 8; i++)
	{
		//If character has change --> Init Card
		if(newCard_Type[i] != oldCard_Type[i])
		{
			//Depending on Card-Typ --> Init Card
			switch(newCard_Type[i])
			{
				//'!' means after a RST --> EEPROM values correct --> no default needed
				case 'A':
					if(oldCard_Type[i]!= '!')
					{
						Amplifier_Default_Values(i+1);
					}
					Amplifier_Init(i+1);
					break;
					
				case 'L':
					if(oldCard_Type[i]!= '!')
					{
						LED_Source_Default_Values(i+1);
					}
					LED_Source_Init(i+1);
					break;
					
				case 'M':
					if(oldCard_Type[i]!= '!')
					{
						MOSFET_Source_Default_Values(i+1);
					}
					MOSFET_Source_Init(i+1);
					break;
					
				case 'B':
					if(oldCard_Type[i]!= '!')
					{
						BreakDown_Default_Values(i+1);
					}
					BreakDown_Init(i+1);
					break;
					
				case 'T':
					if(oldCard_Type[i]!= '!')
					{
						Slot_Tester_Default_Values(i+1);
					}
					Slot_Tester_Init(i+1);
					break;
					
				case 'F':
					if(oldCard_Type[i]!= '!')
					{
						FrontEnd_Default_Values(i+1);
					}
					FrontEnd_Init(i+1);
					break;
					
				default:
					break;
			}
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




