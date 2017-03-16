cmake_minimum_required (VERSION 3.1)

PROJECT (Haltija)
set (CMAKE_CXX_STANDARD 14)
#set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12")

set(CMAKE_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)



if (WIN32)
  set(ZLIB_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/winlib/zlib/zlibstatic.lib")
  set(ZLIB_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/winlib/zlib/include")
  set(HDF5_LIBRARIES  "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/winlib/hdf5/hdf5.lib")
  set(HDF5_INCLUDE_DIRS  "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/winlib/hdf5/include/")
else()
  find_package(ZLIB REQUIRED)
  find_package(HDF5 REQUIRED)
  find_package(ZMQ REQUIRED)
endif()

add_subdirectory(thirdparty/matio)
add_subdirectory(thirdparty/nanopb)
add_subdirectory(thirdparty/yaml-cpp)

message(STATUS "ZLIB LIBRARIES:  ${ZLIB_LIBRARIES}")
message(STATUS "HDF5 LIBRARIES:  ${HDF5_LIBRARIES}")

include_directories ("thirdparty/" "thirdparty/gtest/include/" "thirdparty/gtest" "./" "./algorithm" "thirdparty/matio/src/" "mateigen/" ${ZLIB_INCLUDE_DIRS} ${HDF5_INCLUDE_DIRS} ${ZMQ_INCLUDE_DIR})

add_library(mateigen STATIC
mateigen/matlabreader.cpp
mateigen/matlabwriter.cpp
)

add_library(algorithm STATIC
algorithm/kiss_fft.c
algorithm/haltijamath.cpp
algorithm/preprocessor.cpp
algorithm/respiration.cpp
)

#############
add_library(gtest STATIC 
thirdparty/gtest/gtest-all.cc)

add_definitions(-DUNIT_TEST_DATA="${CMAKE_CURRENT_SOURCE_DIR}/unit-test/test-data/")

############
add_executable(unit-test 
unit-test/main.cpp 
unit-test/testmatio.cpp
unit-test/testfilter.cpp
unit-test/testmatrixmath.cpp
unit-test/testbuffers.cpp
unit-test/testpreprocessor.cpp
)


target_link_libraries(unit-test gtest algorithm ${HDF5_LIBRARIES} ${ZLIB_LIBRARIES} matio mateigen)


###########
add_executable(haltijarunner
haltijarunner.cpp)

target_link_libraries(haltijarunner algorithm ${HDF5_LIBRARIES} ${ZLIB_LIBRARIES} matio mateigen)

############
add_executable(haltijanetworkrunner
haltijanetworkrunner.cpp)

target_link_libraries(haltijanetworkrunner algorithm )


#copy data for tests
#this does not seem to work on windows
#add_custom_command(
#TARGET unit-test POST_BUILD
#COMMAND ${CMAKE_COMMAND} -E copy_directory
#${CMAKE_SOURCE_DIR}/unit-test/test_data
#$<TARGET_FILE_DIR:unit-test>
#)
