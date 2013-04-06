#ifndef __CHECKING_H__
#define __CHECKING_H__

#include "spec_types.h"

/****************************************************/
extern uint8_t crc8_table[256]; /*< 8-bit crc table */


/*! \def crc8_add_byte
 * \brief update macro for crc8 type of checking
 */
#define crc8_add_byte(c,d)		(c)=((crc8_table[(c) ^ (d)]) & 0xff)

/*! \def chs8_add_byte
 * \brief update macro for checksum-8 type of checking
 */
#define chs8_add_byte(c,d)		(c)=(((c)+(d))&0xff)

/*! \def xor8_add_byte
 * \brief update macro for xor8 type of checking
 */
#define xor8_add_byte(c,d)		(c)^=(d)

/*!
 * Different checking typed available
 */
typedef enum
{
	CHECKING_CRC8 = 0,  /**< 8bit crc calculation */
	CHECKING_CHS8 = 1,  /**< 8bit checksum - simply addition of all bytes */
	CHECKING_XOR8 = 2   /**< 8bit xoring of each byte in message */
} CHECKING_TYPE;

/*!
 * \brief init the crc8 table - needed before any usage of the crc generation
 */
void init_crc8();

#endif //__CHECKING_H__

