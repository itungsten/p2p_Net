project(p2pNet)
set(CMAKE_CXX_STANDARD 14)
cmake_minimum_required(VERSION 3.5)
include_directories(/usr/local/boost/include)
link_directories(/usr/local/boost/lib)

ADD_SUBDIRECTORY(p2pClient)
ADD_SUBDIRECTORY(seed_server)
ADD_SUBDIRECTORY(test_net)