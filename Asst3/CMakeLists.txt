cmake_minimum_required(VERSION 2.8)
project(WTF)

include_directories(common)
include_directories(libtar/lib libtar/listhash libtar/compat libtar)

AUX_SOURCE_DIRECTORY(libtar/lib/ libtar_lib)
AUX_SOURCE_DIRECTORY(libtar/listhash/ libtar_listhash)
AUX_SOURCE_DIRECTORY(libtar/compat/ libtar_compact)

foreach(c_file in ${libtar_lib} ${libtar_listhash} ${libtar_compact})
    set_source_files_properties(${c_file} PROPERTIES COMPILE_FLAGS -std=gnu99)
endforeach()

add_executable(WTFserver ${libtar_lib} ${libtar_listhash} ${libtar_compact} common/protocol.c common/util.c common/netutil.c server/main.c server/network.c server/ds.c server/logic.c)
add_executable(WTF ${libtar_lib} ${libtar_listhash} ${libtar_compact} common/protocol.c common/util.c common/netutil.c client/client.c client/network.c)

add_executable(playground playground/main.c)
target_link_libraries(playground /usr/lib64/libtar.so.1.2.11 -lcrypto)

target_link_libraries(WTFserver PRIVATE -lpthread)
target_link_libraries(WTF PRIVATE -lcrypto)

target_include_directories(WTFserver PRIVATE server)
target_include_directories(WTF PRIVATE client)

target_compile_definitions(WTFserver PRIVATE SERVER_COMPILING)
target_compile_definitions(WTF PRIVATE CLIENT_COMPILING)