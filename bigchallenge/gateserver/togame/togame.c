#include "togame.h"

toGame_t g_togame = NULL;

void send2game(wpacket_t wpk)
{
	asyn_send(g_togame->togame,wpk);
}

int32_t togame_processpacket(msgdisp_t disp,rpacket_t rpk)
{
	uint16_t cmd = rpk_peek_uint16(rpk);
	if(cmd >= CMD_GAME2CLIENT && cmd < CMD_GAME2CLIENT_END){
		uint16_t size = reverse_read_uint16(rpk);//这个包需要发给多少个客户端
		//创建一个rpacket_t用于读取需要广播的客户端
		rpacket_t r = rpk_create_skip(rpk,size*sizeof(agentsession)+sizeof(size));
		//将rpk中用于广播的信息丢掉
		rpk_dropback(rpk,size*sizeof(agentsession)+sizeof(size));
		int i = 0;
		wpacket_t wpk = wpk_create_by_rpacket(rpk);
		//发送给所有需要接收的客户端
		for( ; i < size; ++i)
		{
			agentsession session;
			session.data = rpk_read_uint32(r);
			agentplayer_t ply = get_agentplayer(service,session);
			if(ply)	
				asyn_send(ply->con,wpk);
		}
		rpk_destroy(&r);
	}
	return 1;
}

