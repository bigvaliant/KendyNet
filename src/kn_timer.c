#include "kn_timer.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "kn_time.h"
#include "kn_dlist.h"

typedef struct kn_timer{
	kn_dlist_node node;             //同一时间过期的timer被连接在一起
	uint64_t      timeout;
	uint64_t      expire;
	void          *ud;
	kn_cb_timer   timeout_callback;
	kn_timermgr_t mgr;
}*kn_timer_t;


struct timing_wheel{
	uint8_t  type;
	uint16_t curslot;
	uint16_t slotsize;
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
	}else if(type == wheel_sec || type == wheel_min || type == wheel_day){
		wheel = calloc(1,sizeof(*wheel)*60*sizeof(kn_dlist));
		wheel->slotsize = 60;
	}else if(type == wheel_hour){
		wheel = calloc(1,sizeof(*wheel)*24*sizeof(kn_dlist));
		wheel->slotsize = 24;	
	}else 
		return NULL;

	wheel->curslot = 1;
	wheel->type = type;
	uint16_t i = 0;
	for(; i < wheel->slotsize; ++i){
		kn_dlist_init(&wheel->wheel[i]);
	}
	return wheel;	
}


static const uint64_t  T_SEC = 1000;
static const uint64_t  T_MIN = 1000*60;
static const uint64_t  T_HOUR = 1000*60*60;
static const uint64_t  T_DAY = 1000*60*60*24;


static void timing_wheel_add(struct timing_wheel *wheel,kn_timer_t timer){
	uint16_t index;
	uint64_t now = kn_systemms64();
	int64_t delta = 0;
	do{
		if(timer->expire > now)
			delta = (int64_t)(timer->expire - now);
		else 
			break;

		if(wheel->type == wheel_ms && delta >= 1)
			delta -= 1;
		else if(wheel->type == wheel_sec){
			if(delta >= T_SEC) delta -= T_SEC;
			if(delta < T_SEC) delta = 0;
			else delta = delta/T_SEC;
		}else if(wheel->type == wheel_min){
			if(delta >= T_MIN) delta -= T_MIN;
			if(delta < T_MIN) delta = 0;
			else delta = delta/T_MIN;
		}else if(wheel->type == wheel_hour){
			if(delta >= T_HOUR) delta -= T_HOUR;
			if(delta < T_HOUR) delta = 0;
			else delta = delta/T_HOUR;
		}else if(wheel->type == wheel_day){
			if(delta >= T_DAY) delta -= T_DAY;
			if(delta < T_DAY) delta = 0;
			else delta = delta/T_DAY;
		}
	}while(0);
	
	index = (wheel->curslot + (uint16_t)delta)%wheel->slotsize;
	//printf("%d,%d,%d\n",wheel->curslot,index,wheel->type);
	kn_dlist_push(&wheel->wheel[index],(kn_dlist_node*)timer);

}


typedef struct kn_timermgr{
	struct timing_wheel *wheels[wheel_max];
	kn_dlist      pending_reg;
	uint64_t      last_tick;
	uint8_t       intick;
}*kn_timermgr_t;

static inline void _reg_timer(kn_timer_t timer){
	if(timer->mgr->intick){
		kn_dlist_push(&timer->mgr->pending_reg,(kn_dlist_node*)timer);	
		return;
	}
	timer->expire = timer->timeout + kn_systemms64();
	if(timer->timeout <= timer->mgr->wheels[wheel_ms]->slotsize - timer->mgr->wheels[wheel_ms]->curslot)
		timing_wheel_add(timer->mgr->wheels[wheel_ms],timer);
	else if(timer->timeout/T_SEC < timer->mgr->wheels[wheel_sec]->slotsize - timer->mgr->wheels[wheel_sec]->curslot)
		timing_wheel_add(timer->mgr->wheels[wheel_sec],timer);
	else if(timer->timeout/T_MIN < timer->mgr->wheels[wheel_min]->slotsize - timer->mgr->wheels[wheel_min]->curslot)
		timing_wheel_add(timer->mgr->wheels[wheel_min],timer);
	else if(timer->timeout/T_HOUR < timer->mgr->wheels[wheel_hour]->slotsize - timer->mgr->wheels[wheel_hour]->curslot)
		timing_wheel_add(timer->mgr->wheels[wheel_hour],timer);
	else if(timer->timeout/T_DAY < timer->mgr->wheels[wheel_day]->slotsize - timer->mgr->wheels[wheel_day]->curslot)
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
			}/*else
				free(timer);*/
			
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

}

void kn_timermgr_tick(kn_timermgr_t t){
	uint64_t now =  kn_systemms64();
	uint64_t elapse = now - t->last_tick;
	t->intick = 1;
	while(elapse > 0){
		tick_wheel(t,t->wheels[wheel_ms]);
		elapse--;
	}
	t->intick = 0;
	kn_dlist_node *c = kn_dlist_pop(&t->pending_reg);
	while(c){
		_reg_timer((kn_timer_t)c);
		c = kn_dlist_pop(&t->pending_reg);
	}
	t->last_tick = now;
}

static uint64_t MAX_TIMEOUT = 5184000000;//60*24*3600*1000

kn_timer_t _kn_reg_timer(kn_timermgr_t t,uint64_t timeout,kn_cb_timer cb,void *ud){
	if(timeout == 0 || timeout > MAX_TIMEOUT) return NULL;
	kn_timer_t timer = calloc(1,sizeof(*timer));
	timer->ud = ud;
	timer->timeout_callback = cb;
	timer->mgr = t;
	timer->timeout = timeout;
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

	kn_dlist_node *c = kn_dlist_pop(&t->pending_reg);
	while(c){
		free(c);
		c = kn_dlist_pop(&t->pending_reg);
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
	kn_dlist_init(&t->pending_reg);
	return t;
}

