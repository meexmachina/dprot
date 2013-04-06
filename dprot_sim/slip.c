#include "slip.h"


/*
 * slip_get_char, slip_put_cher definitions
 */
fn_put_char slip_put_char = NULL;
fn_get_char slip_get_char = NULL;


/***********************************************************/
uint8_t slip_init(fn_put_char put_function, fn_get_char get_function, fn_get_char_to get_function_to, slip_channel* ch)
{
	ch->slip_put_char = put_function;
	ch->slip_get_char = get_function;
	ch->slip_get_char_to = get_function_to;
	return 0;
}

/***********************************************************/
uint8_t slip_rx(slip_channel* ch, uint8_t* buffer, uint8_t len)
{
	uint8_t bytes_read_so_far = 0;
	uint8_t c = 0;
    
	// check the initialization of the get function
	if (ch->slip_get_char == NULL && ch->slip_get_char_to == NULL)
	{
		return 0;
	}
	
	// run over the bytes and try to fill up the buffer
	while (1)
	{
		// read a character
        if (ch->slip_get_char == NULL)
        {
            if (!ch->slip_get_char_to(SLIP_RX_TIMEOUT, &c))
            {
                // waited and a timeout occured
                return bytes_read_so_far;
            }
        }
        else c = (ch->slip_get_char) ();
		
		switch (c)
		{
			//==========================================================/
			case SLIP_END:
				// The end of a transmission. If the end comes without
				// any input bytes, don't return - try to read the next
				// transmission that actually means something
				if (bytes_read_so_far)
				{
					return bytes_read_so_far;
				}
				else
				{
					break;
				}
				
			//==========================================================/
			case SLIP_ESC:
				// if it's the same code as the ESC character, wait
				// and get another character and then figure out what
				// to store in the packet.
                if (ch->slip_get_char == NULL)
                {
                    if (!ch->slip_get_char_to(SLIP_RX_TIMEOUT, &c))
                    {
                        // waited and a timeout occured
                        return bytes_read_so_far;
                    }
                }
                else c = (ch->slip_get_char) ();
				
				// check if 'c' if one of the two possible combinations.
				// if none of them complies, just push everything into
				// the buffer - it was a violation but what can we do.
				if (c==SLIP_DATA_END) c = SLIP_END;
				else if (c==SLIP_DATA_ESC) c = SLIP_ESC;
			
				// the 'c' from here goes to the default part
				
			//==========================================================/
			default:
				// if we reached the end of the buffer, we stop writing
				// into it and just exhoust the frame until its end 
				// (read more and more characters untill we get END)
				// The layer over this layer should check that the buffer
				// contains a proper information (crc?)
				if (bytes_read_so_far<len)
				{
					buffer[bytes_read_so_far++] = c;
				}
		}
	}
	
	// we shouldn't get here anyway
	return bytes_read_so_far;
}


/***********************************************************/
uint8_t slip_tx(slip_channel* ch, uint8_t* buffer, uint8_t len, uint8_t start_end)
{
	uint8_t bytes_written = len;
	
	// check the initialization of the put function
	if (ch->slip_put_char == NULL)
	{
		return 0;
	}
	
	// Send an initial END character to flush out any data
	// that may have accumulated in the receiver (line noise)
	if (start_end&SLIP_MSG_START)
	{
		ch->slip_put_char (SLIP_END);
	}
	
	// for each byte send an appripriate byte sequence
	while (len--)
	{
		switch (*buffer) 
		{
			//==========================================================/
			case SLIP_END:
				// if it the same character as END we need to send 
				// a sequence 'ESC'+'DATA_END'
				ch->slip_put_char (SLIP_ESC);
				ch->slip_put_char (SLIP_DATA_END);
				break;
				
			//==========================================================/
			case SLIP_ESC:
				// if it the same code as ESC, write the sequence 'ESC'+
				// +'DATA_ESC'
				ch->slip_put_char (SLIP_ESC);
				ch->slip_put_char (SLIP_DATA_ESC);
				break;
				
			//==========================================================/
			default:
				// otherwise just send the character as it is
				ch->slip_put_char (*buffer);
		}
		
		// go to the next byte
		buffer ++;
	}
	
	// send the END byte to end the frame
	if (start_end&SLIP_MSG_END)
	{
		ch->slip_put_char (SLIP_END);
	}
	
	return bytes_written;
}


