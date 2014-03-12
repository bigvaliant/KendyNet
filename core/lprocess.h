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
#ifndef _LPROCESS_H
#define _LPROCESS_H

#include "minheap.h"
#include "uthread.h"
#include "dlist.h"
#include "llist.h"

/*
* ���������̣��û����̣߳���������,��Ҫ����ʵ�ֶ��û��Ѻõ��첽rpc����
*/

//����������״̬
enum
{
    _YIELD = 0,
    _SLEEP,
    _ACTIVE,
    _DIE,
    _START,
    _BLOCK,
};

typedef struct lprocess
{
    struct lnode   next;
    struct dnode   dblink;
    struct heapele _heapele;
    uthread_t      ut;
    void          *stack;
    uint8_t        status;
    uint32_t       timeout;
    void          *ud;
    void* (*star_func)(void *);

}*lprocess_t;

typedef struct lprocess_scheduler
{
    lprocess_t lp;
    int32_t    stack_size;
    minheap_t  _timer;
    int32_t    size;
}*lprocess_scheduler_t;

//����һ��lprocess��ִ��start_fun
int32_t lp_spawn(void*(*start_fun)(void*),void*arg);

//�õ�ǰlprocess����ms����
void    lp_sleep(int32_t ms);

//��ǰlprocess�ó�ִ��Ȩ
void    lp_yield();

//������ǰlprocess(ֻ��������lprocess�����������)
void    lp_block();

void    lp_schedule();

#endif
