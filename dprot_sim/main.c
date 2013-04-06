#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ts_char_queue.h"
#include "dprot.h"
#include "spec_types.h"


// the input and output queues (from the master's point of view)
ts_queue *in_channel = NULL;
ts_queue *out_channel = NULL;
pthread_t master_thread;
pthread_t slave_thread;
unsigned int number_if_messages_to_send = 1000;
double out_channel_ber = 0.001;
double in_channel_ber = 0.00;
int global_count = 0;

//===============================================
// Random number
double drandom (void)
{
	int r =  rand();
	return ((double)(r))/((double)(RAND_MAX));
}

//===============================================
// Random message
uint8_t generate_random_message(uint8_t *buffer, uint8_t max_len)
{
    // generate the length
    uint8_t i = 0;
    uint8_t length = (uint8_t)(drandom()*max_len);
    
    for (i = 0; i<length; i++)
    {
        buffer[i] = (uint8_t)(drandom()*256);
    }
    
    return length;
}

//===============================================
// declaration of the writing/reading functions
// in the channels
uint8_t master_get_char ( uint8_t to, uint8_t *cout)
{
	*cout = 0;
	double r = drandom ();
	
	while ((tsq_pop_item (in_channel, cout)==-1) && --to)
	{
        usleep(1000);
	}
	
    if (!to) return 0;
    
    //tsq_pop_item (in_channel, cout);
    
	if (r<in_channel_ber) *cout = (uint8_t)(drandom()*256);
		
	return 1;
}
void master_put_char (uint8_t c)
{
	uint8_t new_c = c;
	double r = drandom ();
	
	if (r<out_channel_ber) new_c = (uint8_t)(drandom()*256);
	tsq_push_item (out_channel, new_c);
}

uint8_t slave_get_char ( )
{
    uint8_t item = 0;
	while (tsq_pop_item (out_channel, &item)==-1)
	{
	}

	return item;
}

void slave_put_char (uint8_t c)
{
	tsq_push_item (in_channel, c);
}




//===============================================
// The master/slave threads
void *master_thread_function( void *ptr )
{
	int num_msgs = number_if_messages_to_send;
	uint8_t ret = 0;
	uint8_t buffer[256] = {0};
    uint8_t length = 0;
    
	dprot_master_init_protocol (master_put_char, master_get_char);
	
	
	while (num_msgs--)
	{
        // generate a random message
        length = generate_random_message(buffer, 128);
        
		printf("%d) Master => sending random message (#%d)...\n", global_count++, number_if_messages_to_send-num_msgs-1);
		//ret = dprot_master_send_ping ( );
        
		ret = dprot_master_send_data_msg(buffer, length);
		switch (ret)
		{
			case DPROT_ACK_ACCEPTED:
				printf("%d) Master => received ACK (#%d)\n\n", global_count++, number_if_messages_to_send-num_msgs-1);
				break;
			case DPROT_NACK_ACCEPTED:
				printf("%d) Master => received NACK (#%d)\n\n", global_count++, number_if_messages_to_send-num_msgs-1);
				break;
			case DPROT_DATA_ERROR:
				printf("%d) Master => received JUNK (#%d)\n\n", global_count++, number_if_messages_to_send-num_msgs-1);
				break;
			default:
				break;
		}
        
        usleep(10000);
	}
	
	number_if_messages_to_send = 0;
    
    return NULL;
}

void *slave_thread_function( void *ptr )
{
	dprot_slave_init_protocol (slave_put_char, slave_get_char);
	uint8_t buffer[256] = {0};
	unsigned int correct_counter = 0;
	unsigned int incorrect_counter = 0;
	
	while (number_if_messages_to_send)
	{
		uint8_t ret = 0;
		ret = dprot_slave_wait_for_msg (buffer, 255);
		
		switch (ret)
		{
			case DPROT_NO_ERROR:
				printf("%d) Slave => got a proper message (#%u)\n", global_count++, correct_counter++);
				break;
			case DPROT_FRAMING_ERROR:
			case DPROT_DATA_ERROR:
			case DPROT_LOGICAL_ERROR:
				printf("%d) Slave => got error message (#%u)\n", global_count++, incorrect_counter++);
				break;
			default:
				break;
		}		
	}
    
    return NULL;
}


//===============================================
int main ()
{
	int ret1, ret2;

	// create the channels
	in_channel = tsq_create();
	out_channel = tsq_create();

	// create the threads
	ret1 = pthread_create( &slave_thread, NULL, slave_thread_function, NULL);
	ret2 = pthread_create( &master_thread, NULL, master_thread_function, NULL);
	
	// wait for join
	pthread_join( master_thread, NULL);
	pthread_join( slave_thread, NULL);

	// delete the channels
	tsq_delete (in_channel);
	tsq_delete (out_channel);

	printf("Both threads returned.\n");
	exit(0);
}


