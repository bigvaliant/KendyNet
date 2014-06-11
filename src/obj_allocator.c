#include "obj_allocator.h"
#include "kn_common_define.h"
#include "lockfree.h"
#include <pthread.h>
#include "hash_map.c"
#include "common_hash_function.h"

struct obj_block
{
	kn_dlist_node node;
	kn_list       freelist;
	lockfree_stack_t que;
	pthread_t       thdid;//·ÖÅäÏß³ÌµÄid
	char   buf[0];
};

struct obj_slot
{
	kn_list_node      node;
	struct obj_block *block;
	char buf[0];
};


struct pth_allocator
{
	lockfree_stack que;
	uint32_t   free_block_size;
	kn_dlist   free_blocks;
	kn_dlist   recy_blocks;
	uint32_t   free_memsize;
	hash_map_t recy;
};


struct obj_allocator{
	struct kn_allocator base;
	uint32_t alloc_size;
	uint32_t objsize;
	uint32_t reserve_size;
	pthread_key_t pkey;
};

struct recvlist{
	kn_list_node      node;
	kn_list           list;
};

static uint64_t _hash_func(void *key){
	pthread_t k = *(pthread_t*)key;
	return burtle_hash((uint8_t*)&k,sizeof(k),1);
}

static int32_t  _hash_key_eq(void *key1,void *key2){
	return (*(pthread_t*)key1) == (*(pthread_t*)key2) ? 0:-1;	
}

static struct pth_allocator* new_pth(obj_allocator_t allo)
{
	struct pth_allocator *pth = calloc(1,sizeof(*pth));
	kn_dlist_init(&pth->recy_blocks);
	kn_dlist_init(&pth->free_blocks);
	pth->recy = hash_map_create(64,sizeof(void*),sizeof(void*),_hash_func,_hash_key_eq);
	return pth;
}

static inline void __dealloc(obj_allocator_t _allo,struct pth_allocator *pth,struct obj_slot *obj)
{
		kn_list_pushback(&obj->block->freelist,(kn_list_node*)obj);
		uint32_t lsize = kn_list_size(&obj->block->freelist);
		pth->free_memsize += _allo->objsize;
		
		if(likely(lsize > 1 && lsize < _allo->alloc_size)) return;
		
		if(lsize == 1){
			kn_dlist_remove((kn_dlist_node*)obj->block);//remove from _alloc->recy_blocks
			kn_dlist_push(&pth->free_blocks,(kn_dlist_node*)obj->block);	
			pth->free_block_size++;
			return;
		}
		
		if(lsize == _allo->alloc_size && pth->free_memsize > _allo->reserve_size)
		{
			pth->free_memsize -= _allo->alloc_size*_allo->objsize;
			kn_dlist_remove((kn_dlist_node*)obj->block);//remove from _alloc->free_blocks
			pth->free_block_size--;
			free(obj->block);
		}
}

static inline void* __alloc(obj_allocator_t _allo,struct pth_allocator *pth)
{
	struct obj_block *b = (struct obj_block*)kn_dlist_first(&pth->free_blocks);
	struct obj_slot *obj = (struct obj_slot *)kn_list_pop(&b->freelist);
	if(unlikely(!kn_list_size(&b->freelist)))
	{
		//remove from _alloc->free_blocks and push to _alloc->recy_blocks
		kn_dlist_remove((kn_dlist_node*)b);
		kn_dlist_push(&pth->recy_blocks,(kn_dlist_node*)b);	
	}
	pth->free_memsize -= _allo->objsize;
	memset(obj->buf,0,_allo->objsize-sizeof(struct obj_slot));
	return (void*)obj->buf;
}


static __thread pthread_t threadid = 0;

static inline void __expand(obj_allocator_t _allo,struct pth_allocator *pth)
{
	struct obj_block *b = calloc(1,sizeof(*b)+_allo->alloc_size*_allo->objsize);
	b->que = &pth->que;
	if(unlikely(!threadid)) threadid = pthread_self();
	b->thdid = threadid;
	kn_list_init(&b->freelist);
	uint32_t i = 0;
	for(; i < _allo->alloc_size;++i)
	{
		struct obj_slot *o = (struct obj_slot*)&b->buf[i*_allo->objsize];
		o->block = b;
		kn_list_pushback(&b->freelist,(kn_list_node*)o);
	}
	pth->free_memsize += _allo->alloc_size*_allo->objsize;
	kn_dlist_push(&pth->free_blocks,(kn_dlist_node*)b);	
	++pth->free_block_size;
}

void* obj_alloc(struct kn_allocator *allo,int32_t _)
{
	(void)_;
	obj_allocator_t _allo = (obj_allocator_t)allo;
	struct pth_allocator *pth = (struct pth_allocator*)pthread_getspecific(_allo->pkey);
	if(unlikely(!pth))
	{
		pth = new_pth(_allo);
		pthread_setspecific(_allo->pkey,pth);
	}
	if(unlikely(kn_dlist_empty(&pth->free_blocks)))
	{
		struct recvlist *l = (struct recvlist*)lfstack_pop(&pth->que);
		if(l){
			do{
				struct obj_slot *obj;
				while((obj = (struct obj_slot*)kn_list_pop(&l->list)) != NULL){
					__dealloc(_allo,pth,obj);
				}
				free(l);
				l = (struct recvlist*)lfstack_pop(&pth->que);
			}while(l);
		}else
			__expand(_allo,pth);			
	}
	return __alloc(_allo,pth);
}

void obj_dealloc(struct kn_allocator *allo ,void *ptr)
{
	obj_allocator_t _allo = (obj_allocator_t)allo;
	struct obj_slot *obj = (struct obj_slot*)((char*)ptr - sizeof(struct obj_slot));
	if(unlikely(!threadid)) threadid = pthread_self();	
	if(obj->block->thdid == threadid){
		struct pth_allocator *pth = (struct pth_allocator*)pthread_getspecific(_allo->pkey);;
		if(unlikely(!pth))
			abort();
		__dealloc(_allo,pth,obj);
	}
	else{
		struct pth_allocator *pth = (struct pth_allocator*)pthread_getspecific(_allo->pkey);
		if(unlikely(!pth))
		{
			pth = new_pth(_allo);
			pthread_setspecific(_allo->pkey,pth);
		}
		hash_map_iter it = hash_map_find(pth->recy,(void*)&obj->block->thdid); 
		hash_map_iter end = hash_map_end(pth->recy);
		struct recvlist *l;
		if(IT_EQ(it,end)){
			l = calloc(1,sizeof(*l));
			kn_list_init(&l->list);
			hash_map_insert(pth->recy,(void*)&obj->block->thdid,(void*)&l);
		}else{
			l = (struct recvlist*)IT_GET_VAL(void*,it);
		}
		kn_list_pushback(&l->list,(kn_list_node*)obj);
		if(kn_list_size(&l->list) > 4096){
			hash_map_erase(pth->recy,it);
			lfstack_push(obj->block->que,(kn_list_node*)l);
		}
	}
}
	
kn_allocator_t new_obj_allocator(uint32_t objsize)
{
	obj_allocator_t allo = calloc(1,sizeof(*allo));
	objsize += sizeof(struct obj_slot);
    objsize = size_of_pow2(objsize);
    if(objsize < 64) objsize = 64;
	if(objsize < 256) allo->alloc_size = 8192/objsize;
	else allo->alloc_size = 32;
	
	if(objsize < 256)
		allo->reserve_size = (1024*1024*64)/objsize;
	else
		allo->reserve_size = objsize * allo->alloc_size * 32;
	allo->objsize = objsize;
    pthread_key_create(&allo->pkey,NULL);
	((kn_allocator_t)allo)->_alloc = obj_alloc;
	((kn_allocator_t)allo)->_dealloc = obj_dealloc;
	((kn_allocator_t)allo)->_destroy = NULL;
	return (kn_allocator_t)allo;

}
