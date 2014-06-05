#include "kn_timer.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "kn_time.h"
#include "kn_dlist.h"

typedef struct kn_timer{
	kn_dlist_node node;             //同一时间过期的timer被连接在一起
	uint64_t      ms;
	uint64_t      timeout;
	void          *ud;
	kn_cb_timer   timeout_callback;
	kn_timermgr_t mgr;
}*kn_timer_t;


struct timing_wheel{
	uint8_t  type;
	uint16_t curslot;
	uint16_t slotsize;
	//kn_dlist rereg;
	kn_dlist wheel[0]; 
};


enum{
	wheel_ms,   //1000
	wheel_sec,  //60
	wheel_min,  //60
	wheel_hour, //24
	wheel_day, //60 定时器最大计量60天内的时间
	wheel_max,
};

struct timing_wheel* new_timing_wheel(uint8_t type){
	struct timing_wheel *wheel;
	if(type == wheel_ms){
		wheel = calloc(1,sizeof(*wheel)*1000*sizeof(kn_dlist));
		wheel->slotsize = 1000;
	}else if(type == wheel_sec || type == wheel_min){
		wheel = calloc(1,sizeof(*wheel)*60*sizeof(kn_dlist));
		wheel->slotsize = 60;
	}else if(type == wheel_hour){
		wheel = calloc(1,sizeof(*wheel)*24*sizeof(kn_dlist));
		wheel->slotsize = 24;	
	}else if(type == wheel_day){
		wheel = calloc(1,sizeof(*wheel)*60*sizeof(kn_dlist));
		wheel->slotsize = 60;
	}else 
		return NULL;

	wheel->type = type;
	uint16_t i = 0;
	for(; i < wheel->slotsize; ++i){
		kn_dlist_init(&wheel->wheel[i]);
	}
	//kn_dlist_init(&wheel->rereg);
	return wheel;	
}

static void timing_wheel_add(struct timing_wheel *wheel,kn_timer_t timer){
	uint16_t index;
	uint64_t delta = timer->timeout - kn_systemms64();
	if(wheel->type == wheel_ms)
		index = wheel->curslot + (uint16_t)delta;
	else if(wheel->type == wheel_sec)
		index = wheel->curslot + (uint16_t)delta/1000;
	else if(wheel->type == wheel_min)
		index = wheel->curslot + (uint16_t)delta/1000/60;
	else if(wheel->type == wheel_hour)
		index = wheel->curslot + (uint16_t)delta/1000/60/60;
	else if(wheel->type == wheel_day)
		index = wheel->curslot + (uint16_t)delta/1000/60/60/24;
	assert(index < wheel->slotsize);
	kn_dlist_push(&wheel->wheel[index],(kn_dlist_node*)timer);

}


typedef struct kn_timermgr{
	struct timing_wheel *wheels[wheel_max];
	uint64_t      last_tick;
}*kn_timermgr_t;

static void _reg_timer(kn_timer_t timer){
	timer->timeout = timer->ms + kn_systemms64();
	if(timer->ms < timer->mgr->wheels[wheel_ms]->slotsize - timer->mgr->wheels[wheel_ms]->curslot)
		timing_wheel_add(timer->mgr->wheels[wheel_ms],timer);
	else if(timer->ms/1000 < timer->mgr->wheels[wheel_sec]->slotsize - timer->mgr->wheels[wheel_sec]->curslot)
		timing_wheel_add(timer->mgr->wheels[wheel_sec],timer);
	else if(timer->ms/1000/60 < timer->mgr->wheels[wheel_min]->slotsize - timer->mgr->wheels[wheel_min]->curslot)
		timing_wheel_add(timer->mgr->wheels[wheel_min],timer);
	else if(timer->ms/1000/60/60 < timer->mgr->wheels[wheel_hour]->slotsize - timer->mgr->wheels[wheel_hour]->curslot)
		timing_wheel_add(timer->mgr->wheels[wheel_hour],timer);
	else if(timer->ms/1000/60/60/24 < timer->mgr->wheels[wheel_day]->slotsize - timer->mgr->wheels[wheel_day]->curslot)
		timing_wheel_add(timer->mgr->wheels[wheel_day],timer);
	else
		assert(0);
}

static void tick_wheel(kn_timermgr_t t,struct timing_wheel *wheel){
	uint8_t parent = wheel_max;
	uint8_t child = wheel_max;
	if(wheel->type == wheel_ms){
		parent = wheel_sec;
	}else if(wheel->type == wheel_sec){
		parent = wheel_min;
		child = wheel_ms;
	}else if(wheel->type == wheel_min){
		parent = wheel_hour;
		child = wheel_sec;
	}else if(wheel->type == wheel_hour){
		parent = wheel_day;
		child = wheel_min;		
	}else if(wheel->type == wheel_day){
		child = wheel_min;			
	}

	if(wheel->type == wheel_ms){
		kn_dlist_node *c = kn_dlist_pop(&wheel->wheel[wheel->curslot]);
		while(c){
			kn_timer_t timer = (kn_timer_t)c;
			if(timer->timeout_callback(timer)){
				_reg_timer(timer);
				//kn_dlist_push(&wheel->rereg,c);
			}
			c = kn_dlist_pop(&wheel->wheel[wheel->curslot]);
		}
	}else{
		kn_dlist_node *c = kn_dlist_pop(&wheel->wheel[wheel->curslot]);
		while(c){
			timing_wheel_add(t->wheels[child],(kn_timer_t)c);
			c = kn_dlist_pop(&wheel->wheel[wheel->curslot]);
		}
	}

	if(wheel->type == wheel_day)
		wheel->curslot = (wheel->curslot+1)%wheel->slotsize;
	else{
		wheel->curslot += 1;
		if(wheel->curslot >= wheel->slotsize){
			wheel->curslot = 0;
			tick_wheel(t,t->wheels[parent]);
		}		
	}

	/*kn_dlist_node *c = kn_dlist_pop(&wheel->rereg);
	while(c){
		_reg_timer((kn_timer_t)c);
		c = kn_dlist_pop(&wheel->rereg);
	}*/

}

void kn_timermgr_tick(kn_timermgr_t t){
	uint64_t now =  kn_systemms64();
	uint64_t elapse = now - t->last_tick;
	while(elapse > 0){
		tick_wheel(t,t->wheels[wheel_ms]);
		elapse--;
	}
	t->last_tick = now;
}

static uint64_t MAX_TIMEOUT = (uint64_t)(60*24*3600*1000);

kn_timer_t    kn_reg_timer(kn_timermgr_t t,uint64_t timeout,kn_cb_timer cb,void *ud){
	if(timeout == 0 || timeout > MAX_TIMEOUT) return NULL;
	kn_timer_t timer = calloc(1,sizeof(*timer));
	timer->ud = ud;
	timer->timeout_callback = cb;
	timer->mgr = t;
	timer->ms = timeout;
	_reg_timer(timer);
	return timer;
}

void  kn_del_timer(kn_timer_t timer){
	if(timer->mgr)
		kn_dlist_remove((kn_dlist_node*)timer);
	free(timer);
}

void* kn_timer_getud(kn_timer_t timer){
	return timer->ud;
}

void kn_del_timermgr(kn_timermgr_t t){
	int i = 0;
	for(; i < wheel_max; ++i){
		int j = 0;
		for(; j < t->wheels[i]->slotsize; ++j){
			kn_dlist *l = &t->wheels[i]->wheel[j];
			kn_dlist_node *c = kn_dlist_pop(l);
			while(c){
				free(c);
				c = kn_dlist_pop(l);
			}
		}
		free(t->wheels[i]);
	}
	free(t);
}

kn_timermgr_t kn_new_timermgr(){
	kn_timermgr_t t = calloc(1,sizeof(*t));
	int i = 0;
	for(; i < wheel_max; ++i){
		t->wheels[i] = new_timing_wheel(i);
	}
	t->last_tick = kn_systemms64();
	return t;
}





