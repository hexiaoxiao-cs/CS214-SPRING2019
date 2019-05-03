cmake_minimum_required(VERSION 2.8)
project(WTF)
include_directories(common)
add_executable(WTFserver common/protocol.c common/util.c common/netutil.c server/main.c server/network.c server/ds.c server/logic.c)
add_executable(WTF common/protocol.c common/util.c common/netutil.c client/client.c client/network.c common/netutil.c common/netutil.h)

add_executable(playground playground/main.c)
target_link_libraries(playground /usr/lib64/libtar.so.1.2.11 -lcrypto)

target_link_libraries(WTFserver PRIVATE -lpthread /usr/lib64/libtar.so.1.2.11)
target_link_libraries(WTF PRIVATE /usr/lib64/libtar.so.1.2.11 -lcrypto)

target_include_directories(WTFserver PRIVATE server)
target_include_directories(WTF PRIVATE client)

target_compile_definitions(WTFserver PRIVATE SERVER_COMPILING)
target_compile_definitions(WTF PRIVATE CLIENT_COMPILING)