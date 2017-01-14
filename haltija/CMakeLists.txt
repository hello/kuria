cmake_minimum_required (VERSION 2.8) 

PROJECT (Haltija)

#set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12")

set(CMAKE_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

#set up zlib
if (WIN32)
set(ZLIB_LIBRARIES,${CMAKE_CURRENT_SOURCE_DIR}/zlib/zlib.lib)
set(ZLIB_INCLUDE_DIRS,"./zlib/include")
else()
  find_package(zlib REQUIRED)
  find_package(hdf5 REQUIRED)
endif()


include_directories ("gtest/include/" "gtest" "./" "./src" "matio/src/" ${ZLIB_INCLUDE_DIRS})

add_subdirectory(matio)

add_library(haltija STATIC
src/example.cpp
)

#############
add_library(gtest STATIC gtest/gtest-all.cc)

############
add_executable(unit-test 
unit-test/main.cpp 
unit-test/testradar1.cpp
unit-test/testmatio.cpp
)

target_link_libraries(unit-test gtest haltija matio ${HDF5_LIBRARIES})


###########
add_executable(haltijarunner
haltijarunner.cpp)

target_link_libraries(haltijarunner haltija)



#copy data for tests
#this does not seem to work on windows
add_custom_command(
TARGET unit-test POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_SOURCE_DIR}/unit-test/test_data
$<TARGET_FILE_DIR:unit-test>
)
