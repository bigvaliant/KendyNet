#ifndef _MAP_H
#define _MAP_H

#include "core/lua_util.h"

void map_register2lua(lua_State *L);

//在进程启动时调,初始化地图配置信息
void map_init();

#endif
