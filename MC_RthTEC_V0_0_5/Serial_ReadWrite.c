/*
 * Serial_ReadWrite.c
 *
 * Created: 23.03.2020 10:08:14
 *  Author: schmidm
 */ 


#include "Serial_ReadWrite.h"

// -------------------------------------------------------------
// Zeile als Zahl lesen
// -------------------------------------------------------------

uint8_t ParseIntLn(char *string, uint8_t digits, int16_t *num)
{
    uint8_t neg = 0;
    uint8_t i = 0;
    uint8_t temp[6];

    *num = 0;

    //negative number
    if (*string == '-')
    {
        neg = 1;
        string++;
    }

	while ( (temp[i] = (*(string++) - '0')) < 10)
    {
        i++;
    }

    if (*(string-1) == '\n' && i <= digits)
    {
        for (uint8_t j = i;j > 0; j--)
        {
            *num *= 10;
            *num += temp[i-j];
        }

        if (neg)
        {
            *num = - *num;
        }

        return i;
    }

    return 0;
}

// -------------------------------------------------------------
// Zeile als Zahl lesen
// -------------------------------------------------------------

uint8_t ParseLongLn(char *string, uint8_t digits, int32_t *num)
{
    uint8_t neg = 0;
    uint8_t i = 0;
    uint8_t temp[11];

    *num = 0;

    //negative number
    if (*string == '-')
    {
        neg = 1;
        string++;
    }

	while ( (temp[i] = (*(string++) - '0')) < 10)
    {
        i++;
    }

    if (*(string-1) == '\n' && i <= digits)
    {
        for (uint8_t j = i;j > 0; j--)
        {
            *num *= 10;
            *num += temp[i-j];
        }

        if (neg)
        {
            *num = - *num;
        }

        return i;
    }

    return 0;
}

// -------------------------------------------------------------
// String als Boolean lesen
// -------------------------------------------------------------

uint8_t ParseBool(char *string, uint8_t *value)
{
    if (*string == '0')
    {
        *value = 0;

        return 1;
    }

    else if (*string == '1')
    {
        *value = 1;

        return 1;
    }

    return 0;
}

// -------------------------------------------------------------
// String als Byte lesen
// -------------------------------------------------------------

uint8_t ParseByte(char *string, uint8_t *value)
{
	uint8_t temp = 0;
	for(int i = 0; i < 8 ; i++)
	{
		if(string[i] - '0' == 0)
		{
			//Nix zu tun
		}
		else if(string[i] - '0' == 1)
		{
			temp = temp + (1<<i);
		}
		else
		{
			return 0;
		}
	}
	
	*value = temp;
	return 1;
	
}

/*
uint8_t ParseByte(char *string, uint8_t *value)
{
	uint8_t temp[10];
	uint8_t bit = 0;
	uint8_t i = 0;
	uint8_t j = 0;
	
	//Write in Tempo Array
	while ( (temp[i] = (*(string++) - '0')) < 10 && i < 9)
    {
        i++;
    }
	i--;
	
	if(i != 7)
	{
		return 0;
	}
	
	while (j <= i) 
	{
		if (temp[j]  == 0)
		{
			 bit = 0;
		}
		
		else if (temp[j]  == 1)
		{
			bit = 1;
		}
		else
		{
			return 0;
		}
		
		*value += bit << (i-j);
		j++;
	}
	
	return 1;
}
*/

// -------------------------------------------------------------
// String auf Interface ausgeben
// -------------------------------------------------------------

void TransmitString(char *string)
{
    UART0TransmitString(string);
}

// -------------------------------------------------------------
// String & CRLF auf Interface ausgeben
// -------------------------------------------------------------

void TransmitStringLn(char *string)
{
    UART0TransmitStringLn(string);
}

// -------------------------------------------------------------
// Zahl dezimal auf Interface ausgeben (ASCII)
// -------------------------------------------------------------

void TransmitInt(int16_t i, uint8_t digits)
{
	char str[6], temp[6];

    // minus sign
	if(i < 0)
	{
		strcpy(temp,"-");
        i = -i;
	}
	else
	{
		strcpy(temp,"");
	}

	itoa(i,str,10);
	
	strcat(temp,str);
    strcpy(str,temp);


    //format with leading spaces
	if(digits > 0)
	{
		while(strlen(str) < digits)
		{
			strcpy(temp," ");
			strcat(temp,str);
			strcpy(str,temp);
		}
	}
    

    //send
	TransmitString(str);
}

// -------------------------------------------------------------
// Zahl dezimal auf Interface ausgeben (ASCII)
// -------------------------------------------------------------

void TransmitInt0(int16_t i, uint8_t digits)
{
	char str[6], temp[6];
    uint8_t neg = 0;

    // minus sign
	if(i < 0)
	{
        neg = 1;
        i = -i;

        if(digits) digits--;
	}

	itoa(i,str,10);


    //format with leading zeros
	if(digits)
	{
		while(strlen(str) < digits)
		{
			strcpy(temp,"0");
			strcat(temp,str);
			strcpy(str,temp);
		}
	}

    // minus sign
	if(neg)
	{
		strcpy(temp,"-");
		strcat(temp,str);
		strcpy(str,temp);
	}
    

    //send
	TransmitString(str);
}

// -------------------------------------------------------------
// Zahl dezimal auf Interface ausgeben (ASCII)
// -------------------------------------------------------------

void TransmitLong(int32_t i, uint8_t digits)
{
	char str[11], temp[11];

    // minus sign
	if(i < 0)
	{
		strcpy(temp,"-");
        i = -i;
	}
	else
	{
		strcpy(temp,"");
	}

	ltoa(i,str,10);
	
	strcat(temp,str);
    strcpy(str,temp);


    //format with leading spaces
	if(digits > 0)
	{
		while(strlen(str) < digits)
		{
			strcpy(temp," ");
			strcat(temp,str);
			strcpy(str,temp);
		}
	}
    

    //send
	TransmitString(str);
}

// -------------------------------------------------------------
// Zahl als Dezimalbruch auf Interface ausgeben (ASCII)
// -------------------------------------------------------------

void TransmitFloat(int16_t i, uint8_t digits, uint8_t div)
{
	char str[10], temp[10];
	uint16_t pot = 1;
	uint8_t co = div;

	while(div > 0)
	{
		pot *= 10;
		div--;
	}

	itoa(abs(i)%pot,str,10);

	while(strlen(str) < co)
	{
		strcpy(temp,"0");
		strcat(temp,str);
		strcpy(str,temp);
	}

	itoa(abs(i)/pot,temp,10);

	strcat(temp,".");
	strcat(temp,str);


    // minus sign
	if(i < 0)
	{
		strcpy(str,"-");
	}
	else
	{
		strcpy(str,"");
	}
	
	strcat(str,temp);


    //format with leading spaces
	if(digits > 0)
	{
		while(strlen(str) < digits)
		{
			strcpy(temp," ");
			strcat(temp,str);
			strcpy(str,temp);
		}
	}
    

    //send
	TransmitString(str);
}

// -------------------------------------------------------------
// Zahl als Dezimalbruch auf Interface ausgeben (ASCII)
// -------------------------------------------------------------

void TransmitByte(uint8_t byte)
{
	char str[] = "00000000";
	
	for(int i = 7; i >= 0; i--)
	{
		if(byte & (1 << i)){
			str[7-i] = '1';
		}
		else{
			str[7-i] = '0';		
		}
	}
	

	//send
	TransmitString(str);
}

void TransmitByte_Reverse(uint8_t byte)
{
	char str[] = "00000000";
	
	for(int i = 0; i < 8; i++)
	{
		if(byte & (1 << i)){
			str[i] = '1';
		}
		else{
			str[i] = '0';
		}
	}
	

	//send
	TransmitString(str);
}