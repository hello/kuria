cmake_minimum_required (VERSION 3.1)

PROJECT (ZmqTest)
set (CMAKE_CXX_STANDARD 14)
#set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12")

set(CMAKE_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../cmake")

find_package(ZMQ REQUIRED)
message(STATUS "${ZMQ_INCLUDE_DIR}")
include_directories(${ZMQ_INCLUDE_DIR})

#############
add_executable(zmqclient
zmqclient.cpp
)
target_link_libraries(zmqclient ${ZMQ_LIBRARY} )

##############
add_executable(zmqserver
zmqserver.cpp
)
target_link_libraries(zmqserver ${ZMQ_LIBRARY} )
