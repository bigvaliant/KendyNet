#include "kn_timer.h"
#include "kn_time.h"


struct userst{
	uint64_t expecttime;
	uint64_t ms;
};
kn_timermgr_t timermgr=NULL;

int timer_callback(kn_timer_t timer){
	struct userst *st = (struct userst*)kn_timer_getud(timer);
	printf("%d,%d\n",kn_systemms64()-st->expecttime,st->ms);
	st->expecttime = kn_systemms64() + st->ms;
	kn_del_timer(timer);
	kn_reg_timer(timermgr,st->ms,timer_callback,st);
	return 0;
}


int main(){
	
	timermgr = kn_new_timermgr();
	
	struct userst st1,st2,st3;
	st1.ms = 50;
	st1.expecttime = kn_systemms64()+st1.ms;
	
	st2.ms = 100;
	st2.expecttime = kn_systemms64()+st2.ms;
	
	st3.ms = 60*1000;//+59*1000;
	st3.expecttime = kn_systemms64()+st3.ms;
	
//	kn_reg_timer(timermgr,st1.ms,timer_callback,&st1);
//	kn_reg_timer(timermgr,st2.ms,timer_callback,&st2);
	kn_reg_timer(timermgr,st3.ms,timer_callback,&st3);
	
	
	while(1){
		kn_timermgr_tick(timermgr);
		kn_sleepms(50);	
	}
	return 0;
}
