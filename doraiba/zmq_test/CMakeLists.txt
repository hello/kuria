cmake_minimum_required (VERSION 3.1)

project (zmq_test)

set (CMAKE_C_COMPILER   "/usr/bin/clang")
#set (CMAKE_C_COMPILER	"/usr/bin/arm-linux-gnueabihf-gcc")
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

set(CMAKE_PREFIX_PATH /home/pi/div_dev/zmq_test)

# Headers
find_path (ZMQ_INCLUDE_DIR NAMES zmq.h)

include_directories (${ZMQ_INCLUDE_DIR})

# PUBLISHER
add_executable(zmq_pub_test test_pub.c)

target_compile_options(zmq_pub_test PRIVATE -Wall -g -std=gnu++11 -std=c11 -mcpu=cortex-a53)

find_library(ZMQ_LIB libzmq.so "/usr/local/lib/") 
target_link_libraries( zmq_pub_test -lzmq)

# SUBSCRIBER
add_executable(zmq_sub_test test_sub.c)

target_compile_options(zmq_sub_test PRIVATE -Wall -g -std=gnu++11 -std=c11 -mcpu=cortex-a53)

target_link_libraries( zmq_sub_test -lzmq)

