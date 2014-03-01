#ifndef _TLS_H
#define _TLS_H

#include <stdint.h>
#include <pthread.h>

#define MAX_TLS_SIZE 4096


enum{
	tls_wpacket_alloc = 0,
	tls_rpacket_alloc,
	tls_user_define,
};

//typedef void (*TLS_DESTROY_FN)(void*);
//int32_t tls_create(uint16_t key,TLS_DESTROY_FN);
void*    tls_get(uint16_t key);

int32_t  tls_set(uint16_t key,void*/*,TLS_DESTROY_FN*/);

#endif
