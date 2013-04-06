#include "ts_char_queue.h"


//==================================================
ts_queue* 	tsq_create (void)
{
	ts_queue* new_q = (ts_queue*)malloc (sizeof(ts_queue));
	
	if (new_q == NULL)
	{
		return NULL;	
	}
	
	// initialize the data
	memset(new_q->ar, 0, TSQ_MAX_SIZE);
	new_q->front = 0;
	new_q->rear = 0;
	new_q->size = 0;	// current
	
	if (0!=pthread_mutex_init(&new_q->q_mutex, NULL))
	{
		// error creating mutex
		free (new_q);
		new_q = NULL;
	}
	
	return new_q;
}

//==================================================
void 		tsq_delete (ts_queue* q)
{
	if (q==NULL) return;
	
	// destroy mutex
	pthread_mutex_destroy(&q->q_mutex);
	
	free (q);
}

//==================================================
int			tsq_is_full (ts_queue* q)
{ 
	int full = 0;
	pthread_mutex_lock(&q->q_mutex);
	full = q->size == TSQ_MAX_SIZE;
	pthread_mutex_unlock(&q->q_mutex);
	
	return full;
}

//==================================================
int		tsq_pop_item (ts_queue* q, uint8_t *c)
{
	pthread_mutex_lock(&q->q_mutex);
    
    if (q->size == 0)
    {
        pthread_mutex_unlock(&q->q_mutex);
        return -1;
    }
	
    q->size--;
    *c = q->ar[q->front];
    q->front++;
    if (q->front >= TSQ_MAX_SIZE) q->front = 0;
    
	pthread_mutex_unlock(&q->q_mutex);
	
	return 0;
}

//==================================================
void		tsq_push_item (ts_queue* q, uint8_t item)
{
	pthread_mutex_lock(&q->q_mutex);
    
    /*if (q->size == TSQ_MAX_SIZE)
    {
        pthread_mutex_unlock(&q->q_mutex);
        return;
    }*/
	
    q->ar[q->rear] = item;          // if needed overwrite
    
    q->rear ++;
    if (q->rear >=TSQ_MAX_SIZE) q->rear = 0;
    q->size++;
    if (q->size > TSQ_MAX_SIZE) q->size = TSQ_MAX_SIZE;
      
    pthread_mutex_unlock(&q->q_mutex);
}

//==================================================
int			tsq_empty (ts_queue* q)
{
	int empty = 0;
	empty = q->size == 0;
	return empty;
}

