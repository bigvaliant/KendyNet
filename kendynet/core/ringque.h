#ifndef _RINGQUE_H
#define _RINGQUE_H

//one reader one writer ringque,just for test

#include <stdint.h>
#include "atomic.h"
#include "buffer.h"
#include "common_define.h"
#include <assert.h>

typedef struct ringque{	
	volatile uint32_t ridx;
	uint32_t maxsize;
	volatile uint32_t widx;
	void* data[];
}*ringque_t;

static inline ringque_t new_ringque(uint32_t maxsize){
	maxsize = size_of_pow2(maxsize);
	ringque_t ringque = calloc(1,sizeof(*ringque)*sizeof(void*)*maxsize);
	ringque->maxsize = maxsize;
	ringque->ridx = ringque->widx = 0;
	return ringque;
}

static inline int32_t ringque_push(ringque_t que,void *element)
{
	uint32_t maxsize = que->maxsize;
	if(((que->widx+1)%maxsize) == que->ridx)
		return -1;	
	assert(que->data[que->widx] == NULL);
	que->data[que->widx] = element;
	_FENCE;
	que->widx = (que->widx+1)%maxsize;
	return 0;			
}

static inline void *ringque_pop(ringque_t que)
{
	if(que->ridx == que->widx)
		return NULL;
	uint32_t maxsize = que->maxsize;	
	void *ret = que->data[que->ridx];
	que->data[que->ridx] = NULL;
	_FENCE;
	que->ridx = (que->ridx+1)%maxsize;
	return ret;			
}




#endif
