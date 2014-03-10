#ifndef _PATHMAP_H
#define _PATHMAP_H
//寻路地图

//将相关接口函数注册到lua
void register_pathmap2lua(lua_State *L);

//从数据文件中导入地图阻挡数据
void load_pathmap();


#endif
