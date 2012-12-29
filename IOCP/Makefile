all:
	gcc -O2 -c Acceptor.c Connector.c KendyNet.c link_list.c buffer.c Connection.c rpacket.c wpacket.c
	ar -rc iocp.a Acceptor.o Connector.o KendyNet.o link_list.o buffer.o Connection.o rpacket.o wpacket.o
client:iocp.a testclient.c
	gcc -O2 -o client.exe testclient.c iocp.a -lws2_32
server:iocp.a testserver.c
	gcc -O2 -o server.exe testserver.c iocp.a -lws2_32