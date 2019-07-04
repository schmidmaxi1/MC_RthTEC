/* ----------------------------------------------------------
 *
 * Project:  LED Test system
 * Module:   DAC, AD5752R
 *
 * Author:   Johannes Knauss 2017, mail@iet-chiemsee.de
 *
 *
 * ----------------------------------------------------------
 */



/*
 ** Includes
 */

#include "main.h"
#include "ad5752.h"



/*
 ** Compiler-Konstanten
 */



/*
 ** Variablen
 */

//uint8_t last_send_dac = 255;
//uint32_t last_ChipSelectPin = 0;
volatile uint8_t *last_ChipSelectPort = 0;
uint8_t last_ChipSelectPin = 0;



/*
 ** Funktionen
 */

uint8_t DAC_TXWaiting(uint8_t byte, uint8_t dac);

void DAC_Send(uint8_t byte, volatile uint8_t *port, uint8_t pin);

void DAC_Range_and_PowerUp(volatile uint8_t *port, uint8_t pin);




// -------------------------------------------------------------
// Initialisieren
// -------------------------------------------------------------

void DAC_Init()
{
	/*
    set_bit(DAC_1_SYNC);
    clear_bit(DAC_1_CLR);
    set_bit(DAC_1_LDAC);

    set_out(DAC_1_SYNC);
    set_out(DAC_1_CLR);
    set_out(DAC_1_LDAC);

    set_bit(DAC_2_SYNC);
    clear_bit(DAC_2_CLR);
    set_bit(DAC_2_LDAC);

    set_out(DAC_2_SYNC);
    set_out(DAC_2_CLR);
    set_out(DAC_2_LDAC);
	*/
	
	//Clear und Latch-Pins definieren (Inhalt löschen mit CLR = 0)
	clear_bit(DAC_CLR);
	set_bit(DAC_LDAC);
	set_out(DAC_CLR);
	set_out(DAC_LDAC);
	
	//Bautrate
    USART_SPI_InitBaudrate(125);
	
	//Clear auf 1 setzen (Mit löschen aufhören
    set_bit(DAC_CLR);
	
	//DAC initialisieren (LED):
	DAC_Range_and_PowerUp(&IO_Port3, 0);
	//DAC_Range_and_PowerUp(&IO_PORT4, 0);
	
	//Output werte setzen
	LEDSource_set_Heat_Current(heat_pulse_current, 0);
	LEDSource_set_Meas_Current(measure_pulse_current, 0);
	
	//DAC initialisieren(FrontEnd:
	DAC_Range_and_PowerUp(&IO_Port3, 1);
	//DAC_Range_and_PowerUp(&IO_PORT4, 1);
		
	//Output werte setzen
	FrontEnd_set_Offset_Voltage(offset_voltage, 1);
	
	//DAC initialisieren (MOSFET):
	DAC_Range_and_PowerUp(&IO_Port3, 2);
	DAC_Range_and_PowerUp(&IO_PORT4, 2);
		
	//Output werte setzen
	MOSFETSource_set_Heat_Current(heat_pulse_current,2);
	MOSFETSource_set_Meas_Current(measure_pulse_current,2);
	MOSFETSource_set_Meas_Voltage(measure_pulse_voltage,2);
	

		
	//Output werte setzen
	

	
    //DAC_Set((((uint32_t) heat_pulse_current) * 0xffff) / 1500, &ChipSelect_Port , 1, DAC_ADR_DAC_A);
    //DAC_Set((((uint32_t) measure_pulse_current) * 0xffff) / 250, &ChipSelect_Port , 1, DAC_ADR_DAC_B);
	

    
    clear_bit(DAC_LDAC);
   
	/*
    //DAC Window
    //DAC_Send(DAC_REG_RANGE | DAC_ADR_DAC_AB,DAC_2);     //Range
    //DAC_Send(0,DAC_2);                                  //XX
    //DAC_Send(3,DAC_2);                                  //+-5V

    //DAC_Send(DAC_REG_POWER,DAC_2);                      //Power
    //DAC_Send(0,DAC_2);                                  //XX
    //DAC_Send(5,DAC_2);                                  //Power up ch A+B
	*/
}



// -------------------------------------------------------------
// Sendepuffer belegt
// -------------------------------------------------------------

uint8_t DAC_TXWaiting(uint8_t byte, uint8_t dac)
{
    return USART_SPI_TXWaiting();
}





// -------------------------------------------------------------
// Bytes senden
// -------------------------------------------------------------
/*
void DAC_Send(uint8_t byte, uint8_t dac)
{
    if(last_send_dac != dac)
    {
        while(USART_SPI_TXWaiting());

        last_send_dac = dac;

		//Edit: Maxi 20.12.2017
		if(dac == DAC_1){
			//CS Pin auswählen			
			USART_SPI_SetCS(&DAC_1_SYNC_PORT,DAC_1_SYNC_PIN,1,3);
			//Auf mode 1 stellen (oder operation)
			SPCR |= _BV(CPHA);
		}
		if(dac == DAC_2){
			USART_SPI_SetCS(&DAC_2_SYNC_PORT,DAC_2_SYNC_PIN,1,2);
			//Auf Mode 0 stellen (und mit invertierten Byte)
			SPCR &= ~(_BV(CPHA));
		}
    }
    USART_SPI_TxByte(byte);
}
*/

void SPI_Send(uint8_t byte, volatile uint8_t *port, uint8_t pin)
{

	//Wenn sich Port oder Pin geändert habe, CS Pin ändern
	if(last_ChipSelectPin != pin || last_ChipSelectPort != port)
	{
		while(USART_SPI_TXWaiting());

		last_ChipSelectPort = port;
		last_ChipSelectPin = pin;

		//CS Pin auswählen
		USART_SPI_SetCS(port, pin, 1, 3);
		//Auf mode 1 stellen (oder operation)
		SPCR |= _BV(CPHA);
	}
	USART_SPI_TxByte(byte);
	

	
}

void DAC_Range_and_PowerUp(volatile uint8_t *port, uint8_t pin){
	//OUTPUT Range Select Register  (REG = 001) für beide Kanäle
	SPI_Send(DAC_REG_RANGE | DAC_ADR_DAC_AB, port, pin);		//Register & Kanal
	SPI_Send(0, port, pin);                                  //XX
	SPI_Send(0, port, pin);                                  //Range: +5V

	//Power Control Register  (REG = 010)
	SPI_Send(DAC_REG_POWER, port, pin);                      //Power
	SPI_Send(0, port, pin);                                  //XX
	SPI_Send(5, port, pin);                                  //Power up ch A+B
}


// -------------------------------------------------------------
// Ausgang setzen
// -------------------------------------------------------------

void DAC_Set(uint16_t val, volatile uint8_t *port, uint8_t pin, uint8_t ch)
{
	SPI_Send(DAC_REG_DAC | ch, port, pin);
	SPI_Send(val>>8, port, pin);
	SPI_Send(val, port, pin);		
}

//AB hier nicht mehr in der DAC klasse

// -------------------------------------------------------------
// DAC Ausgangs-Spannung anpassen
// -------------------------------------------------------------

void LEDSource_set_Heat_Current(uint16_t heatCurrent, uint8_t slot)
{
	/*
	Input:
	-heatCurrent in mA
	-channel (Einschubkarten-Platz)
	-Heizquelle immer an ChipSelect [channel] & Kanal A
	*/
	
	//Binär Wert berechnen
	uint16_t binary_value = (((uint32_t) heatCurrent) * 0xffff) / 1500;
	
	//Senden
	SPI_Send(DAC_REG_DAC | DAC_ADR_DAC_A, &IO_Port3, slot);
	SPI_Send(binary_value>>8, &IO_Port3, slot);
	SPI_Send(binary_value, &IO_Port3, slot);	
}

void LEDSource_set_Meas_Current(uint16_t measCurrent, uint8_t slot)
{
	/*
	Input:
	-measCurrent in 0,1mA
	-channel (Einschubkarten-Platz)
	-Heizquelle immer an ChipSelect [channel] & Kanal B
	*/
	
	//Binär Wert berechnen
	uint16_t binary_value = (((uint32_t) measCurrent) * 0xffff) / 250;
	
	//Senden
	SPI_Send(DAC_REG_DAC | DAC_ADR_DAC_B, &IO_Port3, slot);
	SPI_Send(binary_value>>8, &IO_Port3, slot);
	SPI_Send(binary_value, &IO_Port3, slot);	
		
}

void MOSFETSource_set_Heat_Current(uint16_t heatCurrent, uint8_t slot)
{
	/*
	Input:
	-heatCurrent in mA
	-channel (Einschubkarten-Platz)
	-Heizquelle immer an ChipSelect [channel] & Kanal A
	*/
	
	//Binär Wert berechnen
	uint16_t binary_value = (((uint32_t) heatCurrent) * 0xffff) / 5000;
	
	//Senden
	SPI_Send(DAC_REG_DAC | DAC_ADR_DAC_A, &IO_Port3, slot);
	SPI_Send(binary_value>>8, &IO_Port3, slot);
	SPI_Send(binary_value, &IO_Port3, slot);	
}

void MOSFETSource_set_Meas_Current(uint16_t measCurrent, uint8_t slot)
{
	/*
	Input:
	-measCurrent in 0,1mA
	-channel (Einschubkarten-Platz)
	-Heizquelle immer an ChipSelect [channel] & Kanal B
	*/
	
	//Binär Wert berechnen
	uint16_t binary_value = (((uint32_t) measCurrent) * 0xffff) / 250;
	
	//Senden	
	SPI_Send(DAC_REG_DAC | DAC_ADR_DAC_B, &IO_Port3, slot);
	SPI_Send(binary_value>>8, &IO_Port3, slot);
	SPI_Send(binary_value, &IO_Port3, slot);	
		
}

void MOSFETSource_set_Meas_Voltage(uint16_t measVoltage, uint8_t slot){
		/*
	Input:
	-measCurrent in 0,1mA
	-channel (Einschubkarten-Platz)
	-Heizquelle immer an ChipSelect [channel] & Kanal B
	*/
	
	//Binär Wert berechnen (URef = 0,33 * U_A- 1,61V)
	uint16_t binary_value = ((((uint32_t)measVoltage) / 3 * 10 - 1610) * 0xffff) / 5000;
	
	
	//Senden	
	SPI_Send(DAC_REG_DAC | DAC_ADR_DAC_A, &IO_PORT4, slot);
	SPI_Send(binary_value>>8, &IO_PORT4, slot);
	SPI_Send(binary_value, &IO_PORT4, slot);	
}


void FrontEnd_set_Offset_Voltage(uint16_t offsetVoltage, uint8_t slot)
{
	/*
	Input:
	-offsetVoltage in mV
	-channel (Einschubkarten-Platz)
	-Heizquelle immer an ChipSelect [channel] & Kanal A
	*/
	
	//Binär Wert berechnen
	uint16_t binary_value = (((uint32_t) offsetVoltage) * 0xffff) / 10000;
	
	//Senden
	SPI_Send(DAC_REG_DAC | DAC_ADR_DAC_A, &IO_Port3, slot);
	SPI_Send(binary_value>>8, &IO_Port3, slot);
	SPI_Send(binary_value, &IO_Port3, slot);	
}



