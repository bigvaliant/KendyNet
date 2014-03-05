#include "togame.h"
#include "common/cmd.h"
#include "common/agentsession.h"

toGame_t g_togame = NULL;

void send2game(wpacket_t wpk)
{
	asyn_send(g_togame->togame,wpk);
}

int32_t togame_processpacket(msgdisp_t disp,rpacket_t rpk)
{
	uint16_t cmd = rpk_peek_uint16(rpk);
	if(cmd >= CMD_GAME2C && cmd < CMD_GAME2C_END){
		//转发到各agentservice
	}
	return 1;
}

