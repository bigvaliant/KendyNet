#ifndef _PATHFINDING_H
#define _PATHFINDING_H
#include "core/lua_util.h"
//寻路地图

//将相关接口函数注册到lua
void register_pathfinding2lua(lua_State *L);

//从数据文件中导入地图阻挡数据
void load_pathfinding();


#endif
