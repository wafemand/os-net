cmake_minimum_required(VERSION 3.10)
project(os-net)

set(CMAKE_CXX_STANDARD 14)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(server_app server/main.cpp server/TFTPServer.cpp util/UDPSocket.cpp)
add_executable(client_app client/main.cpp client/TFTPClient.cpp util/UDPSocket.cpp)
