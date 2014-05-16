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
#ifndef _WPACKET_H
#define _WPACKET_H
#include "packet.h"
#include "kn_common_define.h"

extern kn_allocator_t wpacket_allocator;
typedef struct wpacket
{
	struct packet base;
	uint32_t *len;      //�����ֶ�(ȥ���������ֶ�����ʵ�����ݵĳ���)��buf�еĵ�ַ
    buffer_t writebuf;  //wpos���ڵ�buf
	uint32_t wpos;
	uint32_t data_size;//ʵ�����ݴ�С,����������
}*wpacket_t;

struct rpacket;
typedef struct
{
	buffer_t buf;
	uint32_t wpos;
}write_pos;

wpacket_t wpk_create(uint32_t size,uint8_t is_raw);

wpacket_t wpk_create_by_wpacket(struct wpacket *_w);

wpacket_t wpk_create_by_rpacket(struct rpacket *r);//,uint32_t dropsize);

//wpacket_t wpk_create_by_buffer(buffer_t,uint32_t begpos,uint32_t len,uint32_t is_raw);
void      wpk_destroy(wpacket_t);

//����һ����ԭʼ��(���߳�)
#define NEW_WPK(__SIZE) wpk_create(__SIZE,0)

//����һ��ԭʼ��(���߳�)
#define NEW_WPK_RAW(__SIZE) wpk_create(__SIZE,1)


static inline write_pos wpk_get_writepos(wpacket_t w)
{
    write_pos wp = {NULL,0};
    if(!PACKET_RAW(w))
    {
      wp.buf =  w->writebuf;
      wp.wpos = w->wpos;
    }
	return wp;
}

static inline void wpk_rewrite(write_pos *wp,int8_t *addr,uint32_t size)
{
	int8_t *ptr = addr;
	uint32_t copy_size;
	uint32_t pos = wp->wpos;
	while(size){
		copy_size = wp->buf->capacity - pos;
		copy_size = copy_size > size ? size:copy_size;
		memcpy(wp->buf->buf + pos,ptr,copy_size);
		ptr += copy_size;
		size -= copy_size;
		pos += copy_size;
		if(size && pos >= wp->buf->capacity)
		{
			assert(wp->buf->next);
			wp->buf = wp->buf->next;
			pos = 0;
		}
	}
}

static inline void wpk_rewrite_uint8(write_pos *wp,uint8_t value)
{
	wpk_rewrite(wp,(int8_t*)&value,sizeof(value));
}

static inline void wpk_rewrite_uint16(write_pos *wp,uint16_t value)
{
	wpk_rewrite(wp,(int8_t*)&value,sizeof(value));
}

static inline void wpk_rewrite_uint32(write_pos *wp,uint32_t value)
{
	wpk_rewrite(wp,(int8_t*)&value,sizeof(value));
}

static inline void wpk_rewrite_uint64(write_pos *wp,uint64_t value)
{
	wpk_rewrite(wp,(int8_t*)&value,sizeof(value));
}

static inline void wpk_rewrite_double(write_pos *wp,double value)
{
	wpk_rewrite(wp,(int8_t*)&value,sizeof(value));
}
/*
static inline void wpk_rewrite_pointer(write_pos *wp,void *ptr)
{
#ifdef _X64
	wpk_rewrite_uint64(wp,(uint64_t)ptr);
#else
	wpk_rewrite_uint32(wp,(uint32_t)ptr);
#endif	
}*/

static inline void wpk_expand(wpacket_t w,uint32_t size)
{
    size = size_of_pow2(size);
    if(size < 64) size = 64;
    w->writebuf->next = buffer_create(size);
	w->writebuf = buffer_acquire(w->writebuf,w->writebuf->next);
	w->wpos = 0;
}

//��w���������ݿ�����buf��
static inline void wpk_copy(wpacket_t w,buffer_t buf)
{
	int8_t *ptr = buf->buf;
    buffer_t tmp_buf = PACKET_BUF(w);
	uint32_t copy_size;
    uint32_t size = w->data_size;
    uint32_t pos = PACKET_BEGINPOS(w);
    while(size)
	{
        assert(tmp_buf);
        copy_size = tmp_buf->size - pos;
        memcpy(ptr,tmp_buf->buf+pos,copy_size);
		ptr += copy_size;
        size -= copy_size;
        pos = 0;
		tmp_buf = tmp_buf->next;
	}
}

static inline void do_write_copy(wpacket_t w)
{
	/*wpacket����rpacket����ģ�����ִ��дʱ������
	* ִ�����wpacket�͹���ʱ�����rpacket���ٹ���buffer
	*/
    uint32_t size = size_of_pow2(w->data_size);
    if(size < 64) size = 64;
    buffer_t tmp = buffer_create(size);
	wpk_copy(w,tmp);
    PACKET_BEGINPOS(w) = 0;
    if(!PACKET_RAW(w))
		w->len = (uint32_t*)tmp->buf;
    w->wpos = w->data_size;
    PACKET_BUF(w) = buffer_acquire(PACKET_BUF(w),tmp);
    w->writebuf = buffer_acquire(w->writebuf,PACKET_BUF(w));
}

static inline void wpk_write(wpacket_t w,int8_t *addr,uint32_t size)
{
	int8_t *ptr = addr;
	uint32_t copy_size;
	if(!w->writebuf){
		do_write_copy(w);
	}
	while(size)
	{
		copy_size = w->writebuf->capacity - w->wpos;
		if(copy_size == 0)
		{
            wpk_expand(w,size);//�ռ䲻��,��չ
			copy_size = w->writebuf->capacity - w->wpos;
		}
		copy_size = copy_size > size ? size:copy_size;
		memcpy(w->writebuf->buf + w->wpos,ptr,copy_size);
		w->writebuf->size += copy_size;
		if(w->len)
			(*w->len) += copy_size;
		w->wpos += copy_size;
		ptr += copy_size;
		size -= copy_size;
		w->data_size += copy_size;
	}
}

#define CHECK_WRITE(TYPE,VALUE)\
		if(likely(w->writebuf->capacity - w->wpos >= sizeof(TYPE))){\
			uint32_t pos = w->wpos;\
			w->wpos += sizeof(TYPE);\
			*((TYPE*)(&w->writebuf->buf[pos])) = VALUE;\
			return;\
		}

static inline void wpk_write_uint8(wpacket_t w,uint8_t value)
{
    if(PACKET_RAW(w))return;
    CHECK_WRITE(uint8_t,value);
	wpk_write(w,(int8_t*)&value,sizeof(value));
}

static inline void wpk_write_uint16(wpacket_t w,uint16_t value)
{
    if(PACKET_RAW(w))return;
    CHECK_WRITE(uint16_t,value);
	wpk_write(w,(int8_t*)&value,sizeof(value));
}

static inline void wpk_write_uint32(wpacket_t w,uint32_t value)
{
    if(PACKET_RAW(w))return;
    CHECK_WRITE(uint32_t,value);
	wpk_write(w,(int8_t*)&value,sizeof(value));
}

static inline void wpk_write_uint64(wpacket_t w,uint64_t value)
{
    if(PACKET_RAW(w))return;
    CHECK_WRITE(uint64_t,value);
	wpk_write(w,(int8_t*)&value,sizeof(value));
}

static inline void wpk_write_ident(wpacket_t w,ident value)
{
    if(PACKET_RAW(w))return;
    CHECK_WRITE(ident,value);
	wpk_write(w,(int8_t*)&value,sizeof(value));
}

/*
static inline void wpk_write_pointer(wpacket_t w,void *ptr)
{
#ifdef _X64
	wpk_write_uint64(w,(uint64_t)ptr);
#else
	wpk_write_uint32(w,(uint32_t)ptr);
#endif	
}
*/
static inline void wpk_write_double(wpacket_t w ,double value)
{
    if(PACKET_RAW(w))return;
    CHECK_WRITE(double,value);
	wpk_write(w,(int8_t*)&value,sizeof(value));
}

static inline void wpk_write_binary(wpacket_t w,const void *value,uint32_t size)
{
	assert(value);
    if(!PACKET_RAW(w))
		wpk_write_uint32(w,size);
	wpk_write(w,(int8_t*)value,size);
}

static inline void wpk_write_string(wpacket_t w ,const char *value)
{
    wpk_write_binary(w,value,strlen(value)+1);
}

#endif
