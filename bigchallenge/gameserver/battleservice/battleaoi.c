#include "battleaoi.h"
#include "core/tls.h"
#include "common/tls_define.h"
#include <assert.h>

//只用最简单的网格判断，再视野网格内的对象都认为是进入范围
static uint8_t in_myscope(struct aoi_object *self,struct aoi_object *other)
{
	return 1;
}
	
static void cb_enter(struct aoi_object *self,struct aoi_object *other)
{
	luaObject_t lua_self = (luaObject_t)self->ud;
	luaObject_t lua_other = (luaObject_t)other->ud;
	assert(lua_self->L == lua_other->L);
	CALL_OBJ_FUNC1(lua_self,"EnterView",0,PUSH_LUAOBJECT(lua_self->L,lua_other));
}
	
static void cb_leave(struct aoi_object *self,struct aoi_object *other)
{
	luaObject_t lua_self = (luaObject_t)self->ud;
	luaObject_t lua_other = (luaObject_t)other->ud;
	assert(lua_self->L == lua_other->L);
	CALL_OBJ_FUNC1(lua_self,"LeaveView",0,PUSH_LUAOBJECT(lua_self->L,lua_other));	
}

int luaNewAoiObj(lua_State *L){
	luaObject_t self = create_luaObj(L,-1);
	struct aoi_object *aoi_obj = calloc(1,sizeof(*aoi_obj));
	aoi_obj->ud = self;
	aoi_obj->in_myscope = in_myscope;
	aoi_obj->cb_enter = cb_enter;
	aoi_obj->cb_leave = cb_leave;
	PUSH_LUSRDATA(L,aoi_obj);
	return 1;
}

int luaDelAoiObj(lua_State *L){
	struct aoi_object *aoi_obj = lua_touserdata(L,-1);
	release_luaObj(aoi_obj);
	return 0;
}

int luaNewAoiMap(lua_State *L){
	return 1;
}

int luaDestroyAoiMap(lua_State *L){
	return 0;
}

int luaAoiEnterMap(lua_State *L){
	return 1;
}

int luaAoiLeaveMap(lua_State *L){
	return 1;
}

int luaAoiMoveTo(lua_State *L){
	return 0;
}

void register_aoi2lua(lua_State *L){     
	lua_register(L,"NewAoiObj",&luaNewAoiObj);
	lua_register(L,"DelAoiObj",&luaDelAoiObj);
	lua_register(L,"NewAoiMap",&luaNewAoiMap);
	lua_register(L,"DestroyAoiMap",&luaDestroyAoiMap);
	lua_register(L,"AoiEnterMap",&luaAoiEnterMap);
	lua_register(L,"AoiLeaveMap",&luaAoiLeaveMap);
	lua_register(L,"AoiMoveTo",&luaAoiMoveTo);
}
