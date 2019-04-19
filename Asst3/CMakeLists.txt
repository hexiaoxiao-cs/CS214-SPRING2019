project(WTF)
add_executable(WTFserver common/protocol.c common/util.c server/server.c server/network.c)
add_executable(WTF common/protocol.c common/util.c client/client.c client/network.c)