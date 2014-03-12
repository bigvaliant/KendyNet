#include "lua_util.h"

void testObj(lua_State *L)
{
	if(0 != CALL_LUA_FUNC(L,"testObj",2))
	{
		const char * error = lua_tostring(L, -1);
		lua_pop(L,1);
		printf("%s\n",error);
		return;
	}
	{
		luaObject_t o = create_luaObj(L,-1);
		CALL_OBJ_FUNC(o,"show",0);
		SET_OBJ_FIELD(o,"type",lua_pushnumber,10);
		CALL_OBJ_FUNC(o,"show",0);
	}
	{
		luaObject_t o = create_luaObj(L,-2);
		CALL_OBJ_FUNC(o,"show",0);
		int type = GET_OBJ_FIELD(o,"type",int,lua_tonumber);
		printf("%d\n",type);
	}	
}

void testcall(lua_State *L)
{
	if(0 != CALL_LUA_FUNC1(L,"hello",0,
						     PUSH_TABLE2(L,PUSH_STRING(L,"haha"),
										PUSH_TABLE2(L,
												   PUSH_NUMBER(L,1),
												   PUSH_NUMBER(L,2)	
										))) 
							)
	{
		const char * error = lua_tostring(L, -1);
		lua_pop(L,1);
		printf("%s\n",error);
	}
}

void testArray(lua_State *L)
{
	if(0 != CALL_LUA_FUNC(L,"testArray",1))
	{
		const char * error = lua_tostring(L, -1);
		lua_pop(L,1);
		printf("%s\n",error);
		return;
	}
	int array[5];	
	GET_ARRAY(L,-1,array,lua_tonumber);
	int i = 0;
	for(; i < 5;++i)
		printf("%d\n",array[i]);	
}


int main()
{
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	if (luaL_dofile(L,"kendynet/test/test.lua")) {
		const char * error = lua_tostring(L, -1);
		lua_pop(L,1);
		printf("%s\n",error);
	}
	//testObj(L);
	//testcall(L);
	//testArray(L);
	luaObject_t obj = GETGLOBAL_OBJECT(L,"toredis");
	const char *ip = GET_OBJ_FIELD(obj,"ip",const char*,lua_tostring);
	printf("%s\n",ip);
	
	
	/*int a;	
	if(0 != CALL_LUA_FUNC1(L,"create_socket",1,PUSH_LUSRDATA(L,&a)))
	{
		const char * error = lua_tostring(L, -1);
		lua_pop(L,1);
		printf("create_socket:%s\n",error);
		return 0;
	}
	luaObject_t o = create_luaObj(L,-1);
	*/
	return 0;
}
