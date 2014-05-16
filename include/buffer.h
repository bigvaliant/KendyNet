#ifndef _BUFFER_H
#define _BUFFER_H
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
/*
* �����ü�����buffer
*/

#include <stdint.h>
#include <string.h>
#include "kn_ref.h"
#include "kn_list.h"
#include "kn_allocator.h"

extern uint32_t buffer_count;

typedef struct buffer
{
	kn_ref    _refbase;
	uint32_t  capacity;
	uint32_t  size;
	struct buffer *next;
	int8_t   buf[0];
}*buffer_t;


extern kn_allocator_t buffer_allocator;
//buffer_t buffer_create_and_acquire(buffer_t,uint32_t);

static inline void buffer_release(buffer_t *b)
{
	if(*b){
		kn_ref_release(&(*b)->_refbase);
        *b = NULL;
	}
}

static void buffer_destroy(void *b)
{
	buffer_t _b = (buffer_t)b;
	if(_b->next)
		buffer_release(&(_b)->next);
    FREE(buffer_allocator,_b);
    --buffer_count;
	b = 0;
}

static inline buffer_t buffer_create(uint32_t capacity)
{
	uint32_t size = sizeof(struct buffer) + capacity;
    buffer_t b = (buffer_t)ALLOC(buffer_allocator,size);
	if(b)
	{
		b->size = 0;
		b->capacity = capacity;
		kn_ref_init(&b->_refbase,buffer_destroy);
		++buffer_count;
	}
	return b;
}


static inline buffer_t buffer_acquire(buffer_t b1,buffer_t b2)
{
    if(b1 == b2) return b1;
    if(b2) kn_ref_acquire(&b2->_refbase);
    if(b1) kn_ref_release(&b1->_refbase);
	return b2;
}

/*
* ��b��pos��ʼ��ȡsize�ֽڳ�������,������ȴ���b->size-pos,�᳢�Դ�b->next�ж���ʣ�ಿ��
*/

static inline int buffer_read(buffer_t b,uint32_t pos,int8_t *out,uint32_t size)
{
	uint32_t copy_size;
	while(size){
        if(!b) return -1;
        if(pos >= b->size) return -1;
        copy_size = b->size - pos;
		copy_size = copy_size > size ? size : copy_size;
		memcpy(out,b->buf + pos,copy_size);
		size -= copy_size;
		pos += copy_size;
		out += copy_size;
		if(pos >= b->size){
			pos = 0;
			b = b->next;
		}
	}
	return 0;
}


/*
*������Ϊsize�ֽڵ�����д�뵽b��pos��ʼλ�ã����size����b->capacity-pos,�᳢�Խ�ʣ�ಿ��д�뵽
*b->next��
*/
static inline int buffer_write(buffer_t b,uint32_t pos,int8_t *in,uint32_t size)
{
    uint32_t copy_size;
    while(size){
        if(!b) return -1;
        if(pos >= b->capacity) return -1;
        copy_size = b->capacity - pos;
        copy_size = copy_size > size ? size : copy_size;
        memcpy(b->buf + pos,in,copy_size);
        size -= copy_size;
        pos += copy_size;
        in += copy_size;
        if(pos >= b->capacity){
            pos = 0;
            b = b->next;
        }
    }
    return 0;
}

#endif
