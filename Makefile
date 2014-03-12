CFLAGS = -O2 -g -Wall 
LDFLAGS = -lpthread -lrt -lm -ltcmalloc
SHARED = -fPIC --shared
CC = gcc
INCLUDE =  -Icore -I.\
		   -Ideps/luajit-2.0/src -Ideps/hiredis
DEFINE = -D_DEBUG -D_LINUX -DMQ_HEART_BEAT
TESTDIR = test

kendynet.a: \
		   core/src/buffer.c \
		   core/buffer.h \
		   core/src/connection.c \
		   core/connection.h \
		   core/src/poller.c \
		   core/src/epoll.c \
		   core/src/except.c \
		   core/src/kendynet.c \
		   core/src/msgque.c \
		   core/msgque.h \
		   core/src/netservice.c \
		   core/netservice.h \
		   core/src/rbtree.c \
		   core/src/rpacket.c \
		   core/rpacket.h \
		   core/src/socket.c \
		   core/src/sock_util.c \
		   core/src/spinlock.c \
		   core/src/systime.c \
		   core/src/thread.c \
		   core/src/timer.c \
		   core/src/uthread.c \
		   core/src/refbase.c \
		   core/src/log.c \
		   core/log.h \
		   core/asynnet/src/asynnet.c \
		   core/asynnet/src/asynsock.c \
		   core/asynnet/src/msgdisp.c \
		   core/asynnet/src/asyncall.c \
		   core/src/atomic_st.c \
		   core/src/tls.c \
		   core/src/obj_allocator.c \
		   core/src/lua_util.c\
		   core/db/src/asynredis.c\
		   core/db/src/asyndb.c\
		   core/src/lua_util.c\
		   core/src/kn_string.c\
		   core/src/hash_map.c\
		   core/src/minheap.c\
		   core/src/lookup8.c\
		   core/wpacket.h\
		   core/src/wpacket.c
		$(CC) $(CFLAGS) -c $^ $(INCLUDE) $(DEFINE)
		ar -rc kendynet.a *.o
		rm -f *.o
game.a:\
		game/src/astar.c\
		game/src/aoi.c
		$(CC) $(CFLAGS) -c $^ $(INCLUDE) $(DEFINE)
		ar -rc game.a *.o
		rm -f *.o	
singleton:kendynet.a $(TESTDIR)/testsingleton.c
	$(CC) $(CFLAGS) -o singleton $(TESTDIR)/testsingleton.c kendynet.a $(INCLUDE) $(LDFLAGS) $(DEFINE)
testaoi:kendynet.a game.a $(TESTDIR)/testaoi.c $(TESTDIR)/testcommon.h
	$(CC) $(CFLAGS) -o testaoi $(TESTDIR)/testaoi.c kendynet.a game.a $(INCLUDE) $(LDFLAGS) $(DEFINE) 		
8puzzle:kendynet.a $(TESTDIR)/8puzzle.c $(TESTDIR)/testcommon.h
	$(CC) $(CFLAGS) -o 8puzzle $(TESTDIR)/8puzzle.c kendynet.a $(INCLUDE) $(LDFLAGS) $(DEFINE) 	
testmaze:kendynet.a $(TESTDIR)/testmaze.c $(TESTDIR)/testcommon.h
	$(CC) $(CFLAGS) -o testmaze $(TESTDIR)/testmaze.c kendynet.a $(INCLUDE) $(LDFLAGS) $(DEFINE) 
luanet:luanet.c kendynet.a
	$(CC) $(CFLAGS) -c $(SHARED) luanet.c $(INCLUDE) $(DEFINE) 
	$(CC) $(SHARED) -o luanet.so luanet.o kendynet.a $(LDFLAGS) $(DEFINE)
	rm -f *.o
testredis:kendynet.a $(TESTDIR)/testredis.c $(TESTDIR)/testcommon.h
	$(CC) $(CFLAGS) -o testredis $(TESTDIR)/testredis.c kendynet.a deps/hiredis/libhiredis.a  $(INCLUDE) $(LDFLAGS) $(DEFINE)
packet:kendynet.a $(TESTDIR)/testpacket.c
	$(CC) $(CFLAGS) -o packet $(TESTDIR)/testpacket.c kendynet.a $(INCLUDE) $(LDFLAGS) $(DEFINE)	
gateservice:kendynet.a $(TESTDIR)/gateservice.c $(TESTDIR)/testcommon.h
	$(CC) $(CFLAGS) -o gateservice $(TESTDIR)/gateservice.c kendynet.a deps/hiredis/libhiredis.a $(INCLUDE) $(LDFLAGS) $(DEFINE)	
asynserver:kendynet.a $(TESTDIR)/asynserver.c $(TESTDIR)/testcommon.h
	$(CC) $(CFLAGS) -o asynserver $(TESTDIR)/asynserver.c kendynet.a deps/hiredis/libhiredis.a $(INCLUDE) $(LDFLAGS) $(DEFINE)	
tcpserver:kendynet.a $(TESTDIR)/benchserver.c $(TESTDIR)/testcommon.h
	$(CC) $(CFLAGS) -o tcpserver $(TESTDIR)/benchserver.c kendynet.a $(INCLUDE) $(LDFLAGS) $(DEFINE)
server:kendynet.a $(TESTDIR)/server.c
	$(CC) $(CFLAGS) -o server $(TESTDIR)/server.c kendynet.a $(INCLUDE) $(LDFLAGS) $(DEFINE)	
tcpclient:kendynet.a $(TESTDIR)/benchclient.c $(TESTDIR)/testcommon.h
	$(CC) $(CFLAGS) -o tcpclient $(TESTDIR)/benchclient.c kendynet.a $(INCLUDE) $(LDFLAGS) $(DEFINE)
timer:kendynet.a $(TESTDIR)/testtimer.c
	$(CC) $(CFLAGS) -o timer $(TESTDIR)/testtimer.c kendynet.a $(INCLUDE) $(LDFLAGS) $(DEFINE)				
msgque:kendynet.a $(TESTDIR)/testmq.c
	$(CC) $(CFLAGS) -o msgque $(TESTDIR)/testmq.c kendynet.a $(INCLUDE) $(LDFLAGS) $(DEFINE)
systick:kendynet.a $(TESTDIR)/testgetsystick.c
	$(CC) $(CFLAGS) -o systick $(TESTDIR)/testgetsystick.c kendynet.a deps/hiredis/libhiredis.a $(INCLUDE) $(LDFLAGS)	$(DEFINE)
atomicst:kendynet.a $(TESTDIR)/test_atomic_st.c
	$(CC) $(CFLAGS) -o atomicst $(TESTDIR)/test_atomic_st.c kendynet.a $(INCLUDE) $(LDFLAGS)	$(DEFINE)
testexcp:kendynet.a $(TESTDIR)/testexception.c
	$(CC) $(CFLAGS) -o testexcp $(TESTDIR)/testexception.c kendynet.a $(INCLUDE) $(LDFLAGS)	$(DEFINE) -rdynamic -ldl
testlua:kendynet.a 	$(TESTDIR)/test.c
	$(CC) $(CFLAGS) -o testlua $(TESTDIR)/test.c kendynet.a deps/luajit-2.0/src/libluajit.a $(INCLUDE) $(LDFLAGS)	$(DEFINE) -rdynamic -ldl -lm
testasyncall:kendynet.a $(TESTDIR)/testasyncall.c $(TESTDIR)/testcommon.h
	$(CC) $(CFLAGS) -o testasyncall $(TESTDIR)/testasyncall.c kendynet.a deps/hiredis/libhiredis.a  $(INCLUDE) $(LDFLAGS) $(DEFINE)
testlog:kendynet.a $(TESTDIR)/testlog.c $(TESTDIR)/testcommon.h
	$(CC) $(CFLAGS) -o testlog $(TESTDIR)/testlog.c kendynet.a deps/hiredis/libhiredis.a  $(INCLUDE) $(LDFLAGS) $(DEFINE)	

	
	
