#include "verifyservice.h"

static verfiyservice_t g_verifyservice = NULL;

static void *service_main(void *ud){
    while(!g_verifyservice->stop){
        msg_loop(g_verifyservice->msgdisp,50);
    }
    return NULL;
}

int32_t start_verifyservice(){
	//读取配置
	
	g_verifyservice= calloc(1,sizeof(*g_verifyservice));
	g_verifyservice->msgdisp = new_msgdisp(NULL,0);
	g_verifyservice->thd = create_thread(THREAD_JOINABLE);
	g_verifyservice->dbredis = new_asyndb(db_redis);
	
	/*
	 * TODO
	 * 建立到redis的连接
	*/
	
	thread_start_run(g_verifyservice->thd,service_main,NULL);
	return 0;
}

void stop_verifyservice(){
	g_verifyservice->stop = 1;
	thread_join(g_verifyservice->thd);
}
