PROJECT(project)
cmake_minimum_required(VERSION 2.6)
SET(CMAKE_BUILD_TYPE "Debug")
add_definitions(-std=c++14)
add_definitions(-DNDEBUG)
include_directories(${CMAKE_SOURCE_DIR}/)
include_directories(${CMAKE_SOURCE_DIR}/logging)
include_directories(${CMAKE_SOURCE_DIR}/base)
include_directories(${CMAKE_SOURCE_DIR}/tcp)
include_directories(${CMAKE_SOURCE_DIR}/dash)
add_subdirectory(base)
add_subdirectory(dash)




set(EXECUTABLE_NAME "t_dash_server")
add_executable(${EXECUTABLE_NAME} ${CMAKE_SOURCE_DIR}/dash/dash_server_test.cc)
target_link_libraries(${EXECUTABLE_NAME} dash base  pthread)

set(EXECUTABLE_NAME "t_dash_client")
add_executable(${EXECUTABLE_NAME} ${CMAKE_SOURCE_DIR}/dash/dash_client_test.cc)
target_link_libraries(${EXECUTABLE_NAME} dash base  pthread)

set(EXECUTABLE_NAME "possion_sender")
add_executable(${EXECUTABLE_NAME} ${CMAKE_SOURCE_DIR}/tools/possion_sender.cc)
target_link_libraries(${EXECUTABLE_NAME})
