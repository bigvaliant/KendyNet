#include <stdio.h>
#include <stdlib.h>
#include "asynnet/msgdisp.h"
#include <stdint.h>
#include "testcommon.h"
#include "core/db/asyndb.h"
#include "hiredis.h"

asyndb_t asydb;


int g = 0;
int count = 0;

void db_getcallback(struct db_result *result)
{
	if(result && result->result_set){
		redisReply *r = (redisReply*)result->result_set;	
		if(r->type != REDIS_REPLY_NIL){
			printf("age: %s\n",r->element[0]->str);
			printf("location: %s\n",r->element[1]->str);
		}
	}
}


static void *service_main(void *ud){
    msgdisp_t disp = (msgdisp_t)ud;
    while(!stop){
        msg_loop(disp,50);
    }
    return NULL;
}

int main(int argc,char **argv)
{
    setup_signal_handler();
    msgdisp_t disp1 = new_msgdisp(NULL,0);

    thread_t service1 = create_thread(THREAD_JOINABLE);

    asydb = new_asyndb(db_redis);
    asydb->connectdb(asydb,"127.0.0.1",6379);
    //发出第一个请求uu
    //char req[256];
    //snprintf(req,256,"set key%d %d",g,g);
    
    asydb->request(asydb,new_dbrequest("hmget gavin age location",db_getcallback,disp1,disp1));
    thread_start_run(service1,service_main,(void*)disp1);
    
    uint32_t tick,now;
    tick = now = GetSystemMs();
    while(!stop){
        sleepms(100);
        now = GetSystemMs();
        if(now - tick > 1000)
        {
            printf("count:%d\n",count);
            tick = now;
            count = 0;
        }
    }
    thread_join(service1);
    return 0;
}
