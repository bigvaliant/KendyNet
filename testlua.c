#include "lua/lua_util.h"

int luaTestFun(lua_State *L){
	return 0;
}

void *l_alloc(void *ud, void *ptr, size_t osize,
                                                size_t nsize) {
	(void)ud;  (void)osize;  /* not used */
	if (nsize == 0) {
		free(ptr);
		return NULL;
	}
	else
		return realloc(ptr, nsize);
}

int main(){
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	lua_register(L,"TestFun",&luaTestFun);
	lua_setallocf(L, l_alloc, NULL);
	if (luaL_dofile(L,"test.lua")) {
		const char * error = lua_tostring(L, -1);
		lua_pop(L,1);
		printf("%s\n",error);
	}
	return 0;
}
