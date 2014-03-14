#ifndef _RINGQUE_H
#define _RINGQUE_H

//one reader one writer ringque,just for test

#include <stdint.h>
#include "atomic.h"
#include "buffer.h"
#include "common_define.h"
#include <assert.h>


#define CARGO_CAPACITY 128

#define DECLARE_RINGQUE(NAME,TYPE)\
struct NAME##cargo{\
	TYPE cargo[CARGO_CAPACITY];\
	uint8_t size;\
};\
typedef struct NAME{\
	volatile uint32_t ridx;\
	uint32_t indexmask;\
	volatile uint32_t widx;\
	void (*push)(struct NAME*,TYPE);\
	int32_t (*pop)(struct NAME*,TYPE*);\
	struct NAME##cargo data[];\
}*NAME;\
static inline void NAME##push(NAME que,TYPE element)\
{\
	struct NAME##cargo *cargo = &que->data[que->widx];\
	cargo->cargo[cargo->size++] = element;\
	if(unlikely(cargo->size == CARGO_CAPACITY))\
	{\
		int32_t c=0;\
		uint32_t next_widx = (que->widx+1)&que->indexmask;\
		while(next_widx == que->ridx)\
		{\
			if(c < 4000){\
                ++c;\
                __asm__("pause");\
            }else{\
                struct timespec ts = { 0, 500000 };\
                nanosleep(&ts, NULL);\
            }\
		}\
		_FENCE;\
		que->widx = next_widx;\
	}\
}\
static inline int32_t NAME##pop(NAME que,TYPE *ret)\
{\
	if(que->ridx == que->widx)\
		return -1;\
	_FENCE;\
	struct NAME##cargo *cargo = &que->data[que->ridx];\
	if(unlikely(cargo->size == 0)){\
		_FENCE;\
		que->ridx = (que->ridx+1)&que->indexmask;\
		if(que->ridx == que->widx)\
			return -1;\
		cargo = &que->data[que->ridx];\
	}\
	*ret = cargo->cargo[CARGO_CAPACITY-cargo->size];\
	--cargo->size;\
	return 0;\
}\
static inline NAME NAME##new_ringque(uint32_t maxsize){\
	maxsize = size_of_pow2(maxsize);\
	NAME ringque = calloc(1,sizeof(*ringque)*sizeof(struct NAME##cargo)*maxsize);\
	ringque->indexmask = maxsize-1;\
	ringque->ridx = ringque->widx = 0;\
	ringque->push = NAME##push;\
	ringque->pop = NAME##pop;\
	return ringque;\
}

#define RINGQUE_PUSH(QUE,VALUE) QUE->push(QUE,VALUE)

#define RINGQUE_POP(QUE,RET) QUE->pop(QUE,RET)

#define RINGQUE_NEW(NAME,SIZE) NAME##new_ringque(SIZE)

/*
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
*/

#endif
