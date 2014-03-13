#include "../asyndb.h"
#include "asynredis.h"
#include "asynnet/asynnet.h"

void asyndb_sendresult(msgdisp_t recver,db_result_t result)
{
	if(0 != send_msg(NULL,recver,(msg_t)result))
		free_dbresult(result);
}

void request_destroyer(void *ptr)
{
	free_dbrequest((db_request_t)ptr);
}

asyndb_t new_asyndb(uint8_t dbtype)
{
	if(dbtype == db_redis){
		return redis_new();
	}else
		return NULL;
}

void free_asyndb(asyndb_t asyndb)
{
	asyndb->destroy_function(asyndb);
} 

db_result_t new_dbresult(uint8_t dbtype,void *result_set,DB_CALLBACK callback,void *ud)
{
	db_result_t result = calloc(1,sizeof(*result));
	MSG_TYPE(result) = MSG_DB_RESULT;
	result->dbtype = dbtype;
	result->callback = callback;
	result->ud = ud;
	result->result_set = result_set;
	return result;
}


void free_dbresult(db_result_t result)
{
	redisReply *r = (redisReply*)result->result_set;
	if(r) freeReplyObject(r);
	free(result);
}

db_request_t  new_dbrequest(const char *req_string,DB_CALLBACK callback,void *ud,msgdisp_t sender)
{
	if(!req_string) return NULL;
	db_request_t request = calloc(1,sizeof(*request));
	request->callback = callback;
	request->sender = sender;
	request->query_str = new_string(req_string);
	request->ud = ud;
	return request;
}

void free_dbrequest(db_request_t req)
{
	release_string(req->query_str);
	free(req);
}
