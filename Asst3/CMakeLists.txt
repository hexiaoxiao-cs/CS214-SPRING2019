cmake_minimum_required(VERSION 2.8)
project(WTF)
include_directories(common)
add_executable(WTFserver common/protocol.c common/util.c server/main.c server/network.c server/ds.c)
add_executable(WTF common/protocol.c common/util.c client/client.c client/network.c)

target_link_libraries(WTFserver -lpthread)
target_link_libraries(WTF)