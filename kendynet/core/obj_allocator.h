#ifndef _OBJ_ALLOCATOR_H
#define _OBJ_ALLOCATOR_H

#include "allocator.h"
#include "msgque.h"
#include "llist.h"
#include "dlist.h"

struct obj_block
{
	struct dnode node;
	struct llist freelist;
	char   buf[0];
};

struct obj_slot
{
	union{
		pthread_t       thdid;//分配线程的id
		struct lnode node;
	};
	struct obj_block *block;
	char buf[0];
};


typedef struct obj_allocator{
	struct allocator base;
	msgque_t que;
	uint32_t alloc_size;
	uint32_t objsize;
	uint32_t free_block_size;
	struct dlist free_blocks;
	struct dlist recy_blocks;
	
}*obj_allocator_t;


allocator_t new_obj_allocator(uint32_t objsize,uint32_t initsize);


#endif
