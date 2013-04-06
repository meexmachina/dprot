#ifndef __SLIP_H__
#define __SLIP_H__

#include "spec_types.h"

/*! \file slip.h
 * \brief Data-link layer
 *
 * This layer frames the information and operates over the
 * hardware buffer streams of serial information. The framing
 * is conducted using the SLIP protocol.
 *
 * Message structure:
 *
 *	| N bytes of information (stuffed accordint to the protocol) | END byte |
 *	|------------------------------------------------------------|----------|
 *  | N bytes                                                    |   8 bit  |
 *
 */


#define	SLIP_END		192 	/**< END byte (0xc0) - message ending */
#define	SLIP_DATA_END	220		/**< D_END byte (0xdc) - the pair ESC+DATA_END = END */
#define	SLIP_DATA_ESC	221 	/**< D_ESC byte (0xdd) - stuffing ASC+DATA_ESC = ESC */
#define	SLIP_ESC		219 	/**< ESC byte (0xdb) - stuffing before middle END/ESC */
#define SLIP_RX_TIMEOUT 50      /**< number of milliseconds to wait for a single byte rx */

/*********************************************************/
/*! slip message sending stages
 * Different types of message types send by the dProt
 * network layer.
 */
enum
{
	SLIP_MSG_MIDDLE = 0x00,     /**< send as the middle of a continuing message */
	SLIP_MSG_START = 0x01,      /**< send the header (which doesn't really exist - only a single END to flush receiver's buffers */
	SLIP_MSG_END = 0x02,        /**< send the ending of the message */
	SLIP_MSG_REG = 0x03         /**< send everything as a single whole message */
};


/*********************************************************/
/*! \struct slip_channel
 * This structure defines the physical channel as 3 kinds
 * of functions - sending a byte, receiving a byte (blocking)
 * and receiving a byte (timeouted).
 * The required functions for most applications are 
 * 'slip_put_char' and one of the receiving.
 */
typedef struct
{
    fn_put_char slip_put_char;
    fn_get_char slip_get_char;
    fn_get_char_to slip_get_char_to;
} slip_channel;

/***********************************************************/

/*!
 * \brief initialize slip datalink layer
 *
 * \param put_function the putchar function
 * \param get_function the getchar (blocking) function
 * \param get_function_to the getchar (non-blocking) function
 * \param ch a preallocated 'slip_channel' structure to contain the channel information
 
 * \return result - success(0), failure (otherwise)
 */
uint8_t slip_init(fn_put_char put_function,
                  fn_get_char get_function,
                  fn_get_char_to get_function_to,
                  slip_channel* ch);


/*!
 * \brief receive data from the channel
 *
 * \param ch pre-initialized (with 'slip_init') channel to read from
 * \param buffer preallocated sufficiently big buffer to contain the data
 * \param len the maximal size of the buffer
 *
 * \return the amount of data read before framing (or timeout) occured
 */
uint8_t slip_rx(slip_channel* ch, uint8_t* buffer, uint8_t len);

/*!
 * \brief Send data to the channel
 *
 * \param ch pre-initialized (with 'slip_init') channel to read from
 * \param buffer contains the data to be sent
 * \param len the amount of data to be sent from the 'buffer'
 * \param start_end the stage of sending a frame
 *
 * \return the amount of data actually sent
 */
uint8_t slip_tx(slip_channel* ch, uint8_t* buffer, uint8_t len, uint8_t start_end);

#endif //__SLIP_H__

