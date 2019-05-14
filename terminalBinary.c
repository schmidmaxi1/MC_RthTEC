/* ----------------------------------------------------------
 *
 * Project:  TEC Peltiercontroller
 * Module:   Terminal binary mode
 *
 * Author:   Johannes Knauss 2016, mail@iet-chiemsee.de
 *
 *
 * Test:     7E 01 99 07 90 02 6B 5C 7E (Firmware)
 *
 * ----------------------------------------------------------
 */



/*
 ** Includes
 */

#include <string.h>

#include "main.h"



/*
 ** Compiler constants
 */

#define DEST_ID             0
#define SOURCE_ID           1
#define BINARY_LENGHT       2
#define BINARY_FUNC_H       3
#define BINARY_FUNC_L       4
#define BINARY_DATA         5



/*
 ** Variables
 */

uint8_t frame_length;
uint8_t binary_frame[128];

extern uint8_t msg_recv_id;



/*
 ** Functions
 */

void TerminalBinaryCheckCommand(char *rx_pointer, uint8_t rx_length);
void TerminalBinaryParseCommand(uint16_t cmd, uint8_t data_length, char *data);
void PushAllValues();

void InitBinary(uint8_t dest_id);
void PushBinary(uint8_t data);
void PushBinary16(uint16_t data);
void PushBinaryString(char *str);
void TransmitBinary();
uint16_t CalcCRC16(uint8_t *pointer, uint8_t len);
inline int16_t read16(char *data);



// -------------------------------------------------------------
// Befehl von Interface empfangen
// -------------------------------------------------------------

void TerminalBinaryCheckCommand(char *rx_pointer, uint8_t rx_length)
{
    uint8_t id_counter = 0;

    //Minimale Telegrammlänge 7 Bytes
    if (rx_length < 7) return;
    
    //CRC berechnen
    uint16_t crc = CalcCRC16((uint8_t *) rx_pointer, rx_pointer[BINARY_LENGHT]-2);
    uint16_t checksum = rx_pointer[rx_pointer[BINARY_LENGHT]-2] | rx_pointer[rx_pointer[BINARY_LENGHT]-1]<<8;

    //Wenn CRC ok (oder CRC cheat)
    if (crc == checksum || checksum == 0xAAAA)
    {
        terminal_send_crc_error = 0;

        //this id
        if (rx_pointer[DEST_ID] == receiver_id)
        {
            InitBinary(rx_pointer[SOURCE_ID]);

            //Length correct
            if (rx_pointer[BINARY_LENGHT] == rx_length)
            {
                //ParseCommand, Params: function, data pointer, data length
                TerminalBinaryParseCommand(rx_pointer[BINARY_FUNC_H]<<8|rx_pointer[BINARY_FUNC_L],rx_pointer[BINARY_LENGHT]-7,&rx_pointer[BINARY_DATA]);
            }
            else
            {
                PushBinary16(0xFFA4);                       //length error
                PushBinary(rx_pointer[BINARY_LENGHT]);      //length set
                PushBinary(rx_length);                      //length received
            }

            TransmitBinary();
        }
        //Pass through, if master (ID == 1)
        else if (receiver_id == 1 && rx_pointer[DEST_ID] <= 99)
        {

            //Length correct
            if (rx_pointer[BINARY_LENGHT] == rx_length)
            {
                if (interface == INT_USB)
                {
                    UART1TxByte(0x7E);
                    UART1TransmitBytes(rx_pointer, rx_pointer[BINARY_LENGHT]);
                    UART1TxByte(0x7E);
                }

                if (interface == INT_RS485)
                {
                    UART0TxByte(0x7E);
                    UART0TransmitBytes(rx_pointer, rx_pointer[BINARY_LENGHT]);
                    UART0TxByte(0x7E);
                }
            }
            else
            {
                PushBinary16(0xFFA4);                       //length error
                PushBinary(rx_pointer[BINARY_LENGHT]);      //length set
                PushBinary(rx_length);                      //length received
            }
        }

        //global id, do specials
        if(rx_pointer[DEST_ID] == 0)
        {
            InitBinary(rx_pointer[SOURCE_ID]);

            //Length correct
            if (rx_pointer[BINARY_LENGHT] == rx_length)
            {
                //ParseCommand, Params: function, data pointer, data length
                TerminalBinaryParseCommand(rx_pointer[BINARY_FUNC_H]<<8|rx_pointer[BINARY_FUNC_L],rx_pointer[BINARY_LENGHT]-7,&rx_pointer[BINARY_DATA]);
            }
            else
            {
                PushBinary16(0xFFA4);                       //length error
                PushBinary(rx_pointer[BINARY_LENGHT]);      //length set
                PushBinary(rx_length);                      //length received
            }

            //wait for other controller
            while(1)
            {
                //Interrupt 1ms
                if (interrupt_1ms == (1280 / rs485_baudrate))
                {
                    interrupt_1ms = 0;

                    //Watchdog
                    wdt_reset();

                    id_counter++;

                    if (id_counter == receiver_id)
                    {
                        break;
                    }
                }
            }

            TransmitBinary();
        }
    }

    //crc failure
    else if(terminal_send_crc_error && rx_pointer[DEST_ID] == receiver_id)
    {
        InitBinary(rx_pointer[SOURCE_ID]);
    
        PushBinary16(0xFFA3);    //crc failure

        PushBinary16(crc);

        TransmitBinary();
    }
}



// -------------------------------------------------------------
// Befehl auswerten
// -------------------------------------------------------------

void TerminalBinaryParseCommand(uint16_t cmd, uint8_t data_length, char *data)
{
    switch (cmd)
    {
    //enable
    case 0x1001:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary(conv_enabled);
        }
        //set
        else if (data[0] == 1 && data_length == 1)
        {
            PushBinary16(cmd);      //function

            //converter error
            if (conv_status)
            {
                PushBinary(0);
            }
            else
            {   
                //reset if not enabled
                if (!conv_enabled)
                {
                    conv_power_override = 0;
                    contr_temp_integral = 0;
                    conv_power = 0;

                    conv_heat_power_limit = 0;
                    conv_cool_power_limit = 0;

                    //Temperature ramp function
                    if (contr_temp_ramp)
                    {
                        contr_temp_set_rampval = contr_temp;
                    }
                    else
                    {
                        contr_temp_set_rampval = contr_temp_set;
                    }
                }

                conv_enabled = 1;

                PushBinary(conv_enabled);
            }
        }
        //set (reset)
        else if (data[0] == 2 && data_length == 1)
        {
            PushBinary16(cmd);      //function

            if (conv_sensor_err)
            {
                PushBinary(0);
            }
            else
            {   
                //reset if not enabled
                if (!conv_enabled)
                {
                    conv_power_override = 0;
                    contr_temp_integral = 0;
                    conv_power = 0;

                    conv_heat_power_limit = 0;
                    conv_cool_power_limit = 0;
                }

                conv_enabled = 1;
                conv_tec_err = 0;
                conv_temp_err = 0;
                conv_delta_err = 0;
                conv_status = 0;

                set_bit(LED_TEC_FAIL);
                set_bit(LED_SENSOR_FAIL);

                PushBinary(conv_enabled);
            }
        }
        else if (data[0] == 0 && data_length == 1)
        {
            PushBinary16(cmd);      //function

            conv_enabled = 0;                
            conv_power = 0;
            conv_power_override = 0;

            PushBinary(conv_enabled);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }

        break;

    //temp set
    case 0x1002:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_temp_set);
        }
        //set
        else if (read16(data) >= contr_temp_min && read16(data) <= contr_temp_max && data_length == 2)
        {
            PushBinary16(cmd);      //function

            if (contr_temp_set != read16(data))
            {
                contr_temp_set = read16(data);
                eeprom_write_word(&contr_temp_set_eeprom,read16(data));

                //Temperature ramp function
                if (!contr_temp_ramp)
                {
                    contr_temp_set_rampval = contr_temp_set;

                    //Temperature controller adjustment
                    if (contr_temp < contr_temp_set - 100)
                    {
                        //conv_heat_power_limit = 0;
                        //conv_cool_power_limit = 0;

                        if (contr_temp_integral < 0) contr_temp_integral = 0;
                    }
                    else if (contr_temp > contr_temp_set + 100)
                    {
                        //conv_heat_power_limit = 0;
                        //conv_cool_power_limit = 0;

                        if (contr_temp_integral > 0) contr_temp_integral = 0;
                    }
                }
            }

            PushBinary16(contr_temp_set);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //reset min/max temp value
    case 0x1003:

        //set
        if (data[0] == 1 && data_length == 1)
        {
            PushBinary16(cmd);      //function

            contr_temp_high = -10000;
            contr_temp_low = 30000;

            PushBinary(1);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }

        break;

    //auto enable controller
    case 0x1004:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary(eeprom_read_byte(&conv_auto_enable_eeprom));
        }
        //set
        else if (data[0] >= 0 && data[0] <= 1 && data_length == 1)
        {
            PushBinary16(cmd);      //function

            if (eeprom_read_byte(&conv_auto_enable_eeprom) != data[0]) eeprom_write_byte(&conv_auto_enable_eeprom, data[0]);
            
            PushBinary(data[0]);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }

        break;

    //RS485 baudrate
    case 0x1005:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(rs485_baudrate);
        }
        //set
        else if ((read16(data) == 96 || read16(data) == 144 || read16(data) == 192 || read16(data) == 288 || read16(data) == 384 || read16(data) == 576 || read16(data) == 1152) && data_length == 2)
        {
            PushBinary16(cmd);      //function

            if (rs485_baudrate != read16(data))
            {
                RS485SetBaudrate(read16(data));
            }

            PushBinary16(rs485_baudrate);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //interface mode (ASCII/binary)
    case 0x1006:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary(interface_binary_mode);
        }
        //set
        else if (data[0] >= 0 && data[0] <= 1 && data_length == 1)
        {
            PushBinary16(cmd);      //function

            if (interface_binary_mode != data[0])
            {
                interface_binary_mode = data[0];
                eeprom_write_byte(&interface_binary_mode_eeprom, data[0]);
            }

            PushBinary(interface_binary_mode);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }

        break;

    //ptc 1 coeff
    case 0x2001:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_ptc_1_coeff);
        }
        //set
        else if (read16(data) >= 0 && read16(data) <= 1000 && data_length == 2)
        {
            PushBinary16(cmd);      //function

            if (contr_ptc_1_coeff != read16(data))
            {
                contr_ptc_1_coeff = read16(data);
                eeprom_write_word(&contr_ptc_1_coeff_eeprom, read16(data));
            }

            PushBinary16(contr_ptc_1_coeff);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //ptc 2 coeff
    case 0x2002:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_ptc_2_coeff);
        }
        //set
        else if (read16(data) >= 0 && read16(data) <= 1000 && data_length == 2)
        {
            PushBinary16(cmd);      //function

            if (contr_ptc_2_coeff != read16(data))
            {
                contr_ptc_2_coeff = read16(data);
                eeprom_write_word(&contr_ptc_2_coeff_eeprom, read16(data));
            }

            PushBinary16(contr_ptc_2_coeff);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //ntc coeff
    case 0x2003:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_ntc_coeff);
        }
        //set
        else if (read16(data) >= 0 && read16(data) <= 1000 && data_length == 2)
        {
            PushBinary16(cmd);      //function

            if (contr_ntc_coeff != read16(data))
            {
                contr_ntc_coeff = read16(data);
                eeprom_write_word(&contr_ntc_coeff_eeprom, read16(data));
            }

            PushBinary16(contr_ntc_coeff);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //ptc 1 offset
    case 0x2004:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_ptc_1_offset);
        }
        //set
        else if (read16(data) >= -10000 && read16(data) <= 10000 && data_length == 2)
        {
            PushBinary16(cmd);      //function

            if (contr_ptc_1_offset != read16(data))
            {
                contr_ptc_1_offset = read16(data);
                eeprom_write_word(&contr_ptc_1_offset_eeprom, read16(data));
            }

            PushBinary16(contr_ptc_1_offset);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //ptc 2 offset
    case 0x2005:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_ptc_2_offset);
        }
        //set
        else if (read16(data) >= -10000 && read16(data) <= 10000 && data_length == 2)
        {
            PushBinary16(cmd);      //function

            if (contr_ptc_2_offset != read16(data))
            {
                contr_ptc_2_offset = read16(data);
                eeprom_write_word(&contr_ptc_2_offset_eeprom, read16(data));
            }

            PushBinary16(contr_ptc_2_offset);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //ntc offset
    case 0x2006:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_ntc_offset);
        }
        //set
        else if (read16(data) >= -10000 && read16(data) <= 10000 && data_length == 2)
        {
            PushBinary16(cmd);      //function

            if (contr_ntc_offset != read16(data))
            {
                contr_ntc_offset = read16(data);
                eeprom_write_word(&contr_ntc_offset_eeprom, read16(data));
            }

            PushBinary16(contr_ntc_offset);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //temp max
    case 0x2007:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_temp_max);
        }
        //set
        else if (read16(data) >= contr_temp_min+100 && read16(data) <= 15000 && data_length == 2)
        {
            PushBinary16(cmd);      //function

            if (contr_temp_max != read16(data))
            {
                contr_temp_max = read16(data);
                eeprom_write_word(&contr_temp_max_eeprom,read16(data));
            }

            PushBinary16(contr_temp_max);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //temp min
    case 0x2008:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_temp_min);
        }
        //set
        else if (read16(data) >= -5000 && read16(data) <= contr_temp_max-100 && data_length == 2)
        {
            PushBinary16(cmd);      //function

            if (contr_temp_min != read16(data))
            {
                contr_temp_min = read16(data);
                eeprom_write_word(&contr_temp_min_eeprom,read16(data));
            }

            PushBinary16(contr_temp_min);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //pos 'temp ok' window limit
    case 0x2009:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_temp_ok_pos);
        }
        //set
        else if (read16(data) >= 0 && read16(data) <= 1000 && data_length == 2)
        {
            PushBinary16(cmd);      //function
            
            if (contr_temp_ok_pos != read16(data))
            {
                contr_temp_ok_pos = read16(data);
                eeprom_write_word(&contr_temp_ok_pos_eeprom,read16(data));
            }

            PushBinary16(contr_temp_ok_pos);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //neg 'temp ok' window limit
    case 0x2010:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_temp_ok_neg);
        }
        //set
        else if (read16(data) >= 0 && read16(data) <= 1000 && data_length == 2)
        {
            PushBinary16(cmd);      //function
            
            if (contr_temp_ok_neg != read16(data))
            {
                contr_temp_ok_neg = read16(data);
                eeprom_write_word(&contr_temp_ok_neg_eeprom,read16(data));
            }

            PushBinary16(contr_temp_ok_neg);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //use temp set input
    case 0x2011:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary(contr_temp_set_input_use);
        }
        //set
        else if (data[0] >= 0 && data[0] <= 1 && data_length == 1)
        {
            PushBinary16(cmd);      //function

            if (contr_temp_set_input_use != data[0])
            {
                contr_temp_set_input_use = data[0];
                eeprom_write_byte(&contr_temp_set_input_use_eeprom, data[0]);
            }

            if (contr_temp_set_input_use == 0)
            {
                contr_temp_set = eeprom_read_word(&contr_temp_set_eeprom);
            }
            
            PushBinary(contr_temp_set_input_use);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }

        break;

    //max temp delta sensor 1 and 2
    case 0x2012:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_temp_delta_max);
        }
        //set
        else if (read16(data) >= 0 && read16(data) <= 15000 && data_length == 2)
        {
            PushBinary16(cmd);      //function
            
            if (contr_temp_delta_max != read16(data))
            {
                contr_temp_delta_max = read16(data);
                eeprom_write_word(&contr_temp_delta_max_eeprom,read16(data));
            }

            PushBinary16(contr_temp_delta_max);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //absolute max temp sensor 1
    case 0x2013:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_temp_1_max);
        }
        //set
        else if (read16(data) >= 0 && read16(data) <= 15000 && data_length == 2)
        {
            PushBinary16(cmd);      //function
            
            if (contr_temp_1_max != read16(data))
            {
                contr_temp_1_max = read16(data);
                eeprom_write_word(&contr_temp_1_max_eeprom,read16(data));
            }

            PushBinary16(contr_temp_1_max);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //sensor for temp control
    case 0x2014:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary(contr_temp_use);
        }
        //set
        else if (data[0] >= 1 && data[0] <= 3 && data_length == 1)
        {
            PushBinary16(cmd);      //function

            if (contr_temp_use != data[0])
            {
                contr_temp_use = data[0];
                eeprom_write_byte(&contr_temp_use_eeprom, data[0]);
            }
            
            PushBinary(contr_temp_use);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }

        break;

    //controller proportional factor
    case 0x2015:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(controller_p_factor);
        }
        //set
        else if (read16(data) >= 0 && read16(data) <= 1000 && data_length == 2)
        {
            PushBinary16(cmd);      //function
            
            if (controller_p_factor != read16(data))
            {
                controller_p_factor = read16(data);
                eeprom_write_word(&controller_p_factor_eeprom,read16(data));
            }

            PushBinary16(controller_p_factor);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //controller integral factor
    case 0x2016:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(controller_i_factor);
        }
        //set
        else if (read16(data) >= 0 && read16(data) <= 1000 && data_length == 2)
        {
            PushBinary16(cmd);      //function
            
            if (controller_i_factor != read16(data))
            {
                controller_i_factor = read16(data);
                eeprom_write_word(&controller_i_factor_eeprom,read16(data));
            }

            PushBinary16(controller_i_factor);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //absolute max temp sensor 2
    case 0x2017:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_temp_2_max);
        }
        //set
        else if (read16(data) >= 0 && read16(data) <= 15000 && data_length == 2)
        {
            PushBinary16(cmd);      //function
            
            if (contr_temp_2_max != read16(data))
            {
                contr_temp_2_max = read16(data);
                eeprom_write_word(&contr_temp_2_max_eeprom,read16(data));
            }

            PushBinary16(contr_temp_2_max);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //absolute max temp sensor 3
    case 0x2018:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_temp_3_max);
        }
        //set
        else if (read16(data) >= 0 && read16(data) <= 15000 && data_length == 2)
        {
            PushBinary16(cmd);      //function
            
            if (contr_temp_3_max != read16(data))
            {
                contr_temp_3_max = read16(data);
                eeprom_write_word(&contr_temp_3_max_eeprom,read16(data));
            }

            PushBinary16(contr_temp_3_max);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //value sensor 1
    case 0x2020:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_temp_1);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //value sensor 2
    case 0x2021:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_temp_2);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //value sensor 3
    case 0x2022:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_temp_3);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //status sensor 1
    case 0x2023:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary(temp_1_fail);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //status sensor 2
    case 0x2024:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary(temp_2_fail);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //status sensor 3
    case 0x2025:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary(temp_3_fail);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //status 'temp ok'
    case 0x2026:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary(conv_temp_ok);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //set temperatur ramp
    case 0x2027:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_temp_set);
        }
        //set
        else if (read16(data) >= 0 && read16(data) <= 1000 && data_length == 2)
        {
            PushBinary16(cmd);      //function

            if (contr_temp_ramp != read16(data))
            {
                contr_temp_ramp = read16(data);
                eeprom_write_word(&contr_temp_ramp_eeprom,read16(data));
            }

            PushBinary16(contr_temp_ramp);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //positive current limit
    case 0x2101:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(conv_current_pos_max);
        }
        //set
        else if (read16(data) >= 0 && read16(data) <= HW_CURRENT_LIMIT && data_length == 2)
        {
            PushBinary16(cmd);      //function
            
            if (conv_current_pos_max != read16(data))
            {
                conv_current_pos_max = read16(data);
                eeprom_write_word(&conv_current_pos_max_eeprom,read16(data));
            }

            PushBinary16(conv_current_pos_max);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //negative current limit
    case 0x2102:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(-conv_current_neg_max);
        }
        //set
        else if (read16(data) >= 0 && read16(data) <= HW_CURRENT_LIMIT && data_length == 2)
        {
            PushBinary16(cmd);      //function
            
            if (conv_current_neg_max != read16(data))
            {
                conv_current_neg_max = -read16(data);
                eeprom_write_word(&conv_current_neg_max_eeprom,-read16(data));
            }

            PushBinary16(-conv_current_neg_max);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //value current
    case 0x2103:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(conv_current);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //value voltage driver A
    case 0x2104:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(conv_voltage_heat);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //value voltage driver B
    case 0x2105:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(conv_voltage_cool);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //status tec module
    case 0x2106:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary(conv_tec_err);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //fan control sensor
    case 0x2201:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary(fan_contr_sensor);
        }
        //set
        else if (data[0] >= 0 && data[0] <= 4 && data_length == 1)
        {
            PushBinary16(cmd);      //function

            if (fan_contr_sensor != data[0])
            {
                fan_contr_sensor = data[0];
                eeprom_write_byte(&fan_contr_sensor_eeprom, data[0]);
            }
            
            PushBinary(fan_contr_sensor);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //fan control environmental temperature
    case 0x2202:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(fan_contr_env_temp);
        }
        //set
        else if (read16(data) >= -5000 && read16(data) <= 15000 && data_length == 2)
        {
            PushBinary16(cmd);      //function
            
            if (fan_contr_env_temp != read16(data))
            {
                fan_contr_env_temp = read16(data);
                eeprom_write_word(&fan_contr_env_temp_eeprom,read16(data));
            }

            PushBinary16(fan_contr_env_temp);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //fan control temperature delta
    case 0x2203:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(fan_contr_temp_delta);
        }
        //set
        else if (read16(data) >= 0 && read16(data) <= 15000 && data_length == 2)
        {
            PushBinary16(cmd);      //function
            
            if (fan_contr_temp_delta != read16(data))
            {
                fan_contr_temp_delta = read16(data);
                eeprom_write_word(&fan_contr_temp_delta_eeprom,read16(data));
            }

            PushBinary16(fan_contr_temp_delta);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //fan control max value
    case 0x2204:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary(fan_power_set);
        }
        //set
        else if (data[0] >= 0 && data[0] <= 100 && data_length == 1)
        {
            PushBinary16(cmd);      //function

            //10% steps
            fan_power_set = (data[0]+5) /10;
            fan_power_set *= 10;
            
            eeprom_write_byte(&fan_power_set_eeprom,fan_power_set);

            PushBinary(fan_power_set);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //set Aux I/O 1 use
    case 0x2205:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary(contr_aux_1);
        }
        //set
        else if (data[0] >= 0 && data[0] <= 3 && data_length == 1)
        {
            PushBinary16(cmd);      //function

            contr_aux_1 = data[0];

            PushBinary(contr_aux_1);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //set Aux I/O 2 use
    case 0x2206:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary(contr_aux_2);
        }
        //set
        else if (data[0] >= 0 && data[0] <= 3 && data_length == 1)
        {
            PushBinary16(cmd);      //function

            contr_aux_2 = data[0];

            PushBinary(contr_aux_2);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //set Aux I/O 3 use
    case 0x2207:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary(contr_aux_3);
        }
        //set
        else if (data[0] >= 0 && data[0] <= 3 && data_length == 1)
        {
            PushBinary16(cmd);      //function

            contr_aux_3 = data[0];

            PushBinary(contr_aux_3);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //temp 1 pwm out max value
    case 0x2208:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_temp_out_max[0]);
        }
        //set
        else if (read16(data) >= contr_temp_out_min[0]+100 && read16(data) <= 15000 && data_length == 2)
        {
            PushBinary16(cmd);      //function
            
            if (contr_temp_out_max[0] != read16(data))
            {
                contr_temp_out_max[0] = read16(data);
                eeprom_write_word(&contr_temp_out_max_eeprom[0],read16(data));
            }

            PushBinary16(contr_temp_out_max[0]);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //temp 2 pwm out max value
    case 0x2209:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_temp_out_max[1]);
        }
        //set
        else if (read16(data) >= contr_temp_out_min[1]+100 && read16(data) <= 15000 && data_length == 2)
        {
            PushBinary16(cmd);      //function
            
            if (contr_temp_out_max[1] != read16(data))
            {
                contr_temp_out_max[1] = read16(data);
                eeprom_write_word(&contr_temp_out_max_eeprom[1],read16(data));
            }

            PushBinary16(contr_temp_out_max[1]);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //temp 3 pwm out max value
    case 0x2210:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_temp_out_max[2]);
        }
        //set
        else if (read16(data) >= contr_temp_out_min[2]+100 && read16(data) <= 15000 && data_length == 2)
        {
            PushBinary16(cmd);      //function
            
            if (contr_temp_out_max[2] != read16(data))
            {
                contr_temp_out_max[2] = read16(data);
                eeprom_write_word(&contr_temp_out_max_eeprom[2],read16(data));
            }

            PushBinary16(contr_temp_out_max[2]);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //temp 1 pwm out min value
    case 0x2211:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_temp_out_min[0]);
        }
        //set
        else if (read16(data) >= -5000 && read16(data) <= contr_temp_out_max[0]-100 && data_length == 2)
        {
            PushBinary16(cmd);      //function
            
            if (contr_temp_out_min[0] != read16(data))
            {
                contr_temp_out_min[0] = read16(data);
                eeprom_write_word(&contr_temp_out_min_eeprom[0],read16(data));
            }

            PushBinary16(contr_temp_out_min[0]);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //temp 2 pwm out min value
    case 0x2212:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_temp_out_min[1]);
        }
        //set
        else if (read16(data) >= -5000 && read16(data) <= contr_temp_out_max[1]-100 && data_length == 2)
        {
            PushBinary16(cmd);      //function
            
            if (contr_temp_out_min[1] != read16(data))
            {
                contr_temp_out_min[1] = read16(data);
                eeprom_write_word(&contr_temp_out_min_eeprom[1],read16(data));
            }

            PushBinary16(contr_temp_out_min[1]);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //temp 3 pwm out min value
    case 0x2213:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(contr_temp_out_min[2]);
        }
        //set
        else if (read16(data) >= -5000 && read16(data) <= contr_temp_out_max[2]-100 && data_length == 2)
        {
            PushBinary16(cmd);      //function
            
            if (contr_temp_out_min[2] != read16(data))
            {
                contr_temp_out_min[2] = read16(data);
                eeprom_write_word(&contr_temp_out_min_eeprom[2],read16(data));
            }

            PushBinary16(contr_temp_out_min[2]);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //firmware version (code)
    case 0x9001:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(FIRMWARE_CODE);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //firmware version (text)
    case 0x9002:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinaryString(FIRMWARE_VERSION);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //slave type (text)
    case 0x9003:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinaryString(CONTROLLER_TYPE);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //receiver id
    case 0xAA01:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary(receiver_id);
        }
        //set
        else if (data[0] >= 1 && data[0] <= 99 && data_length == 1)
        {
            PushBinary16(cmd);      //function

            receiver_id = data[0];
            eeprom_write_byte(&receiver_id_eeprom, data[0]);

            PushBinary(receiver_id);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }

        break;

    //manual power
    case 0xAA02:

        //get
        if (data_length == 0)
        {
            PushBinary16(cmd);      //function
            PushBinary16(conv_power_override);
        }
        //set
        else if (read16(data) >= -4095 && read16(data) <= 4095 && data_length == 2)
        {
            PushBinary16(cmd);      //function

            conv_power_override = read16(data);

            PushBinary16(conv_power_override);

            if (conv_enabled)
            {
                conv_enabled = 0;
            }
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }
        break;

    //reset
    case 0xAAFF:

        //set
        if (data[0] >= 0 && data[0] <= 1 && data_length == 1)
        {
            PushBinary16(cmd);      //function

            conv_enabled = 0;
            conv_power_override = 0;
            contr_temp_integral = 0;
            conv_power = 0;

            conv_heat_power_limit = 0;
            conv_cool_power_limit = 0;

            PushBinary16(data[0]);
            TransmitBinary();       //send

            //load factory defaults
            if (data[0] == 1)
            {
                eeprom_write_word(&firmware_code_eeprom, 0xFFFF);
            }

            wdt_enable(WDTO_15MS);
            while(1);
        }
        else
        {
            PushBinary16(0xFFA2);    //data error
        }

        break;

    default:
        PushBinary16(0xFFA1);    //function error
    }
}



// -------------------------------------------------------------
// Alle Werte auf Interface ausgeben (Binär)
// -------------------------------------------------------------

void PushAllValues()
{
    PushBinary(1);          //all values
    PushBinary(255);        //all values

    //Status
    PushBinary(conv_enabled);


    //Temp set
    if (contr_temp_set > 0) PushBinary16((contr_temp_set+5)/10);
    else PushBinary16((contr_temp_set-5)/10);
	
    //Temp sensor 1 fail
    PushBinary(temp_1_fail);

    //Temp 1
    if (contr_temp_1 > 0) PushBinary16((contr_temp_1+5)/10);
    else PushBinary16((contr_temp_1-5)/10);
	
    //Temp sensor 2 fail
    PushBinary(temp_2_fail);

    //Temp 2
    if (contr_temp_2 > 0) PushBinary16((contr_temp_2+5)/10);
    else PushBinary16((contr_temp_2-5)/10);
	
    //Temp sensor 3 fail
    PushBinary(temp_3_fail);

    //Temp 3
    if (contr_temp_3 > 0) PushBinary16((contr_temp_3+5)/10);
    else PushBinary16((contr_temp_3-5)/10);

    //Current
    PushBinary16(conv_current);

    //Voltage cool
    PushBinary16(conv_voltage_cool);

    //Voltage heat
    PushBinary16(conv_voltage_heat);

/*

    //LEDs
    TransmitString(",");
    TransmitInt(read_bit_n(LED_HEAT), 1);
    TransmitString(",");
    TransmitInt(read_bit_n(LED_COOL), 1);
    TransmitString(",");
    TransmitInt(read_bit_n(LED_TEMP_OK), 1);
    TransmitString(",");
    TransmitInt(read_bit_n(LED_TEC_FAIL), 1);
    TransmitString(",");
    TransmitInt(read_bit_n(LED_SENSOR_FAIL), 1);

    //Temp high
    TransmitString(",");
    TransmitFloat(contr_temp_high, 1, 2);

    //Temp low
    TransmitString(",");
    TransmitFloat(contr_temp_low, 1, 2);

    //Temp error
    TransmitString(",");
    TransmitInt(conv_temp_err, 1);


    //end
    TransmitStringLn("");

    
#if DEBUG_ENABLE
    debug_send = 1;
#endif

    if(debug_send)
    {
        TransmitString("DBG=");

        //Converter
        TransmitString("CONV: ");
        TransmitInt(conv_power, 4);
        TransmitString(" / ");

        TransmitInt(OCR3B, 4);

        TransmitString(",");
        TransmitInt(OCR3C, 4);

        TransmitString("     P:");
        TransmitInt(contr_temp_proportion, 5);

        TransmitString(" I:");
        TransmitInt(contr_temp_integral, 5);

        TransmitString(",");
        TransmitInt(contr_temp_delta_counter, 3);

        //Temp
        TransmitString("     TMP: ");
        TransmitInt(adc_gemittelt[ADC_PTC_1], 4);

        TransmitString(",");
        TransmitInt(adc_gemittelt[ADC_PTC_2], 4);

        TransmitString(",");
        TransmitInt(0, 4);

        TransmitStringLn("");
    }*/
}



// -------------------------------------------------------------
// Binär-Telegram initialisieren
// -------------------------------------------------------------

void InitBinary(uint8_t dest_id)
{
    binary_frame[0] = 0x7E;
    binary_frame[1] = dest_id;
    binary_frame[2] = receiver_id;
    frame_length = 4;
}



// -------------------------------------------------------------
// Daten in Binär-Telegram schreiben
// -------------------------------------------------------------

void PushBinary(uint8_t data)
{
    binary_frame[frame_length++] = data;
}



// -------------------------------------------------------------
// Daten in Binär-Telegram schreiben
// -------------------------------------------------------------

void PushBinary16(uint16_t data)
{
    binary_frame[frame_length++] = data>>8;
    binary_frame[frame_length++] = data;
}



// -------------------------------------------------------------
// String in Binär-Telegram schreiben
// -------------------------------------------------------------

void PushBinaryString(char *str)
{
    uint8_t i = 0;

    while (str[i])
    {
        binary_frame[frame_length++] = str[i];

        i++;
    }
}



// -------------------------------------------------------------
// Binär-Telegram auf Interface ausgeben
// -------------------------------------------------------------

void TransmitBinary()
{
    //set frame lenght
    binary_frame[3] = frame_length+1;

    //calculate CRC16 checksum
    uint16_t crc = CalcCRC16(&binary_frame[1], frame_length-1);

    //append CRC16 checksum
    binary_frame[frame_length++] = crc;
    binary_frame[frame_length++] = crc>>8;

    //append 0x7E
    binary_frame[frame_length++] = 0x7E;

    if (interface == INT_USB)
    {
        UART0TransmitBytes((char*) binary_frame, frame_length);
    }

    if (interface == INT_RS485)
    {
        UART1TransmitBytes((char*) binary_frame, frame_length);
    }
}



// -------------------------------------------------------------
// CRC berechnen
// -------------------------------------------------------------

uint16_t CalcCRC16(uint8_t *pointer, uint8_t len)
{
    uint16_t crc = 0xffff;
    uint8_t i,j;

    for( i = 0; i < len; i++ )
    {
        crc ^= (uint8_t) pointer[i];

        for( j = 8; j; j-- )
        {
            crc = (crc >> 1) ^ ((crc & 1) ? 0xA001 : 0 );
        }
    }

    return crc;
}



// -------------------------------------------------------------
// 16bit von Adresse lesen
// -------------------------------------------------------------

inline int16_t read16(char *data)
{
    return data[0]<<8 | data[1];
}
