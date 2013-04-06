#include "dprot.h"

slip_channel    master_channel = {0};
uint8_t         master_last_parity = 1;

/***********************************************************/
uint8_t dprot_master_init_protocol (fn_put_char put_function, fn_get_char_to get_function)
{
	// initialize the crc table
	init_crc8( );
    
    master_last_parity = 0;
	
	// initialize the slip protocol
	return slip_init (put_function, NULL, get_function, &master_channel);
}

/***********************************************************/
uint8_t dprot_master_wait_for_data (uint8_t* buffer, uint8_t max_len)
{
	uint8_t actual_rx = 0;
	uint8_t type = 0;
	uint8_t length = 0;
    uint8_t seq_parity = 0;
	uint8_t calc_check = 0;
	uint8_t i = 0;
	uint8_t *data_ptr = NULL;
	
	// read a slip frame with maximum 'max_len' size
	actual_rx = slip_rx(&master_channel, buffer, max_len);
	
	// read out all needed information
    seq_parity = (buffer[0] & 0x01);
	type = (buffer[0]&0xfe);
	length = buffer[1];
	data_ptr = &buffer[2];
    
    // check parity
    if (seq_parity != master_last_parity)
	{
        // error - we got an ack of encient message
        return DPROT_DATA_ERROR;
    }
    
    // check the length
	if (length > DPROT_MAX_PAYLOAD)
	{
		// length error - checking method can't be applied
		// because we don't know where actually the msg ends
		return DPROT_LOGICAL_ERROR;
	}
	
	// calculate the checking byte
	DPROT_CHECKING(calc_check,buffer[0]);
	DPROT_CHECKING(calc_check,buffer[1]);
	for (i = 0; i < length; i++)
	{
		DPROT_CHECKING(calc_check,data_ptr[i]);
	}
    
	
	// compare the checking byte
	if (data_ptr[i] != calc_check)
	{
		// checksum/crc8 error. return a DATA_ERROR
		return DPROT_DATA_ERROR;
	}
    
	// check the type
	switch (type)
	{
		case DPROT_TYPE_DATA:
            break;
            
		case DROPT_TYPE_ARP:
        case DPROT_TYPE_SYNC:
        default:
			// this message is not a data packet
			return DPROT_LOGICAL_ERROR;
	}
	
    
	return DPROT_NO_ERROR;
}


/***********************************************************/
uint8_t dprot_master_wait_for_ack_nack ( void )
{
	uint8_t type = 0;
	uint8_t length = 0;
	uint8_t calc_check = 0;
	uint8_t buffer[4] = {0};
	uint8_t actual_rx = 0;
	uint8_t parity = 0;
    
	// the expectes size of ack message is 3
	actual_rx = slip_rx(&master_channel, buffer, 3);
	
	// check that we got exactly the length we needed
	if (actual_rx != 3)
	{
		// the input data is shorter than expected
		return DPROT_DATA_ERROR;
	}
	
    parity = buffer[0]&0x1;
	type = buffer[0]&0xfe;
	length = buffer[1];
    
    // check parity
    if (parity != master_last_parity)
	{
        // error - we got an ack of encient message
        return DPROT_DATA_ERROR;
    }
        
	// check type
	if (type != DPROT_TYPE_ACK && type != DPROT_TYPE_NACK)
	{
		// an unexpected data type has been received
		// return and let the master sender to decide what
		// to do next
		return DPROT_DATA_ERROR;
	}
	
	// check length
	if (length > 0)
	{
		// an unexpected data length (other then 0) was received
		// return with error because it violates the protocol
		return DPROT_DATA_ERROR;
	}
	
	// check the checking byte - calculate and compare
	DPROT_CHECKING(calc_check,buffer[0]);
	DPROT_CHECKING(calc_check,buffer[1]);
	
	if (buffer[2] != calc_check)
	{
		// checksum error
		return DPROT_DATA_ERROR;
	}
	
	if (type == DPROT_TYPE_ACK) return DPROT_ACK_ACCEPTED;
	return DPROT_NACK_ACCEPTED;
}

/***********************************************************/
uint8_t dprot_master_send_data_msg (uint8_t* buffer, uint8_t len)
{
	uint8_t i;
    uint8_t ret = 0;
	uint8_t calc_check = 0;
    uint8_t retry = DPROT_MASTER_NUM_RETRIES;
	uint8_t header[2] = { DPROT_TYPE_DATA, len };
    
   
	
	if (len > DPROT_MAX_PAYLOAD)
	{
		// the buffer is bigger than the maximal allowed
		// transaction size
		return DPROT_MSG_SIZE_ERROR;
	}
		
	// calculate the checking
	DPROT_CHECKING(calc_check,header[0]);
	DPROT_CHECKING(calc_check,header[1]);
	for (i = 0; i < len; i++)
	{
		DPROT_CHECKING(calc_check,buffer[i]);
	}
	
    while (retry--)
    {
        // finally send the data
        slip_tx(&master_channel, header, 2, SLIP_MSG_START);
        slip_tx(&master_channel, buffer, len, SLIP_MSG_MIDDLE);
        slip_tx(&master_channel, &calc_check, 1, SLIP_MSG_END);
        
        // wait for response
        ret = dprot_master_wait_for_ack_nack ( );
        if (ret == DPROT_ACK_ACCEPTED)
        {
            // stop trying
            break;
        }

    }
	return ret;
}

/***********************************************************/
uint8_t dprot_master_send_ping ( void )
{
    uint8_t ret = 0;
    uint8_t retry = DPROT_MASTER_NUM_RETRIES;
    uint8_t buffer[3] = { DROPT_TYPE_ARP, 0, 0 };
    
    // advance the parity and embed it
    master_last_parity = !master_last_parity;
    buffer[0] |= master_last_parity;
	
	// calculate checking
	DPROT_CHECKING(buffer[2],buffer[0]);
	DPROT_CHECKING(buffer[2],buffer[1]);

    while (retry--)
    {
        slip_tx(&master_channel, buffer, 3, SLIP_MSG_REG);
        
        // wait for response
        ret = dprot_master_wait_for_ack_nack ( );
        if (ret == DPROT_ACK_ACCEPTED)
        {
            // stop trying
            break;
        }
    }
    
    return ret;
}

/***********************************************************/
uint8_t dprot_master_send_sync ( void )
{
	// currently not implemented
	return DPROT_NO_ERROR;
}

