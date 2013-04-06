#ifndef __SPEC_TYPES_H__
#define __SPEC_TYPES_H__

#ifdef __AVR__
#include <avr/io.h> 
#endif
#include <stdio.h>

#ifndef __AVR__
	typedef unsigned char uint8_t;
	typedef unsigned short uint16_t;
	typedef unsigned int uint32_t;
//	typedef char int8_t;
	typedef short int16_t;
	typedef int int32_t;
#endif


/***********************************************************/

/*! \typedef fn_put_char
 * this pointer to function should send a single character 
 * and returns immediatelly
 */
typedef void (*fn_put_char)(uint8_t c);


/*! \typedef fn_get_char
 * this pointer to function should get a single character
 * from the input. This call is blocking - the function 
 * will wait until a single character will be available in 
 * the input stream.
 */
typedef uint8_t (*fn_get_char)(void);


/*! \typedef fn_get_char
 * this pointer to function should get a single character
 * from the input. This call is blocking UNTIL it reaches 
 * the timeout [ms] specified in 'to'. The returned value 
 * is 1 (got a single char) or 0 (exited on timeout). 
 * The incoming character is written in 'cout'
 */
typedef uint8_t (*fn_get_char_to)(uint8_t to, uint8_t* cout);


#endif //__SPEC_TYPES_H__

