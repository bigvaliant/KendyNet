#include "obj_allocator.h"
#include "buffer.h"
#include "common_define.h"
#include "log.h"
struct obj_block
{
	struct dnode node;
	struct llist freelist;
	msgque_t que;
	pthread_t       thdid;//·ÖÅäÏß³ÌµÄid
	char   buf[0];
};

struct obj_slot
{
	struct lnode node;
	struct obj_block *block;
	char buf[0];
};


struct pth_allocator
{
	msgque_t que;
	uint32_t free_block_size;
	struct dlist free_blocks;
	struct dlist recy_blocks;
};


struct obj_allocator{
	struct allocator base;
	uint32_t alloc_size;
	uint32_t objsize;
	uint16_t tls_type;
	pthread_key_t pkey;
};

static struct pth_allocator* new_pth(obj_allocator_t allo)
{
	struct pth_allocator *pth = calloc(1,sizeof(*pth));
	pth->que = new_msgque(64,NULL,0);
	dlist_init(&pth->recy_blocks);
	dlist_init(&pth->free_blocks);
	return pth;
}

static inline void __dealloc(obj_allocator_t _allo,struct pth_allocator *pth,struct obj_slot *obj)
{
		LLIST_PUSH_BACK(&obj->block->freelist,obj);
		uint32_t lsize = llist_size(&obj->block->freelist);
		if(unlikely(lsize == 1)){
				dlist_remove((struct dnode*)obj->block);//remove from _alloc->recy_blocks
				dlist_push(&pth->free_blocks,(struct dnode*)obj->block);	
				pth->free_block_size++;
		}
		else if(unlikely(lsize == _allo->alloc_size && pth->free_block_size > 1))
		{
			dlist_remove((struct dnode*)obj->block);//remove from _alloc->free_blocks
			pth->free_block_size--;
			free(obj->block);
		}
}


static inline void* __alloc(obj_allocator_t _allo,struct pth_allocator *pth)
{
	struct obj_block *b = (struct obj_block*)dlist_first(&pth->free_blocks);
	struct obj_slot *obj = LLIST_POP(struct obj_slot*,&b->freelist);
	if(unlikely(llist_is_empty(&b->freelist)))
	{
		//remove from _alloc->free_blocks and push to _alloc->recy_blocks
		dlist_remove((struct dnode*)b);
		dlist_push(&pth->recy_blocks,(struct dnode*)b);	
	}
	memset(obj->buf,0,_allo->objsize-sizeof(struct obj_slot));
	return (void*)obj->buf;
}

static inline void __expand(obj_allocator_t _allo,struct pth_allocator *pth)
{

	struct obj_block *b = calloc(1,sizeof(*b)+_allo->alloc_size*_allo->objsize);
	b->que = pth->que;
	b->thdid = pthread_self();
	llist_init(&b->freelist);
	uint32_t i = 0;
	for(; i < _allo->alloc_size;++i)
	{
		struct obj_slot *o = (struct obj_slot*)&b->buf[i*_allo->objsize];
		o->block = b;
		LLIST_PUSH_BACK(&b->freelist,o);
	}
	
	dlist_push(&pth->free_blocks,(struct dnode*)b);	
	++pth->free_block_size;
}

void* obj_alloc(struct allocator *allo,int32_t size)
{
	obj_allocator_t _allo = (obj_allocator_t)allo;
	struct pth_allocator *pth = (struct pth_allocator*)pthread_getspecific(_allo->pkey);
	//struct pth_allocator *pth = (struct pth_allocator*)tls_get(_allo->tls_type);
	if(unlikely(!pth))
	{
		pth = new_pth(_allo);
		//tls_set(_allo->tls_type,pth,NULL);

		pthread_setspecific(_allo->pkey,pth);
		
	}
	if(unlikely(dlist_empty(&pth->free_blocks)))
	{
		struct lnode *n;
		do{
			msgque_get(pth->que,&n,0);
			if(NULL != n)
				__dealloc(_allo,pth,(struct obj_slot*)n);
			else
				break;
		}while(1);
				
	}else
		return __alloc(_allo,pth);

	if(unlikely(dlist_empty(&pth->free_blocks)))
		__expand(_allo,pth);
	return __alloc(_allo,pth);
}



void obj_dealloc(struct allocator *allo ,void *ptr)
{
	obj_allocator_t _allo = (obj_allocator_t)allo;
	struct obj_slot *obj = (struct obj_slot*)((char*)ptr - sizeof(struct obj_slot));	
	if(obj->block->thdid == pthread_self()){

		struct pth_allocator *pth = (struct pth_allocator*)pthread_getspecific(_allo->pkey);;
		//	struct pth_allocator *pth = (struct pth_allocator*)tls_get(_allo->tls_type);
		if(unlikely(!pth))
			abort();
		__dealloc(_allo,pth,obj);
	}
	else
	{
		obj->node.next = NULL;
		msgque_put(obj->block->que,(struct lnode*)obj);
	}
}
	
allocator_t new_obj_allocator(uint32_t objsize,uint32_t initsize)
{
	obj_allocator_t allo = calloc(1,sizeof(*allo));
	objsize += sizeof(struct obj_slot);
    objsize = size_of_pow2(objsize);
    if(objsize < 64) objsize = 64;
	initsize = size_of_pow2(initsize);
	if(initsize < 1024) initsize = 1024;
	allo->alloc_size = initsize;
	allo->objsize = objsize;
	//allo->tls_type = tls_type;
    pthread_key_create(&allo->pkey,NULL);
	((allocator_t)allo)->_alloc = obj_alloc;
	((allocator_t)allo)->_dealloc = obj_dealloc;
	((allocator_t)allo)->_destroy = NULL;
	return (allocator_t)allo;

}
