#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>  
#include "core/singleton.h"
#include "core/thread.h"

typedef struct testst{
    int32_t value;
}testst;

testst *create_test(){
    printf("create_test\n");
    testst *t = calloc(1,sizeof(*t));
    t->value = 100;
    return t;
}

void destroy_test(void *ud){
	printf("destroy_test\n");
	free(ud);
}

DECLARE_SINGLETON(testst);
IMPLEMENT_SINGLETON(testst,create_test,destroy_test);

void *Routine1(void *arg)
{
    testst *t = GET_INSTANCE(testst);
    printf("%d\n",t->value);
    printf("Routine1 end\n");
    return NULL;
}


int main()
{
    testst *t = GET_INSTANCE(testst);
    printf("%d\n",t->value);
    GET_INSTANCE(testst);
    thread_t t1 = create_thread(0);
    thread_start_run(t1,Routine1,NULL);
    getchar();
    return 0;
}    