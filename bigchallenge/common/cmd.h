#ifndef _CMD_H
#define _CMD_H

/*
*   命令码的定义
*/

enum{
	//客户端到网关
	CMD_C2GATE = 0,

	CMD_C2GATE_LOGIN,   //玩家登录
	CMD_C2GATE_CREATE,  //创建角色
	
	CMD_C2GATE_END,

	//客户端到游戏服务器
	CMD_C2GAME = 100,

	CMD_C2GAME_MOVE,   //客户端移动请求
	
	CMD_C2GAME_END,
	
	
	//网关到客户端
	CMD_GATE2C = 200,
	
	CMD_GATE2C_BUSY, //服务器繁忙
	
	CMD_GATE2_END,
	
	//游戏服到客户端	
	CMD_GAME2C = 300,

	CMD_GAME2C_ENTERVIEW, //对象进入视野
	CMD_GAME2C_LEAVEVIEW, //对象离开视野
	
	CMD_GAME2C_END,

	//网关到游戏服
	CMD_GATE2GAME = 400
	
	CMD_GATE2GAME_CDISCONNECT, //客户端连接断开
	
	CMD_GATE2GAME_END,

	//游戏服到网关
	CMD_GAME2GATE = 500,
	CMD_GAME2GATE_END,
	

};

struct rpacket;
typedef void (*cmd_handler)(struct rpacket*);

#endif
