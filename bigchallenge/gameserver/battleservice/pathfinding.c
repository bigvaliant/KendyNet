#include "pathfinding.h"
#include <math.h>
#include <assert.h>
#include "game/astar.h"

enum{
	BLOCK = 65536,
	LAND  = 5,   //平地
	SWAMP = 15,  //沼泽
	WATER = 50,  //水域
};

struct map;
struct node
{
	struct map_node _base;
	struct map      *_map;
	uint16_t         weights;
	int    x; 
	int    y;	
};

struct map
{
	struct node **_map;
	int    max_x;
	int    max_y;
};

int direction[8][2] = {
	{0,-1},//上
	{0,1},//下
	{-1,0},//左
	{1,0},//右
	{-1,-1},//左上
	{1,-1},//右上
	{-1,1},//左下
	{1,1},//右下
};

static  inline struct node *get_node_by_xy(struct map *map,int x,int y)
{
	if(x < 0 || x >= map->max_x || y < 0 || y >= map->max_y)
		return NULL;
	return map->_map[y*map->max_x+x];
}

//获得当前node的8个临近节点,如果是阻挡点会被忽略
static struct map_node** _get_neighbors(struct map_node *mnode)
{
	struct map_node **ret = (struct map_node **)calloc(9,sizeof(*ret));
	struct node *_node = (struct node*)mnode;
	struct map *_map = _node->_map;
	int32_t i = 0;
	int32_t c = 0;
	for( ; i < 8; ++i)
	{
		int x = _node->x + direction[i][0];
		int y = _node->y + direction[i][1];
		struct node *tmp = get_node_by_xy(_map,x,y);
		if(tmp && tmp->weights != BLOCK)
			ret[c++] = (struct map_node*)tmp;
	}
	ret[c] = NULL;
	return ret;
}
//计算到达相临节点需要的代价
static double _cost_2_neighbor(struct node *from,struct node *to)
{
	double cost;
	int delta_x = ((struct node*)from)->x - ((struct node*)to)->x;
	int delta_y = ((struct node*)from)->y - ((struct node*)to)->y;
	if(abs(delta_x) > 1 && abs(delta_y) > 1){
		//斜向
		cost = (double)((struct node*)to)->weights;
		cost *= 1.25f;		
	}else
		cost = (double)((struct node*)to)->weights; 
	return cost;
}

//用平地路径做估算值
static double _cost_2_goal(struct path_node *from,struct path_node *to)
{
	int delta_x = abs(((struct node*)from)->x - ((struct node*)to)->x);
	int delta_y = abs(((struct node*)from)->y - ((struct node*)to)->y);
	return (delta_x * LAND) + (delta_y * LAND);
}

struct map* create_map(uint16_t *array,int max_x,int max_y)
{
	struct map *_map = calloc(1,sizeof(*_map));
	_map->max_x = max_x;
	_map->max_y = max_y;
	_map->_map = (struct node**)calloc(max_x*max_y,sizeof(struct node*));
	int i = 0;
	int j = 0;
	for( ; i < max_y; ++i)
	{
		for(j = 0; j < max_x;++j)
		{		
			_map->_map[i*max_x+j] = calloc(1,sizeof(struct node));
			struct node *tmp = _map->_map[i*max_x+j];
			tmp->_map = _map;
			tmp->x = j;
			tmp->y = i;
			tmp->weights = array[i*max_x+j];			
		}
	}
	return _map;
}

void destroy_map(struct map **map)
{
	free((*map)->_map);
	free(*map);
}

struct AstarMap{
	struct map *map;
	struct A_star_procedure *astar;
};


int luaCreateAstarMap(lua_State *L){
	return 1;
}

int luaDestroyAstarMap(lua_State *L){
	return 0;
}

//获取从源到目标的一条路径，如果不能通达返回空表，否则返回一条路径表
int luaGetPath(lua_State *L){	
	struct AstarMap *astar = (struct AstarMap *)lua_touserdata(L,-1);
	int to_x = (int)lua_tonumber(L,-2);
	int to_y = (int)lua_tonumber(L,-3);
	int from_x = (int)lua_tonumber(L,-4);
	int from_y = (int)lua_tonumber(L,-5);
	
	struct map_node *from = (struct map_node*)get_node_by_xy(astar->map,from_x,from_y);
	struct map_node *to = (struct map_node*)get_node_by_xy(astar->map,to_x,to_y);
	struct path_node *path = find_path(astar->astar,from,to);
	if(!path)
	{
		lua_pushnil(L);
		return 1;
	}
	lua_newtable(L);
	int i = 1;
	while(path)
	{
		struct node *mnode = (struct node*)path->_map_node;
		PUSH_TABLE2(L,lua_pushnumber(L,mnode->x),lua_pushnumber(L,mnode->y));
		lua_rawseti(L,-2,i++);		
		path = path->parent;
	}	
	return 1;
}

void register_pathmap2lua(lua_State *L)
{
	lua_register(L,"CreateAstarMap",&luaCreateAstarMap);
	lua_register(L,"DestroyAstarMap",&luaDestroyAstarMap);
	lua_register(L,"GetPath",&luaGetPath);
}
