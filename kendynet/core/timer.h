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
#ifndef _TIMER_H
#define _TIMER_H
#include <time.h>
#include <stdint.h>
#include "minheap.h"

struct timer;
struct timer_item;

//如果返回1则timer_callback调用完之后会释放掉timer_item
typedef int8_t (*timer_callback)(struct timer*,struct timer_item*,void*);


struct timer *new_timer();
void   delete_timer(struct timer**);

//更新定时器
void update_timer(struct timer*,uint64_t now);
struct timer_item* register_timer(struct timer*,struct timer_item*,timer_callback,void *ud,uint64_t timeout);
void unregister_timer(struct timer_item **);    


/*    
#include "dlist.h"

//6级时间轮，最大到年，最小到豪秒
enum
{
    SEC = 0,
	MIN,
	HOUR,
	DAY,
	YEAR,
	SIZE,
};

struct timer;
struct timer_item
{
    struct dnode dlnode;
	void  *ud_ptr;
	void (*callback)(struct timer*,struct timer_item*,void*);
};

void   init_timer_item(struct timer_item*);
struct timer *new_timer();
void   delete_timer(struct timer**);
//更新定时器
void update_timer(struct timer*,time_t now);
int8_t register_timer(struct timer*,struct timer_item*,time_t timeout);
void unregister_timer(struct timer_item*);
*/

#endif
