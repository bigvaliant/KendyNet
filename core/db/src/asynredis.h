#ifndef _ASYNREDIS_H
#define _ASYNREDIS_H

#include "../asyndb.h"
#include "msgque.h"
#include "thread.h"
#include "hiredis.h"
#include "llist.h"
#include "sync.h"

struct asynredis
{
	struct asyndb   base;
	thread_t        worker;
	volatile int8_t stop; 
	redisContext   *context;
	char            ip[32];
	int32_t         port;	
	msgque_t        mq;
};

asyndb_t redis_new();
#endif
