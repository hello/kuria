cmake_minimum_required (VERSION 2.8) 

PROJECT (Haltija)

#set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12")

set(CMAKE_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)


if (WIN32)
  set(ZLIB_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/winlib/zlib/zlibstatic.lib")
  set(ZLIB_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/winlib/zlib/include")
  set(HDF5_LIBRARIES  "${CMAKE_CURRENT_SOURCE_DIR}/winlib/hdf5/hdf5.lib")
  set(HDF5_INCLUDE_DIRS  "${CMAKE_CURRENT_SOURCE_DIR}/winlib/hdf5/include/")
else()
  find_package(zlib REQUIRED)
  find_package(HDF5 REQUIRED)
endif()


message(STATUS "ZLIB LIBRARIES:  ${ZLIB_LIBRARIES}")
message(STATUS "HDF5 LIBRARIES:  ${HDF5_LIBRARIES}")

include_directories ("gtest/include/" "gtest" "./" "./src" "matio/src/" ${ZLIB_INCLUDE_DIRS} ${HDF5_INCLUDE_DIRS})

add_subdirectory(matio)

add_library(haltija STATIC
src/example.cpp
)

#############
add_library(gtest STATIC gtest/gtest-all.cc)

add_definitions(-DUNIT_TEST_DATA="${CMAKE_CURRENT_SOURCE_DIR}/unit-test/test-data/")

############
add_executable(unit-test 
unit-test/main.cpp 
unit-test/testradar1.cpp
unit-test/testmatio.cpp
)

target_link_libraries(unit-test gtest haltija ${HDF5_LIBRARIES} ${ZLIB_LIBRARIES} matio)


###########
add_executable(haltijarunner
haltijarunner.cpp)

target_link_libraries(haltijarunner haltija)


 
#copy data for tests
#this does not seem to work on windows
#add_custom_command(
#TARGET unit-test POST_BUILD
#COMMAND ${CMAKE_COMMAND} -E copy_directory
#${CMAKE_SOURCE_DIR}/unit-test/test_data
#$<TARGET_FILE_DIR:unit-test>
#)
