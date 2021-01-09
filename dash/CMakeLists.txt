set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(${CMAKE_SOURCE_DIR}/)
set(dash_LIB
${CMAKE_SOURCE_DIR}/dash/dash_algorithm.cc
${CMAKE_SOURCE_DIR}/dash/dash_client.cc
${CMAKE_SOURCE_DIR}/dash/dash_server.cc
${CMAKE_SOURCE_DIR}/dash/dash_util.cc
${CMAKE_SOURCE_DIR}/dash/split.cc
)
add_library(dash STATIC ${dash_LIB})