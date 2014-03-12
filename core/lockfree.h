#ifndef _LOCKFREE_H
#define _LOCKFREE_H

#include "llist.h"
#include "atomic.h"

typedef struct lockfree_stack
{
	struct lnode *head;
}lockfree_stack,*lockfree_stack_t;

static inline void lfstack_push(lockfree_stack_t ls,struct lnode *n)
{
		while(1)
		{
			struct lnode *lhead = ls->head;
			n->next = lhead;
			if(COMPARE_AND_SWAP(&ls->head,lhead,n))
				break;
		}
}

static inline struct lnode* lfstack_pop(lockfree_stack_t ls)
{
		while(1)
		{
			struct lnode *lhead = ls->head;
			if(!lhead) return NULL;
			struct lnode *ret = ls->head;				
			if(COMPARE_AND_SWAP(&ls->head,lhead,ls->head->next))
			{
				ret->next = NULL;
				return ret;
			}
		}
}

#endif
