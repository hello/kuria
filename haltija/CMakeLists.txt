cmake_minimum_required (VERSION 2.8) 

PROJECT (Haltija)

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.10")

set(CMAKE_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

if (${CMAKE_SYSTEM_NAME} EQUAL "WINDOWS")
set(ZLIB_LIBRARIES,${CMAKE_CURRENT_SOURCE_DIR}/zlib/zlib.lib)
set(ZLIB_INCLUDE_DIRS,"./zlib/include")
else()
find_package(zlib REQUIRED)
endif()

include_directories ("gtest/include/" "gtest" "./" "./src" ${ZLIB_INCLUDE_DIRS})

add_library(matio STATIC
matio/endian.c
matio/inflate.c
matio/io.c
matio/mat.c
matio/mat4.c
matio/mat5.c
matio/mat73.c
matio/matvar_cell.c
matio/matvar_struct.c
matio/read_data.c
matio/snprintf.c)

target_link_libraries(matio ${ZLIB_LIBRARIES})

add_library(haltija STATIC
src/example.cpp
)

#############
add_library(gtest STATIC gtest/gtest-all.cc)
#target_link_libraries(gtest pthread)

############
add_executable(unit-test 
unit-test/main.cpp 
unit-test/testradar1.cpp
)

target_link_libraries(unit-test gtest haltija)


###########
add_executable(haltijarunner
haltijarunner.cpp)

target_link_libraries(haltijarunner haltija matio)
