cmake_minimum_required(VERSION 2.8)
project(WTF)

include_directories(common)
include_directories(libtar/lib libtar/listhash libtar/compat libtar)

list(APPEND
        libtar_
        libtar/lib/append.c libtar/lib/block.c libtar/lib/decode.c libtar/lib/encode.c libtar/lib/extract.c libtar/lib/handle.c libtar/lib/output.c libtar/lib/util.c libtar/lib/wrapper.c
        libtar/listhash/libtar_hash.c libtar/listhash/libtar_list.c
        libtar/compat/basename.c libtar/compat/dirname.c libtar/compat/strlcpy.c libtar/compat/strmode.c)

foreach(c_file IN LISTS libtar_)
    set_source_files_properties(${c_file} PROPERTIES COMPILE_FLAGS -std=gnu99)
endforeach()

add_executable(WTFserver ${libtar_} common/protocol.c common/util.c common/netutil.c server/main.c server/network.c server/ds.c server/logic.c server/serverutil.c server/serverutil.h)
add_executable(WTF ${libtar_} common/protocol.c common/util.c common/netutil.c client/client.c client/network.c)

add_executable(WTFtest ${libtar_} playground/main.c common/protocol.c common/util.c common/netutil.c client/network.c)
target_link_libraries(WTFtest  -lcrypto)

target_link_libraries(WTFserver PRIVATE -lpthread)
target_link_libraries(WTF PRIVATE -lcrypto)

target_include_directories(WTFserver PRIVATE server)
target_include_directories(WTF PRIVATE client)
target_include_directories(WTFtest PRIVATE client)

target_compile_definitions(WTFserver PRIVATE SERVER_COMPILING)
target_compile_definitions(WTF PRIVATE CLIENT_COMPILING)