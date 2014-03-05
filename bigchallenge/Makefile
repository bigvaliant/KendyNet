CFLAGS = -O2 -g -Wall 
LDFLAGS = -lpthread -lrt -lm -ltcmalloc
SHARED = -fPIC --shared
CC = gcc
INCLUDE = -Ikendynet -Ikendynet/core -I.. -Ikendynet/bigchallenge\
		  -Ikendynet/deps/luajit-2.0/src -Ikendynet/deps/hiredis
DEFINE = -D_DEBUG -D_LINUX -DMQ_HEART_BEAT

gateservice.a:\
		kendynet/bigchallenge/gateserver/agentservice/agentservice.h\
		kendynet/bigchallenge/gateserver/agentservice/agentservice.c
		$(CC) $(CFLAGS) -c $^ $(INCLUDE) $(DEFINE)
		ar -rc gateservice.a *.o
		rm -f *.o

