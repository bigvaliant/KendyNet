#include "obj_allocator.h"
#include "buffer.h"

static inline void __dealloc(obj_allocator_t _allo,struct obj_slot *obj)
{
		LLIST_PUSH_BACK(&obj->block->freelist,obj);
		uint32_t lsize = llist_size(&obj->block->freelist);
		if(lsize == 1){
				dlist_remove((struct dnode*)obj->block);//remove from _alloc->recy_blocks
				dlist_push(&_allo->free_blocks,(struct dnode*)obj->block);	
		}
		else if(lsize == _allo->alloc_size && _allo->free_block_size > 1)
		{
			dlist_remove((struct dnode*)obj->block);//remove from _alloc->free_blocks
			_allo->free_block_size -= 1;
			free(obj->block);
		}
}


static inline void* __alloc(obj_allocator_t _allo)
{
	struct obj_block *b = (struct obj_block*)dlist_first(&_allo->free_blocks);
	struct obj_slot *obj = LLIST_POP(struct obj_slot*,&b->freelist);
	obj->block = b;
	if(llist_is_empty(&b->freelist))
	{
		//remove from _alloc->free_blocks and push to _alloc->recy_blocks
		dlist_remove((struct dnode*)b);
		dlist_push(&_allo->recy_blocks,(struct dnode*)b);	
	}
	return (void*)b->buf;

}

static inline void __expand(obj_allocator_t _allo)
{

	struct obj_block *b = calloc(1,sizeof(*b)+_allo->alloc_size*_allo->objsize);
	llist_init(&b->freelist);
	uint32_t i = 0;
	for(; i < _allo->alloc_size;++i)
	{
		struct obj_slot *o = (struct obj_slot*)&b->buf[i*_allo->objsize];
		LLIST_PUSH_BACK(&o->block->freelist,o);
	}
	dlist_push(&_allo->free_blocks,(struct dnode*)b);	
	++_allo->free_block_size;
}

void* obj_alloc(struct allocator *allo,int32_t size)
{
	obj_allocator_t _allo = (obj_allocator_t)allo;
	if(dlist_empty(&_allo->free_blocks))
	{
		struct lnode *n;
		do{
			msgque_get(_allo->que,&n,0);
		}while(n != NULL);
				
	}else
		return __alloc(_allo);

	if(dlist_empty(&_allo->free_blocks))
		__expand(_allo);
	return __alloc(_allo);
}



void obj_dealloc(struct allocator *allo ,void *ptr)
{
	obj_allocator_t _allo = (obj_allocator_t)allo;
	struct obj_slot *obj = (struct obj_slot*)((char*)ptr - sizeof(struct obj_slot));	
	if(obj->thdid == pthread_self())
		__dealloc(_allo,obj);
	else
		msgque_put(_allo->que,(struct lnode*)obj);
}
	
allocator_t new_obj_allocator(uint32_t objsize,uint32_t initsize)
{
	obj_allocator_t allo = calloc(1,sizeof(*allo));
	objsize += sizeof(struct obj_slot);
    objsize = size_of_pow2(objsize);
    if(objsize < 64) objsize = 64;
	initsize = size_of_pow2(initsize);
	if(initsize < 1024) initsize = 1024;
	allo->que = new_msgque(64,NULL);
	allo->alloc_size = initsize;
	allo->objsize = objsize;
	__expand(allo);

	dlist_init(&allo->recy_blocks);
	dlist_init(&allo->free_blocks);
	((allocator_t)allo)->_alloc = obj_alloc;
	((allocator_t)allo)->_dealloc = obj_dealloc;
	((allocator_t)allo)->_destroy = NULL;
	return (allocator_t)allo;

}
