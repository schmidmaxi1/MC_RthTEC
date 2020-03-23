/*
 * Slot_Tester.h
 *
 * Created: 30.04.2019 12:31:34
 *  Author: schmidm
 */ 


/*
 ** Compiler-Konstanten
 */

#define Segment_0	0b00000011
#define Segment_1	0b11001111
#define Segment_2	0b00100101
#define Segment_3	0b10000101
#define Segment_4	0b11001001
#define Segment_5	0b10010001
#define Segment_6	0b00010001
#define Segment_7	0b11000111
#define Segment_8	0b00000001
#define Segment_9	0b10000001



/*
 ** Funktionen
 */

void Slot_Tester_Init(int slot_nr);
void Slot_Tester_Default_Values(int slot_nr);

void Slot_Tester_Gesamtablauf(int slot_nr);

void Slot_Tester_set_7Segment(int slot_nr);
int Slot_Tester_get_7Segment(int slot_nr);
void Slot_Tester_IO_Lauflicht(int slot_nr);