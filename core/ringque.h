/*
    Copyright (C) <2012>  <huangweilook@21cn.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _RINGQUE_H
#define _RINGQUE_H

//one reader one writer ringque

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
	struct NAME##cargo data[];\
}*NAME;\
static inline void NAME##_push(NAME que,TYPE element)\
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
static inline int32_t NAME##_pop(NAME que,TYPE *ret)\
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
static inline NAME NAME##_new(uint32_t maxsize){\
	maxsize = size_of_pow2(maxsize);\
	NAME ringque = calloc(1,sizeof(*ringque)*sizeof(struct NAME##cargo)*maxsize);\
	ringque->indexmask = maxsize-1;\
	ringque->ridx = ringque->widx = 0;\
	return ringque;\
}

#endif
