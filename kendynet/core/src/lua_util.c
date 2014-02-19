#include "lua_util.h"
#include "common_define.h"

void luaObjcet_destroy(void *ptr)
{
	luaObject_t o = (luaObject_t)ptr;
	luaL_unref(o->L,LUA_REGISTRYINDEX,o->rindex);
	printf("o->rindex = %d\n",o->rindex);	
	free(ptr);
	printf("luaObjcet_destroy\n");
}

luaObject_t create_luaObj(lua_State *L,int idx)
{
	luaObject_t o = calloc(1,sizeof(*o));
	ref_init((struct refbase*)o,type_luaobjcet,luaObjcet_destroy,1);
	o->L = L;
	lua_pushvalue(L,idx);
	o->rindex = luaL_ref(L,LUA_REGISTRYINDEX);
	printf("o->rindex = %d\n",o->rindex);
	if(LUA_REFNIL == o->rindex)
	{
		free(o);
		o = NULL;
	}
	return o;
}

void release_luaObj(luaObject_t o)
{
	ref_decrease((struct refbase*)o);
}
