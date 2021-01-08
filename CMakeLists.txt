PROJECT(project)
cmake_minimum_required(VERSION 2.6)
SET(CMAKE_BUILD_TYPE "Debug")
add_definitions(-std=c++11)
add_definitions(-DNDEBUG)
include_directories(${CMAKE_SOURCE_DIR}/)
include_directories(${CMAKE_SOURCE_DIR}/logging)
include_directories(${CMAKE_SOURCE_DIR}/base)
include_directories(${CMAKE_SOURCE_DIR}/tcp)
include_directories(${CMAKE_SOURCE_DIR}/dash)
include_directories(${CMAKE_SOURCE_DIR}/tpproxy)
set(base_LIB
base/base_alarm.cc
base/base_clock.cc
base/base_epoll_alarm_factory.cc  
base/base_epoll_clock.cc
base/base_thread.cc
base/base_time.cc
base/byte_codec.cc 
base/epoll_thread.cc
base/ip_address.cc
base/random.cc
base/simple_epoll_server.cc
base/socket_address.cc
tcp/tcp_client.cc
tcp/tcp_handle.cc
tcp/tcp_server.cc
)
set(dash_LIB
dash/dash_algorithm.cc
dash/dash_client.cc
dash/dash_server.cc
dash/dash_util.cc
dash/split.cc
)
add_library(base STATIC ${base_LIB})
add_library(dash STATIC ${dash_LIB})
set(EXECUTABLE_NAME "t_dash_server")
add_executable(${EXECUTABLE_NAME} ${CMAKE_SOURCE_DIR}/dash/dash_server_test.cc)
target_link_libraries(${EXECUTABLE_NAME} dash base  pthread)

set(EXECUTABLE_NAME "t_dash_client")
add_executable(${EXECUTABLE_NAME} ${CMAKE_SOURCE_DIR}/dash/dash_client_test.cc)
target_link_libraries(${EXECUTABLE_NAME} dash base  pthread)

set(EXECUTABLE_NAME "possion_sender")
add_executable(${EXECUTABLE_NAME} ${CMAKE_SOURCE_DIR}/tools/possion_sender.cc)
target_link_libraries(${EXECUTABLE_NAME})

set(tpproxy_src
tpproxy/tpproxy_server_test.cc
tpproxy/tpproxy_server.cc
)
set(EXECUTABLE_NAME "t_proxy")
add_executable(${EXECUTABLE_NAME} ${tpproxy_src})
target_link_libraries(${EXECUTABLE_NAME} base  pthread)

