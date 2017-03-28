cmake_minimum_required (VERSION 3.6.2)

project (sati_kuria_proto)

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
target_compile_options(kuria_proto_test PRIVATE -Wall -std=c11 -mcpu=cortex-a53 -D_GNU_SOURCE)

# TODO - Added commands to build freertos static library from freertos source files
find_library(FREERTOS_1 freertos)
find_library(FREERTOS_2 freertoslinux)
target_link_libraries( kuria_proto_test ${FREERTOS_1} ${FREERTOS_2} "-lwiringPi -lm -lpthread")

