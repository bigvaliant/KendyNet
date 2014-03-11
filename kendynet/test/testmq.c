#include <stdio.h>
#include <stdlib.h>
#include "core/kendynet.h"
#include "core/thread.h"
#include "core/atomic.h"
#include "core/systime.h"
#include "core/msgque.h"
#include "core/lockfree.h"
#include "core/ringque.h"

lnode *node_list1[5];
lnode *node_list2[5];
lnode *node_list3[5];
//msgque_t mq1;

ringque_t  mq1;


//lockfree_stack mq1;
void *Routine1(void *arg)
{
    for(;;){
        int j = 0;
        for(; j < 5;++j)
        {
            int i = 0;
            for(; i < 10000000; ++i)
            {
				//lfstack_push(&mq1,&node_list1[j][i]);
                //msgque_put(mq1,&node_list1[j][i]);
                while(ringque_push(mq1,&node_list1[j][i]) != 0);
                
            }
            //sleepms(100);
        }
    }
    printf("Routine1 end\n");
    return NULL;
}
/*
void *Routine2(void *arg)
{
    for(;;){
        int j = 0;
        for(; j < 5;++j)
        {
            int i = 0;
            for(; i < 10000000; ++i)
            {
				//lfstack_push(&mq1,&node_list2[j][i]);
                msgque_put(mq1,&node_list2[j][i]);
            }
            sleepms(200);
        }
    }
    printf("Routine2 end\n");
    return NULL;
}

void *Routine3(void *arg)
{
	for(;;){
		int j = 0;
		for(; j < 5;++j)
		{
			int i = 0;
			for(; i < 10000000; ++i)
			{
				msgque_put(mq1,&node_list3[j][i]);
				//lfstack_push(&mq1,&node_list3[j][i]);
			}
			sleepms(200);
		}
	}
	printf("Routine3 end\n");
	return NULL;
}*/

void *Routine4(void *arg)
{
	uint64_t count = 0;
	uint64_t total_count = 0;
	uint32_t tick = GetSystemMs();
	for( ; ; )
	{
		lnode *n = ringque_pop(mq1);// = lfstack_pop(&mq1);
		//if(0 != msgque_get(mq1,&n,50))
         //   break;
		if(n)
		{
			++count;
			++total_count;
		}
		uint32_t now = GetSystemMs();
		if(now - tick > 1000)
		{
			printf("recv:%d\n",(uint32_t)((count*1000)/(now-tick)));
			tick = now;
			count = 0;
		}
	}
    printf("Routine4 end\n");
    return NULL;
}

int main()
{
	int i = 0;
	for( ; i < 5; ++i)
	{
		node_list1[i] = calloc(10000000,sizeof(lnode));
		node_list2[i] = calloc(10000000,sizeof(lnode));
		node_list3[i] = calloc(10000000,sizeof(lnode));
	}
	//mq1 = new_msgque(32,NULL);
	//mq1.head = NULL;
	
	mq1 = new_ringque(10000000);
	
	thread_t t4 = create_thread(0);
	thread_start_run(t4,Routine4,NULL);

	thread_t t1 = create_thread(0);
	thread_start_run(t1,Routine1,NULL);

	//thread_t t2 = create_thread(0);
	//thread_start_run(t2,Routine2,NULL);

	//thread_t t3 = create_thread(0);
	//thread_start_run(t3,Routine3,NULL);

	getchar();
	return 0;
}

