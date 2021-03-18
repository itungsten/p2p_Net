
project(p2pseed)

set(CMAKE_CXX_STANDARD 14)

add_executable(p2pseed main.cpp Server.cpp Server.h utils.cpp utils.h Socketor.cpp Socketor.h)

target_link_libraries(p2pseed boost_system pthread)