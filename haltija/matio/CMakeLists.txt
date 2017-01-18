cmake_minimum_required (VERSION 2.8)
project (matio)

if (WIN32)
  set(ZLIB_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/../winlib/zlib/zlibstatic.lib")
  set(ZLIB_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../winlib/zlib/include")
  set(HDF5_LIBRARIES  "${CMAKE_CURRENT_SOURCE_DIR}/../winlib/hdf5/hdf5.lib")
  set(HDF5_INCLUDE_DIRS  "${CMAKE_CURRENT_SOURCE_DIR}/../winlib/hdf5/include/")
else()
  find_package(zlib)
  find_package(HDF5 REQUIRED)
endif()


set(CMAKE_INCLUDE_DIRECTORIES_BEFORE TRUE)
include_directories(${HDF5_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})

add_definitions(-DMAT73)
set(SOURCES
./getopt/getopt_long.c
./src/endian.c
./src/inflate.c
./src/io.c
./src/mat.c
./src/mat4.c
./src/mat5.c
./src/mat73.c
./src/matvar_cell.c
./src/matvar_struct.c
./src/read_data.c
./src/snprintf.c
)

#message(STATUS "Didn't find HDF5 - Skipping HDF5 support")
#set(SOURCES
#./getopt/getopt_long.c
#./src/endian.c
#./src/inflate.c
#./src/io.c
#./src/mat.c
#./src/mat4.c
#./src/mat5.c
#./src/matvar_cell.c
#./src/matvar_struct.c
#./src/read_data.c
#./src/snprintf.c
#)


include_directories (
./getopt
./src
)


set(EXTRA
./test/test_mat.c
./test/test_snprintf.c
./tools/matdump.c
)


add_library(matio STATIC ${SOURCES})
#install (FILES src/matio.h DESTINATION "/usr/local/include")
#install(TARGETS matio DESTINATION "/usr/local/lib")

