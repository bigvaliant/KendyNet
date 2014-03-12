#ifndef _RINGQUE_H
#define _RINGQUE_H

//one reader one writer ringque,just for test

#include <stdint.h>
#include "atomic.h"
#include "buffer.h"
#include "common_define.h"
#include <assert.h>


#define CARGO_CAPACITY 64

struct cargo{
	void *cargo[CARGO_CAPACITY];
	uint8_t size;
};

typedef struct ringque{	
	volatile uint32_t ridx;
	uint32_t indexmask;
	volatile uint32_t widx;
	struct cargo data[];
}*ringque_t;

static inline ringque_t new_ringque(uint32_t maxsize){
	maxsize = size_of_pow2(maxsize);
	ringque_t ringque = calloc(1,sizeof(*ringque)*sizeof(struct cargo)*maxsize);
	ringque->indexmask = maxsize-1;
	ringque->ridx = ringque->widx = 0;
	return ringque;
}

static inline void ringque_push(ringque_t que,void *element)
{
	struct cargo *cargo = &que->data[que->widx];
	cargo->cargo[cargo->size++] = element;
	if(unlikely(cargo->size >= CARGO_CAPACITY))
	{
		_FENCE;
		int32_t c=0;//,max;
		uint32_t next_widx = (que->widx+1)&que->indexmask;
		while(next_widx == que->ridx)
		{
			if(c < 4000){
                ++c;
                __asm__("pause");
            }else{
                struct timespec ts = { 0, 500000 };
                nanosleep(&ts, NULL);
            }
			//__asm__ volatile("" : : : "memory");
			//for(c = 0; c < (max = rand()%4096); ++c)
			//	__asm__("pause");
		}		
		que->widx = next_widx;
	}			
}

static inline void *ringque_pop(ringque_t que)
{	
	struct cargo *cargo = &que->data[que->ridx];
	if(unlikely(cargo->size == 0)){
		if(que->ridx == que->widx)
			return NULL;		
		que->ridx = (que->ridx+1)&que->indexmask;
		_FENCE;
		cargo = &que->data[que->ridx]; 	
	}
	void *ret = cargo->cargo[CARGO_CAPACITY-cargo->size];
	--cargo->size;
	return ret;		
}

#endif
