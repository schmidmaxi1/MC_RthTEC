/*
 * Config.h
 *
 * Created: 20.03.2020 10:11:19
 *  Author: schmidm
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_

//Holds all global SETTINGs
//Pins, Freq, ...

//Was earlier in main.h
//change because of circular include

/*
 ** User settings
 */

#define F_CPU               16000000UL          //Systemtakt in Hz - Definition als unsigned long (!)


/*
 ** Compiler constants
 */

#define FIRMWARE_VER_R      V0.0.9
#define FIRMWARE_CODE       0x0009

#define CONTROLLER_TYPE_R   RthTEC_mC_Unit_V2

#define FIRMWARE_VERSION    S(CONTROLLER_TYPE_R FIRMWARE_VER_R)
#define CONTROLLER_TYPE     S(CONTROLLER_TYPE_R)
#define SOFTWARE_VERSION	S(FIRMWARE_VER_R)


#define DEBUG_ENABLE        0
#define VT100_ENABLE        0

/*
 ** Pin-Setup for MCU-Bord V_2
 */

//1. Signal LEDs
#define LED_1A              PORTG,3			//green
#define LED_1B              PORTG,2			//red
#define LED_DATA            PORTG,0			//green --> Data-Transfer
#define LED_Pulse           PORTG,1			//red	--> Pulse in use


//2. Fail & Enable & DAC-Latch
#define FAIL_IN             PORTB,7
#define FAIL_OUT            PORTB,6
#define ENABLE              PORTE,2

#define DAC_CLR				PORTE,4
#define DAC_LDAC			PORTE,1


//3. Heat-Pulse-Pins (IO x.1) [for up to 8 Slots]
//All on Port H
#define HP_Port	PORTH

#define HP_1	PORTH,0
#define HP_2	PORTH,1
#define HP_3	PORTH,2
#define HP_4	PORTH,3
#define HP_5	PORTH,4
#define HP_6	PORTH,5
#define HP_7	PORTH,6
#define HP_8	PORTH,7

//4. Meas-Pulse-Pins (IO x.2)[for up to 8 Slots]
//All on Port K
#define MP_Port	PORTK

#define MP_1	PORTK,0
#define MP_2	PORTK,1
#define MP_3	PORTK,2
#define MP_4	PORTK,3
#define MP_5	PORTK,4
#define MP_6	PORTK,5
#define MP_7	PORTK,6
#define MP_8	PORTK,7

//5. ChipSelct-Pins (IO x.3)[for up to 8 Slots]
//All on Port A
#define IO_PORT3			PORTA

#define IO_PORT3_1			PORTA,0
#define IO_PORT3_2			PORTA,1
#define IO_PORT3_3			PORTA,2
#define IO_PORT3_4			PORTA,3
#define IO_PORT3_5			PORTA,4
#define IO_PORT3_6			PORTA,5
#define IO_PORT3_7			PORTA,6
#define IO_PORT3_8			PORTA,7

//6. ChipSelct-Pins (IO x.4)[for up to 8 Slots]
//All on Port J
#define IO_PORT4			PORTJ

#define IO_PORT4_1			PORTJ,0
#define IO_PORT4_2			PORTJ,1
#define IO_PORT4_3			PORTJ,2
#define IO_PORT4_4			PORTJ,3
#define IO_PORT4_5			PORTJ,4
#define IO_PORT4_6			PORTJ,5
#define IO_PORT4_7			PORTJ,6
#define IO_PORT4_8			PORTJ,7

//7. ChipSelct-Pins (IO x.5)[for up to 8 Slots]
//All on Port D
#define IO_PORT5			PORTD

#define IO_PORT5_1			PORTD,0
#define IO_PORT5_2			PORTD,1
#define IO_PORT5_3			PORTD,2
#define IO_PORT5_4			PORTD,3
#define IO_PORT5_5			PORTD,4
#define IO_PORT5_6			PORTD,5
#define IO_PORT5_7			PORTD,6
#define IO_PORT5_8			PORTD,7


//8. ChipSelct-Pins (IO x.6)[for up to 8 Slots]
//All on Port D
#define IO_PORT6			PORTC

#define IO_PORT6_1			PORTC,0
#define IO_PORT6_2			PORTC,1
#define IO_PORT6_3			PORTC,2
#define IO_PORT6_4			PORTC,3
#define IO_PORT6_5			PORTC,4
#define IO_PORT6_6			PORTC,5
#define IO_PORT6_7			PORTC,6
#define IO_PORT6_8			PORTC,7





#endif /* CONFIG_H_ */