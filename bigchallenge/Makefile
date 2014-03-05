CFLAGS = -O2 -g -Wall 
LDFLAGS = -lpthread -lrt -lm -ltcmalloc
SHARED = -fPIC --shared
CC = gcc
INCLUDE = -I../kendynet -I../kendynet/core -I.. -I. -Igateserver\
		  -I../kendynet/deps/luajit-2.0/src -I../kendynet/deps/hiredis
DEFINE = -D_DEBUG -D_LINUX -DMQ_HEART_BEAT

gateservice.a:\
		gateserver/agentservice/agentservice.c\
		gateserver/agentservice/agentservice.h\
		gateserver/togame/togame.c\
		gateserver/togame/togame.h
		$(CC) $(CFLAGS) -c $^ $(INCLUDE) $(DEFINE)
		ar -rc gateservice.a *.o
		rm -f *.o

