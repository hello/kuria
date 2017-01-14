cmake_minimum_required (VERSION 2.8)
project (matio)

set(LIBRARY_OUTPUT_PATH ..)
set(CMAKE_C_FLAGS "-Wall -Wextra -Werror -Wno-unused-function")

find_package(HDF5)
if (HDF5_FOUND)
message(STATUS "Found HDF5")
include_directories(${HDF5_INCLUDE_DIRS})
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

else()
message(STATUS "Didn't find HDF5 - Skipping HDF5 support")
set(SOURCES
./getopt/getopt_long.c
./src/endian.c
./src/inflate.c
./src/io.c
./src/mat.c
./src/mat4.c
./src/mat5.c
./src/matvar_cell.c
./src/matvar_struct.c
./src/read_data.c
./src/snprintf.c
)

endif (HDF5_FOUND)

include_directories (
./getopt
./src
)


set(EXTRA
./test/test_mat.c
./test/test_snprintf.c
./tools/matdump.c
)


add_library(matio ${SOURCES})
install (FILES src/matio.h DESTINATION "/usr/local/include")
install(TARGETS matio DESTINATION "/usr/local/lib")

