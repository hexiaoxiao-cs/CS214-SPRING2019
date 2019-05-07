libtar_objs=libtar/lib/append.o libtar/lib/block.o libtar/lib/decode.o libtar/lib/encode.o libtar/lib/extract.o libtar/lib/handle.o libtar/lib/output.o libtar/lib/util.o libtar/lib/wrapper.o libtar/listhash/libtar_hash.o libtar/listhash/libtar_list.o libtar/compat/basename.o libtar/compat/dirname.o libtar/compat/strlcpy.o libtar/compat/strmode.o
wtf_server_objs=common/util.o common/netutil.o server/main.o server/network.o server/ds.o server/logic.o server/serverutil.o 
wtf_objs=common/util.o common/netutil.o client/network.o
wtf_test_objs=playground/main.o common/util.o common/netutil.o client/network.o

client_protocol_objs=common/protocol_client.o
server_protocol_objs=common/protocol_server.o
client_obj=client/client.o
client_test_obj=client/client_test.o

CFLAGS=-Icommon -Ilibtar/lib -Ilibtar/listhash -Ilibtar/compat -Ilibtar

$(libtar_objs): EXTRA_CFLAGS := -std=gnu99
$(wtf_objs): INCLUDE := -Iclient
$(wtf_server_objs): INCLUDE := -Iserver
$(wtf_test_objs): INCLUDE := -Iclient

all: WTFserver WTF test


common/protocol_server.o:
	$(CC) $(CFLAGS) -Iserver -DSERVER_COMPILING -o common/protocol_server.o -c common/protocol.c

common/protocol_client.o:
	$(CC) $(CFLAGS) -Iclient -DCLIENT_COMPILING -o common/protocol_client.o -c common/protocol.c

client/client.o:
	$(CC) $(CFLAGS) -Iclient -DCLIENT_COMPILING -o client/client.o -c client/client.c

client/client_test.o:
	$(CC) $(CFLAGS) -Iclient -DTEST_COMPILING -o client/client_test.o -c client/client.c

WTFserver: $(libtar_objs) $(wtf_server_objs) $(server_protocol_objs)
	$(CC) $(libtar_objs) $(wtf_server_objs) $(server_protocol_objs) -o WTFserver -lpthread

WTF: $(libtar_objs) $(wtf_objs) $(client_protocol_objs) $(client_obj)
	$(CC) $(libtar_objs) $(wtf_objs) $(client_protocol_objs) $(client_obj) -o WTF -lcrypto

test: $(libtar_objs) $(wtf_test_objs) $(client_test_obj) $(client_protocol_objs)
	$(CC) $(libtar_objs) $(wtf_test_objs) $(client_test_obj) $(client_protocol_objs) -o WTFtest -lcrypto -lpthread

clean: 
	find . -name "*.o" | xargs rm -rf
	rm -rf WTF WTFserver

%.o: %.c
	@echo [Compiling]: $<
	$(CC) $(CFLAGS) $(EXTRA_FLAGS) $(INCLUDE) -o $@ -c $<
