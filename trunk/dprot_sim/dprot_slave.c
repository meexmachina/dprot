#include "dprot.h"

slip_channel    slave_channel = {0};
uint8_t         slave_last_parity = 1;

/***********************************************************/
uint8_t dprot_slave_init_protocol (fn_put_char put_function, fn_get_char get_function)
{
	// initialize the crc table
	init_crc8( );
	
    slave_last_parity = 0;
    
	// initialize the slip protocol
	return slip_init (put_function, get_function, NULL, &slave_channel);
}

/***********************************************************/
uint8_t dprot_slave_wait_for_msg (uint8_t* buffer, uint8_t max_len)
{
	uint8_t actual_rx = 0;
	uint8_t type = 0;
	uint8_t length = 0;
    uint8_t seq_parity = 0;
	uint8_t calc_check = 0;
	uint8_t i = 0;
	uint8_t *data_ptr = NULL;
	
	// read a slip frame with maximum 'max_len' size
	actual_rx = slip_rx(&slave_channel, buffer, max_len);
	
	// read out all needed information
    seq_parity = (buffer[0] & 0x01);
	type = (buffer[0]&0xfe);
	length = buffer[1];
	data_ptr = &buffer[2];
    
    // check if we already delt with this request
    if (seq_parity == slave_last_parity)
    {
        //printf("SLAVE ==> SAME PARITY\n");
        dprot_slave_send_ack ( );
        return DPROT_NO_ERROR;
    }
	
    // check the length
	if (length > DPROT_MAX_PAYLOAD)
	{
		// length error - checking method can't be applied
		// because we don't know where actually the msg ends
		// Send nack
		dprot_slave_send_nack ( );
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
		// checksum/crc8 error. We need to send a NACK
		// message and return a DATA_ERROR
		dprot_slave_send_nack ( );
		return DPROT_DATA_ERROR;
	}

    // save the last request's sequencial parity
    slave_last_parity = seq_parity;
	
    
	// check the type
	switch (type)
	{
		case DPROT_TYPE_DATA:
		case DROPT_TYPE_ARP:
            dprot_slave_send_ack ( );
            break;
		case DPROT_TYPE_SYNC:
            // can be used for auto-baudrate in the future
            break;
            
		default:
			// error - this msg type doesn't exist.
			return DPROT_LOGICAL_ERROR;
	}
	
    
	return DPROT_NO_ERROR;
}

/***********************************************************/
uint8_t dprot_slave_send_data_msg (uint8_t* buffer, uint8_t len)
{
	uint8_t i;
	uint8_t calc_check = 0;
	uint8_t header[2] = { DPROT_TYPE_DATA, len };
	
    header[0] |= slave_last_parity;
    
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
	
	// finally send the data
	slip_tx(&slave_channel, header, 2, SLIP_MSG_START);
	slip_tx(&slave_channel, buffer, len, SLIP_MSG_MIDDLE);
	slip_tx(&slave_channel, &calc_check, 1, SLIP_MSG_END);
	
	return DPROT_NO_ERROR;
}


/***********************************************************/
uint8_t dprot_slave_send_ack ( void )
{
	uint8_t buffer[3] = { DPROT_TYPE_ACK, 0, 0 };
    
    buffer[0] |= slave_last_parity;
	
	// calculate checking
	DPROT_CHECKING(buffer[2],buffer[0]);
	DPROT_CHECKING(buffer[2],buffer[1]);
    
	slip_tx(&slave_channel, buffer, 3, SLIP_MSG_REG);
	return DPROT_NO_ERROR;
}

/***********************************************************/
uint8_t dprot_slave_send_nack ( void )
{
	uint8_t buffer[3] = { DPROT_TYPE_NACK, 0, 0 };
	
    buffer[0] |= slave_last_parity;
    
	// calculate checking
	DPROT_CHECKING(buffer[2],buffer[0]);
	DPROT_CHECKING(buffer[2],buffer[1]);
    
	slip_tx(&slave_channel, buffer, 3, SLIP_MSG_REG);
	return DPROT_NO_ERROR;
}

