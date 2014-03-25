//#define _GNU_SOURCE
//#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include "core/kendynet.h"
#include "core/thread.h"
#include "core/atomic.h"
#include "core/systime.h"
#include "core/msgque.h"
#include "core/lockfree.h"
#include "core/ringque.h"

DECLARE_RINGQUE(rque_int32,int32_t);

rque_int32  mq1;


//lockfree_stack mq1;
void *Routine1(void *arg)
{
	//cpu_set_t mask;
	//CPU_ZERO(&mask);
    //CPU_SET(0, &mask);
    //if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
	//	fprintf(stderr, "set thread affinity failed\n");
    //}	
    for(;;){
        int j = 0;
        for(; j < 5;++j)
        {
            int i = 0;
            for(; i < 10000000; ++i)
            {				
                rque_int32_push(mq1,1);//GetSystemMs());
            }
        }
    }
    printf("Routine1 end\n");
    return NULL;
}

uint64_t count = 0;

void *Routine4(void *arg)
{
	//cpu_set_t mask;
	//CPU_ZERO(&mask);
    //CPU_SET(3, &mask);
    //if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
	//	fprintf(stderr, "set thread affinity failed\n");
    //}	
	for( ; ; )
	{
		int32_t ret;
		if(0 == rque_int32_pop(mq1,&ret))
		{
			++count;
		}
	}
    printf("Routine4 end\n");
    return NULL;
}

int main()
{
	
	mq1 = rque_int32_new(65536/2);
	
	thread_t t4 = create_thread(0);
	thread_start_run(t4,Routine4,NULL);

	thread_t t1 = create_thread(0);
	thread_start_run(t1,Routine1,NULL);
	uint32_t tick = GetSystemMs();
	for(;;){
		uint32_t now = GetSystemMs();
		if(now - tick > 1000)
		{
			printf("recv:%d\n",(uint32_t)((count*1000)/(now-tick)));
			tick = now;
			count = 0;
		}
		sleepms(1);
	}
	getchar();
	return 0;
}

