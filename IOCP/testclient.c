#include <stdio.h>
#include <winsock2.h>
#include <WinBase.h>
#include <Winerror.h>
#include "KendyNet.h"
#include "Connector.h"
#include "Connection.h"
#include <stdint.h>
static uint32_t connect_count = 0;
uint32_t packet_recv = 0;
uint32_t packet_send = 0;
uint32_t send_request = 0;
uint32_t tick = 0;
uint32_t now = 0;
uint32_t bf_count = 0;
#define MAX_CLIENT 1000
static struct connection *clients[MAX_CLIENT];
uint32_t last_recv = 0;
uint32_t ava_interval = 0;
uint32_t s_p = 0;
void init_clients()
{
	uint32_t i = 0;
	for(; i < MAX_CLIENT;++i)
		clients[i] = 0;
}

void add_client(struct connection *c)
{
	uint32_t i = 0;
	for(; i < MAX_CLIENT; ++i)
	{
		if(clients[i] == 0)
		{
			clients[i] = c;
			break;
		}
	}
}

void remove_client(struct connection *c)
{
	uint32_t i = 0;
	for(; i < MAX_CLIENT; ++i)
	{
		if(clients[i] == c)
		{
			clients[i] = 0;
			break;
		}
	}
}

connector_t con = NULL;
uint32_t iocp_count = 0;

void on_process_packet(struct connection *c,rpacket_t r)
{
	uint32_t s = rpacket_read_uint32(r);
	uint32_t t;
	//connector_run(con,1);
	if(s == c->socket.sock)
	{
		t = rpacket_read_uint32(r);
		ava_interval += GetTickCount() - t;
		ava_interval /= 2;
	}
	++packet_recv;
	/*wpacket_t wpk = wpacket_create(64,0);
	wpacket_write_uint32(wpk,(uint32_t)s);
	wpacket_write_uint32(wpk,GetTickCount());
	wpacket_write_string(wpk,"hello kenny");
	connection_send(c,wpk,0);	
	*/
	rpacket_destroy(&r);
	
}

void on_connect_callback(SOCKET s,const char *ip,uint32_t port,void *ud)
{
	uint32_t err_code = 0;
	HANDLE *iocp = (HANDLE*)ud;
	uint32_t ul = 1;
	BOOL                         optval=1;
	struct connection *c;
	wpacket_t wpk;
	++connect_count;
	if(s == INVALID_SOCKET)
	{
		printf("%d,连接到:%s,%d,失败\n",s,ip,port);
	}
	else
	{
		printf("%d,连接到:%s,%d,成功\n",s,ip,port);
		ioctlsocket(s,FIONBIO,(uint32_t*)&ul);
		//setsockopt(s,IPPROTO_TCP,TCP_NODELAY,(char*)&optval,sizeof(optval));         //不采用延时算法 

		c = connection_create(s,0,on_process_packet,remove_client);
		add_client(c);
		Bind2Engine(*iocp,(Socket_t)c);
		wpk = wpacket_create(64,0);
		wpacket_write_uint32(wpk,(uint32_t)s);
		wpacket_write_uint32(wpk,GetTickCount());
		wpacket_write_string(wpk,"hello kenny");
		connection_send(c,wpk,0);
		connection_recv(c);
	}
}

int main(int32_t argc,char **argv)
{	
	HANDLE iocp;
	const char *ip = argv[1];
	uint32_t port = atoi(argv[2]);
	int32_t client_count = atoi(argv[3]);
	int32_t ret;
	int32_t i = 0;
	uint32_t send_interval = 8;
	uint32_t send_tick = 0;
	wpacket_t wpk;
	init_wpacket_pool(500000);
	init_rpacket_pool(10000000);
	buffer_init_maxbuffer_size(2000);
	buffer_init_64(2000);
	InitNetSystem();
	init_clients();
	iocp = CreateNetEngine(1);
	con =  connector_create();
	for( ; i < client_count;++i)
	{
		ret = connector_connect(con,ip,port,on_connect_callback,&iocp,1000*20);
		Sleep(1);
	}
	//while(connect_count < 1)
	//	connector_run(c,0);
	while(1)
	{
		connector_run(con,1);
		RunEngine(iocp,50);
		now = GetTickCount();
		if(now - tick > 1000)
		{
			printf("recv:%u,send:%u,s_req:%u,ava_interval:%u,bf:%u\n",packet_recv,packet_send,send_request,ava_interval,bf_count);
			tick = now;
			packet_recv = 0;
			packet_send = 0;
			send_request = 0;
			ava_interval = 0;
		}
		if(ava_interval > 200)
			send_interval = 200;
		else
			send_interval = 8;
		if(now - send_tick > send_interval)
		{
			send_tick = now;
			for(i = 0; i < MAX_CLIENT; ++i)
			{
				if(clients[i])
				{
					wpk = wpacket_create(64,0);
					wpacket_write_uint32(wpk,clients[i]->socket.sock);
					wpacket_write_uint32(wpk,GetTickCount());
					wpacket_write_string(wpk,"hello kenny");
					connection_send(clients[i],wpk,0);
				}
			}
		}
	}

	getchar();
	return 0;
}
