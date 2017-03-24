cmake_minimum_required (VERSION 3.6.2)
project (sati_kuria_proto)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

set(CMAKE_PREFIX_PATH /home/pi/github/kuria/kuria-rpi-proto)
add_definitions(-lwiringPi -lm -lpthread)
# Headers
include_directories( "include")

# The file (GLOB...) allows for wildcard additions:
file(GLOB_RECURSE SOURCES "src/*.c")

add_executable(kuria_proto_test ${SOURCES})
target_compile_options(kuria_proto_test PRIVATE -Wall -std=c11 -mcpu=cortex-a53)

# set(PROJECT_LINK_LIBS libfreertos.a libfreertoslinux.a)
# link_directories("/home/pi/github/kuria/kuria-rpi-proto/bib")
find_library(FREERTOS_1 freertos)
find_library(FREERTOS_2 freertoslinux)
target_link_libraries( kuria_proto_test ${FREERTOS_1} ${FREERTOS_2} "-lwiringPi -lm -lpthread")

