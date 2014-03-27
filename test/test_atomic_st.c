#include "core/thread.h"
#include "core/systime.h"
#include "core/atomic.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core/sync.h"
#include "core/atomic_st.h"

struct point
{
	volatile int x;
	volatile int y;
	volatile int z;
};


/*volatile int get_count;
volatile int set_count;
volatile int miss_count;


struct atomic_pointatomic_st{
	volatile int32_t version;
	struct point data;
};

typedef struct atomic_pointatomic_type
{
	uint32_t g_version;
	int32_t index;
	volatile struct atomic_pointatomic_st *ptr;
	struct atomic_pointatomic_st array[2];
	struct point (*get)(struct atomic_pointatomic_type*);
	void (*set)(struct atomic_pointatomic_type*,struct point VAL);
}atomic_point;

static inline struct point atomic_pointaotmic_get(struct atomic_pointatomic_type *at)
{
	struct point ret;
	while(1)
	{
		struct atomic_pointatomic_st *ptr_p = (struct atomic_pointatomic_st *)at->ptr;
		int save_version = ptr_p->version;
		ret = ptr_p->data;
        _FENCE;
		if(ptr_p == at->ptr && save_version == ptr_p->version)
			break;
		ATOMIC_INCREASE(&miss_count);
	}
	ATOMIC_INCREASE(&get_count);
	return ret;
}
static inline void atomic_pointatomic_staotmic_set(struct atomic_pointatomic_type *at,struct point v)
{
	struct atomic_pointatomic_st *new_p = &at->array[at->index];
	at->index ^= 0x1;
	new_p->data = v;
    _FENCE;
	new_p->version = ++at->g_version;
    _FENCE;
	at->ptr = new_p;
	ATOMIC_INCREASE(&set_count);
}
static inline struct atomic_pointatomic_type *atomic_point_new()
{
	struct atomic_pointatomic_type *at = calloc(1,sizeof(*at));
	at->index = 0;
	at->g_version = 0;
	at->ptr = NULL;
	at->get = atomic_pointaotmic_get;
	at->set = atomic_pointatomic_staotmic_set;
	return at;
}*/


DECLARE_ATOMIC_TYPE(atomic_point,struct point);

//GET_ATOMIC_ST(GetPoint,struct point);	
//SET_ATOMIC_ST(SetPoint,struct point);	
	
struct atomic_point *g_points[1000];

void *SetRotine(void *arg)
{
    int idx = 0;
    int pos = 0;
	while(1)
	{
		struct point p;
		++pos;
		p.x = p.y = p.z = pos+1;
		atomic_point_set(g_points[idx],p);
		//SetPoint(g_points[idx],&p);
		idx = (idx + 1)%100;
	}
}

void *GetRoutine(void *arg)
{
    int idx = 0;
	while(1)
	{
		struct point ret = atomic_point_get(g_points[idx]);
		//GetPoint(g_points[idx],&ret);
		if(ret.x != ret.y || ret.x != ret.z || ret.y != ret.z)
		{
			printf("%d,%d,%d\n",ret.x,ret.y,ret.z);
			assert(0);
		}			
		idx = (idx + 1)%100;
	}
}

int main()
{		
	struct point p;
	p.x = p.y = p.z = 1;
	for(int i = 0; i < 1000; ++i)
	{
		g_points[i] = atomic_point_new();
		atomic_point_set(g_points[i],p);
	}
	CREATE_THREAD_RUN(1,SetRotine,NULL);
	CREATE_THREAD_RUN(1,GetRoutine,(void*)1);	
    CREATE_THREAD_RUN(1,GetRoutine,(void*)2);
    //thread_t t4 = CREATE_THREAD_RUN(1,GetRoutine,(void*)3);
    //thread_t t5 = CREATE_THREAD_RUN(1,GetRoutine,(void*)4);
    //thread_t t6 = CREATE_THREAD_RUN(1,GetRoutine,(void*)5);
    //thread_t t7 = CREATE_THREAD_RUN(1,GetRoutine,(void*)6);
	uint32_t tick = GetSystemMs();
	while(1)
	{
		uint32_t new_tick = GetSystemMs();
		if(new_tick - tick >= 1000)
		{
			printf("get:%d,set:%d,miss:%d\n",get_count,set_count,miss_count);
			get_count = set_count = miss_count = 0;
			tick = new_tick;
		}
		sleepms(50);
	}
}
