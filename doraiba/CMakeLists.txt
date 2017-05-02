cmake_minimum_required (VERSION 3.5.1)

project (sati_kuria_proto)

set (CMAKE_C_COMPILER   "/usr/bin/gcc")
#set (CMAKE_C_COMPILER	"/usr/bin/arm-linux-gnueabihf-gcc")
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

set(CMAKE_PREFIX_PATH /home/pi/github/kuria/doraiba)

# Headers
include_directories( "include")

# TODO not sure if passing define _GNU_SOURCE is a good idea. But it seems to fix issues where
# pthread recursive mutex attr functions throw an error

# RADAR PUBLISHER
set (PUB_PB_SOURCES 
    src/pb_encode.c
    src/pb_common.c
    src/novelda.pb.c
    src/novelda_protobuf.c
    )
set (PUB_HAL_SOURCES
    src/spidriver.c
    src/x4driver.c
    src/gpio_hal.c
    )
set (PUB_UTILS_SOURCES
    src/hlo_notify.c
    src/kuria_utils.c
    )
set (PUB_SOURCES 
    src/radar_publisher.c
    src/radar_task.c
    src/dispatch_radar_frame.c
    )
set (PUB_ALL_SRCS
    ${PUB_PB_SOURCES}
    ${PUB_HAL_SOURCES}
    ${PUB_UTILS_SOURCES}
    ${PUB_SOURCES}
    )
add_executable(radar_publisher ${PUB_ALL_SRCS})

target_compile_options(radar_publisher PRIVATE -Wall -g -std=c11 -mcpu=native -D_GNU_SOURCE -pg )

find_library( WIRINGPI wiringPi "lib") 
target_link_libraries( radar_publisher ${WIRINGPI} -lzmq -lm -lpthread -pg)

# RADAR SUBSCRIBER
set (SUB_SOURCES src/radar_subscriber.c 
    src/pb_decode.c
    src/pb_common.c
    src/novelda.pb.c
    src/novelda_protobuf.c
    src/base64.c
    )
add_executable(radar_subscriber ${SUB_SOURCES})

target_compile_options(radar_subscriber PRIVATE -Wall -g -std=c11 -mcpu=native -D_GNU_SOURCE -DRADAR_DATA_TESTING -DWRITE_PB_TO_FILE)

target_link_libraries( radar_subscriber -lzmq -lpthread)
