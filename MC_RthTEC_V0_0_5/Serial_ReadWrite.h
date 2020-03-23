/*
 * Serial_ReadWrite.h
 *
 * Created: 23.03.2020 10:08:50
 *  Author: schmidm
 */ 


#ifndef SERIAL_READWRITE_H_
#define SERIAL_READWRITE_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uart0.h"
#include "helper.h"



void TransmitAll();


void TransmitString(char *string);
void TransmitStringLn(char *string);
void TransmitInt(int16_t i, uint8_t digits);
void TransmitLong(int32_t i, uint8_t digits);
void TransmitInt0(int16_t i, uint8_t digits);
void TransmitFloat(int16_t i, uint8_t digits, uint8_t div);
void TransmitByte(uint8_t byte);
void TransmitByte_Reverse(uint8_t byte);

extern uint8_t ParseIntLn(char *string, uint8_t digits, int16_t *num);
uint8_t ParseIntLn(char *string, uint8_t digits, int16_t *num);
uint8_t ParseLongLn(char *string, uint8_t digits, int32_t *num);
uint8_t ParseBool(char *string, uint8_t *value);
uint8_t ParseByte(char *string, uint8_t *value);






#endif /* SERIAL_READWRITE_H_ */