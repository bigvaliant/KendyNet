#ifndef _RINGQUE_H
#define _RINGQUE_H

//one reader one writer ringque,just for test

#include <stdint.h>
#include "atomic.h"
#include "buffer.h"
#include "common_define.h"
#include <assert.h>

/*
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
	if(cargo->cargo[cargo->size] != NULL){
		printf("3 %d,%d\n",(int)cargo->cargo[cargo->size],cargo->size);
		exit(0);
	}
	cargo->cargo[cargo->size++] = element;
	if(unlikely(cargo->size == CARGO_CAPACITY))
	{
		int32_t c=0;
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
		}
		_FENCE;		
		que->widx = next_widx;
		if(que->data[next_widx].size != 0)
		{
			printf("1 %d\n",que->data[next_widx].size);
			exit(0);
		}
	}			
}

static inline void *ringque_pop(ringque_t que)
{
	if(que->ridx == que->widx)
		return NULL;
	//printf("h\n");			
	struct cargo *cargo = &que->data[que->ridx];
	if(unlikely(cargo->size == 0)){	
		que->ridx = (que->ridx+1)&que->indexmask;
		if(que->ridx == que->widx)
			return NULL;		
		cargo = &que->data[que->ridx];
		if(cargo->size != CARGO_CAPACITY)
		{
			printf("2 %d\n",cargo->size);
			exit(0);
		} 	
	}
	void *ret = cargo->cargo[CARGO_CAPACITY-cargo->size];
	cargo->cargo[CARGO_CAPACITY-cargo->size] = NULL;
	--cargo->size;
	return ret;		
}*/

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
	int32_t c=0;
	uint32_t next_widx = (que->widx+1)%que->maxsize;
	while(next_widx == que->ridx)
	{
		if(c < 4000){
			++c;
			__asm__("pause");
		}else{
			struct timespec ts = { 0, 500000 };
			nanosleep(&ts, NULL);
		}
	}
	if(que->data[que->widx] != NULL)
		exit(0);
	que->data[que->widx] = element;
	_FENCE;
	que->widx = next_widx;
	return 0;			
}

static inline void *ringque_pop(ringque_t que)
{
	if(que->ridx == que->widx)
		return NULL;
	void *ret = que->data[que->ridx];
	if(ret == NULL)
		exit(0);	
	que->data[que->ridx] = NULL;
	_FENCE;
	que->ridx = (que->ridx+1)%que->maxsize;
	return ret;			
}


#endif
