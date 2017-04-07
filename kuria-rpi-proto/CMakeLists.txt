cmake_minimum_required (VERSION 3.5.1)

project (sati_kuria_proto)

if (UNIX AND NOT APPLE)
    message ("LINUX")
elseif (APPLE)
    message ("APPLE")
endif (UNIX AND NOT APPLE)

if (LINUX)
    message ("LINUX")
endif (LINUX)

# set (CMAKE_C_COMPILER   "/usr/bin/clang")
set (CMAKE_C_COMPILER	"/usr/bin/arm-linux-gnueabihf-gcc")
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

set(CMAKE_PREFIX_PATH /home/pi/github/kuria/kuria-rpi-proto)

# Headers
include_directories( "include")

# All the sources
# The file (GLOB...) allows for wildcard additions:
file(GLOB_RECURSE SOURCES "src/*.c")

add_executable(kuria_proto_test ${SOURCES})

# TODO not sure if passing define _GNU_SOURCE is a good idea. But it seems to fix issues where
# pthread recursive mutex attr functions throw an error
target_compile_options(kuria_proto_test PRIVATE -Wall -g -std=c11 -mcpu=cortex-a53 -D_GNU_SOURCE)

# TODO - Added commands to build freertos static library from freertos source files
find_library( WIRINGPI wiringPi "lib") 
target_link_libraries( kuria_proto_test ${WIRINGPI} "-lm -lpthread")

