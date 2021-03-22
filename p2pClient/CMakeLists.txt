
project(p2pclient)

set(CMAKE_CXX_STANDARD 14)

add_executable(p2pclient main.cpp peerClient.cpp peerClient.h ../config.h SM2Translator.cpp SM2Translator.h)

target_link_libraries(p2pclient boost_system pthread)