#include "kendynet.h"


struct userst{
	uint64_t expecttime;
	uint64_t ms;
};
kn_proactor_t proactor=NULL;

int timer_callback(kn_timer_t timer){
	struct userst *st = (struct userst*)kn_timer_getud(timer);
	printf("%d,%d\n",kn_systemms64()-st->expecttime,st->ms);
	st->expecttime = kn_systemms64() + st->ms;
	return 1;
}


int main(){
	
	proactor = kn_new_proactor();
	
	struct userst st1,st2,st3;
	st1.ms = 100;
	st1.expecttime = kn_systemms64()+st1.ms;
	
	st2.ms = 10*60*1000;
	st2.expecttime = kn_systemms64()+st2.ms;
	
	st3.ms = 3600*1000;//+59*1000;
	st3.expecttime = kn_systemms64()+st3.ms;
	
	kn_reg_timer(proactor,st1.ms,timer_callback,&st1);
	kn_reg_timer(proactor,st2.ms,timer_callback,&st2);
	kn_reg_timer(proactor,st3.ms,timer_callback,&st3);
	
	
	while(1){
		kn_proactor_run(proactor,1);	
	}
	return 0;
}
