#ifndef __TS_CHAR_QUEUE_H__
#define __TS_CHAR_QUEUE_H__

#include "spec_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define TSQ_MAX_SIZE	128

typedef struct
{
	uint8_t ar[TSQ_MAX_SIZE];
	int32_t front;
	int32_t rear;
	int32_t size;
	pthread_mutex_t q_mutex;
} ts_queue;

ts_queue* 	tsq_create (void);
void 		tsq_delete (ts_queue* q);
int			tsq_is_full (ts_queue* q);
int         tsq_pop_item (ts_queue* q, uint8_t *c);
void		tsq_push_item (ts_queue* q, uint8_t item);
int			tsq_empty (ts_queue* q);

#endif //__TS_CHAR_QUEUE_H__

