#ifndef __DPROT_H__
#define __DPROT_H__

#include "spec_types.h"
#include "slip.h"
#include "checking.h"

/*! \file dprot.h
 * \brief Transport layer
 *
 * Its in charge of the error checking, ack/nack-ing, sync
 * and logical checkings on the trasmitter and receiver.
 *
 * Message structure:
 *
 *	| 7bit |1bit | 8bit | 'length'-bytes    |  8bit   |
 *	|------|-----|------|-------------------|---------|
 *  | type | seq |length|   d a t a ...     |checking8|
 *
 * The maximal message length (type+seq+length+data+check)
 * is 256 bytes. Longer messages have to be framed in higher
 * layer. The checking8 byte is one of 'crc8'/'chs8'/
 * /'xor8' checksum methods (you choose) of 'type'+'length'
 * +'data'.
 * The sequencial parity will keep a track on the
 * 'sequencial number' of single bit long.
 */

/*********************************************************/

/*! \def DPROT_MAX_MSG
 * \brief Defines that maximal allowed message size
 */
#define DPROT_MAX_MSG				256

/*! \def DPROT_PTOT_SIZE
 * \brief Total bytes taked by the protocol for header and footer of messages
 */
#define DPROT_PTOT_SIZE				3

/*! \def DPROT_MAX_PAYLOAD
 * \brief The total neto size of a payload
 */
#define DPROT_MAX_PAYLOAD			(DPROT_MAX_MSG-DPROT_PTOT_SIZE)

/*! \def DPROT_CHECKING
 * \brief The checking algorithm used by dProt
 */
#define DPROT_CHECKING(c,d)			crc8_add_byte((c),(d))

/*! \def DPROT_MASTER_NUM_RETRIES
 * \brief The number of retries on communication
 */
#define DPROT_MASTER_NUM_RETRIES	5




/*********************************************************/
/*! dProt protocol message types enumeration
 * Different types of message types send by the dProt
 * network layer.
 */
enum
{
	DPROT_TYPE_DATA = 0x10, /**< Data message */
	DROPT_TYPE_ARP = 0x20,  /**< ARP like 'ping' message */
	DPROT_TYPE_ACK = 0x30,  /**< Acknowledge signs that a message was accepted by the slave */
	DPROT_TYPE_NACK = 0x40, /**< Not-Acknowledge - some data was received but it was corrupted */
	DPROT_TYPE_SYNC = 0xA0  /**< Future use for syncing transmitter to receiver (auto-baudrate) */
};

/*********************************************************/
/*!
 * Function return codes (errors/warnings/data)
 */
enum
{
	DPROT_NO_ERROR = 0x00,      /**< All was OK */
	DPROT_FRAMING_ERROR = 0x01, /**< The received data contained framing problem (mostly inconsistence with length) */
	DPROT_DATA_ERROR = 0x02, 	/**< The received message had data error - crc8/checksum8/xor8 mismatch */
	DPROT_LOGICAL_ERROR = 0x03, /**< The received data contained logical error like msg-type inconsistence, lentgh problem */
	DPROT_MSG_SIZE_ERROR = 0x04,/**< The received data had size problem */
	DPROT_ACK_ACCEPTED = 0xA0,  /**< Ack message was received */
	DPROT_NACK_ACCEPTED = 0xB0  /**< Nack message was received */
};

/*********************************************************/

/*!
 * \brief Initializing the master side protocol of the dProt
 *
 * \param put_function the 'putchar' function to be assigned to the lower layers
 * \param get_function the 'getchar' function to be assigned to the lower layers
 *
 * \return success (DPROT_NO_ERROR), error otherwise
 *
 */
uint8_t dprot_master_init_protocol (fn_put_char put_function, fn_get_char_to get_function);

/*!
 * \brief dProt master send ping message to the slave
 * \return success (DPROT_NO_ERROR), error otherwise
 */
uint8_t dprot_master_send_ping ( void );

/*!
 * \brief dProt master send sync message to the slave
 * \return success (DPROT_NO_ERROR), error otherwise
 */
uint8_t dprot_master_send_sync ( void );

/*!
 * \brief dProt master send data to the slave
 *
 * \param pre-allocated buffer to be sent to the slave
 * \param the number of bytes of buffer to be sent
 *
 * \return operation result:
 * \return          DPROT_NO_ERROR - Success
 * \return          DPROT_MSG_SIZE_ERROR - the requested buffer is too big for a single transaction.
 */
uint8_t dprot_master_send_data_msg (uint8_t* buffer, uint8_t len);

/*!
 * \brief master node waiting for the ack/nack message from the slave
 * \return result:
 * \return      DPROT_ACK_ACCEPTED - ack was received
 * \return      DPROT_NACK_ACCEPTED - nack was received
 * \return      DPROT_DATA_ERROR - 	the crc/chs/xor didn't match or unexpected type of msg was received
 */
uint8_t dprot_master_wait_for_ack_nack ( void );


/*!
 * \brief master node waiting a data message from the slave.
 * \brief It can happen adter the master intiated data request transactions.
 * \return result:
 * \return          DPROT_NO_ERROR - Success
 * \retrun          DPROT_MAX_PAYLOAD - data came corrupted - length is too big
 * \return          DPROT_DATA_ERROR - checking error
 * \return          DPROT_LOGICAL_ERROR - the incoming frame didn't contain data type of message
 */
uint8_t dprot_master_wait_for_data (uint8_t* buffer, uint8_t max_len);

/*!
 * \brief Initializing the slave side protocol of the dProt
 *
 * \param put_function the 'putchar' function to be assigned to the lower layers
 * \param get_function the 'getchar' function to be assigned to the lower layers
 *
 * \return success (DPROT_NO_ERROR), error otherwise
 *
 */
uint8_t dprot_slave_init_protocol (fn_put_char put_function, fn_get_char get_function);


/*!
 * \brief dProt slave waits for data message.
 * This function analyzes the received message types and automatically
 * filters them (sends back arp responses and sync stuff) until
 * a data message arrives. This message is returned to the higher layer.
 * The input 'buffer' and 'max_len' take into account the
 * additional 'type', 'length' and 'checking_byte'
 * which accumulate to additional 3 bytes. That means that
 * the maximum possible 'payload length' is 256-3=253 bytes
 * per transaction.

 *
 * \param pre-allocated buffer to store the rx elements
 * \param maximal length of buffer.
 *
 * \return the result: 
 * \return      DPROT_NO_ERROR (success)
 * \return      DPROT_FRAMING_ERROR (faming error occured)
 * \return      DPROT_DATA_ERROR (the crc/chs/xor didn't match)
 * \return      DPROT_LOGICAL_ERROR - sync/length or something else went wrong
 */
uint8_t dprot_slave_wait_for_msg (uint8_t* buffer, uint8_t max_len);


/*!
 * \brief dProt slave sending ack to the master
 * \return success (DPROT_NO_ERROR), error otherwise
 */
uint8_t dprot_slave_send_ack ( void );

/*!
 * \brief dProt slave sends 'nack' to the master (message error)
 * \return success (DPROT_NO_ERROR), error otherwise
 */
uint8_t dprot_slave_send_nack ( void );

/*!
 * \brief dProt slave send data to the master
 * This function will be always conducted by the slave in order to
 * send back data to the initiator (master)
 *
 * \param pre-allocated buffer to be sent to the master
 * \param the number of bytes of buffer to be sent
 *
 * \return operation result:
 * \return          DPROT_NO_ERROR - Success
 * \return          DPROT_MSG_SIZE_ERROR - the requested buffer is too big for a single transaction.
 */
uint8_t dprot_slave_send_data_msg (uint8_t* buffer, uint8_t len);

#endif //__DPROT_H__

