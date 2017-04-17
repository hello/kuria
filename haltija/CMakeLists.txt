cmake_minimum_required (VERSION 3.1)

PROJECT (Haltija)
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_C_STANDARD 99)

set(CMAKE_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

option(BUILD_MATLAB_RUNNER "build the matlab runner" true)
option(BUILD_PUBLISHER "build the novelda publisher" true)

if (WIN32)
  set(BUILD_MATLAB_RUNNER true)
  set(ZLIB_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/winlib/zlib/zlibstatic.lib")
  set(ZLIB_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/winlib/zlib/include")
  set(HDF5_LIBRARIES  "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/winlib/hdf5/hdf5.lib")
  set(HDF5_INCLUDE_DIRS  "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/winlib/hdf5/include/")
else()
  link_directories("/usr/local/lib")

  if (BUILD_MATLAB_RUNNER)
    find_package(ZLIB REQUIRED)
    find_package(HDF5 REQUIRED)
  endif()

  find_package(ZMQ REQUIRED)
  if (BUILD_PUBLISHER)
    find_package(ModuleConnector REQUIRED)
#link_directories("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/novelda/lib/osx")
  endif()
endif()


if (BUILD_MATLAB_RUNNER)
  add_subdirectory(thirdparty/matio)
  message(STATUS "ZLIB LIBRARIES:  ${ZLIB_LIBRARIES}")
  message(STATUS "HDF5 LIBRARIES:  ${HDF5_LIBRARIES}")
endif()

add_subdirectory(thirdparty/nanopb)
add_subdirectory(thirdparty/yaml-cpp)

include_directories (
"include" 
"thirdparty/" 
"thirdparty/nanopb/" 
"thirdparty/gtest/include/" 
"thirdparty/gtest" 
"./" 
"./algorithm" 
"thirdparty/matio/src/" 
"mateigen/" 
${ZLIB_INCLUDE_DIRS} 
${HDF5_INCLUDE_DIRS} 
${ZMQ_INCLUDE_DIR}
${MODULECONNECTOR_INCLUDE_DIRS}
"thirdparty/yaml-cpp/include/"
)

if (BUILD_MATLAB_RUNNER)
add_library(mateigen STATIC
mateigen/matlabreader.cpp
mateigen/matlabwriter.cpp
)

target_link_libraries(mateigen matio)
endif()

add_library(algorithm STATIC
algorithm/kiss_fft.c
algorithm/haltijamath.cpp
algorithm/preprocessor.cpp
algorithm/preprocessorIIR.cpp
algorithm/respiration.cpp
algorithm/novelda_radar_subscriber.cpp
algorithm/rangebincombiner.cpp
algorithm/peakFinding.cpp
algorithm/ekf.cpp
algorithm/debug_publisher.cpp
algorithm/respiration_classifier.cpp
algorithm/hmm/HmmHelpers.cpp
algorithm/hmm/MatrixHelpers.cpp
algorithm/hmm/AllModels.cpp
)

add_library(network STATIC
network/noveldaprotobuf.cpp
network/radarmessageprotobuf.cpp
network/novelda.pb.c
network/simple_matrix.pb.c
network/radar_messages.pb.c
network/protobuf_matrix_utils.c
network/proto_utils.c
)

#############
add_library(gtest STATIC 
thirdparty/gtest/gtest-all.cc)

add_definitions(-DUNIT_TEST_DATA="${CMAKE_CURRENT_SOURCE_DIR}/unit-test/test-data/")

############
set(UNIT_TEST_SRC
unit-test/main.cpp 
unit-test/testfilter.cpp
unit-test/testmatrixmath.cpp
unit-test/testbuffers.cpp
unit-test/testnetwork.cpp
)


set(UNIT_TEST_LIBS gtest algorithm network protobuf-nanopb)

if (UNIX)
  set(UNIT_TEST_LIBS ${UNIT_TEST_LIBS} pthread)
endif()

if (BUILD_MATLAB_RUNNER)
  set(UNIT_TEST_SRC ${UNIT_TEST_SRC} 
                    unit-test/testmatio.cpp
                    unit-test/testpreprocessor.cpp)

  set(UNIT_TEST_LIBS ${UNIT_TEST_LIBS} ${HDF5_LIBRARIES} ${ZLIB_LIBRARIES} matio mateigen)
endif()

add_executable(unit-test 
${UNIT_TEST_SRC}
)

target_link_libraries(unit-test ${UNIT_TEST_LIBS})


###########
if (BUILD_MATLAB_RUNNER)
add_executable(haltijarunner
haltijarunner.cpp)

target_link_libraries(haltijarunner algorithm ${HDF5_LIBRARIES} ${ZLIB_LIBRARIES} matio mateigen)
endif()

############
if (NOT WIN32)
  add_executable(haltijanetworkrunner
  haltijanetworkrunner.cpp)

  target_link_libraries(haltijanetworkrunner algorithm network protobuf-nanopb zmq)
endif()
############
if (BUILD_PUBLISHER)
  add_executable(noveldapublisher
                 noveldapublisher.cpp)

  target_link_libraries(noveldapublisher ${MODULECONNECTOR_LIBRARIES} network protobuf-nanopb zmq yaml-cpp)

endif()


#copy data for tests
#this does not seem to work on windows
#add_custom_command(
#TARGET unit-test POST_BUILD
#COMMAND ${CMAKE_COMMAND} -E copy_directory
#${CMAKE_SOURCE_DIR}/unit-test/test_data
#$<TARGET_FILE_DIR:unit-test>
#)
