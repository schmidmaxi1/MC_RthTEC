/* ----------------------------------------------------------
 *
 * Helper functions
 *
 * Author:  Johannes Knauss 2017, mail@iet-chiemsee.de
 *
 *
 * ----------------------------------------------------------
 */



#ifndef _HELPER_FUNCS_
#define _HELPER_FUNCS_



/*
 ** Compiler macros
 */

#define _BV32(bit)		        ((uint32_t) 1 << bit)

#define _MK16(a,b)              (((uint16_t)b)<<8 | a)
#define _MK32(a,b,c,d)          (((uint32_t)'d')<<24 | ((uint32_t)'c')<<16 | 'b'<<8 | 'a')

#define cast16                  *(uint16_t*)&
#define cast32                  *(uint32_t*)&

#define STRINGIZE(s)            #s
#define S(s)                    STRINGIZE(s)


#define _set_in(port,pin)		DDR(port) &= ~_BV(pin)
#define _set_out(port,pin)		DDR(port) |=  _BV(pin)

#define _set_bit(byte,bit)		byte |=  _BV(bit)
#define _clear_bit(byte,bit)    byte &= ~_BV(bit)
#define _toggle_bit(byte,bit)	byte ^= _BV(bit)

#define _read_bit(port,pin)		((PIN(port) & _BV(pin))>>pin)
#define _read_bit_n(port,pin)	(((PIN(port) & _BV(pin))>>pin) ^1)


#define set_in(id)		        _set_in(id)
#define set_out(id)		        _set_out(id)

#define set_bit(id)		        _set_bit(id)
#define clear_bit(id)		    _clear_bit(id)
#define toggle_bit(id)	        _toggle_bit(id)


#define is_set(byte,bit)		((byte & _BV(bit))>>bit)
#define is_clear(byte,bit)		(((byte & _BV(bit))>>bit) ^1)

#define read_bit(id)		    _read_bit(id)
#define read_bit_n(id)	        _read_bit_n(id)

#define DDR(port)               (*(&port - 1))

#if defined(__AVR_ATmega64__) || defined(__AVR_ATmega128__)
    #define PIN(x)              ( &PORTF==&(x) ? _SFR_IO8(0x00) : (*(&x - 2)) ) //on ATmega64/ATmega128 PINF is on port 0x00
#else
    #define PIN(x) (*(&x - 2))
#endif



#endif
