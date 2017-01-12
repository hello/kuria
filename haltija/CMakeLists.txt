cmake_minimum_required (VERSION 2.8) 

PROJECT (Haltija)

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.10")

set(CMAKE_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)


find_package(eigen3 REQUIRED)



#add_definitions(-DUSE_CPP_DEBUG_LOGGING)

include_directories (${EIGEN3_INCLUDE_DIR} "gtest/include/" "gtest" "./" "./src")
message(STATUS "${EIGEN3_INCLUDE_DIR}")

add_library(haltija STATIC
src/example.cpp
)

#############
add_library(gtest STATIC gtest/gtest-all.cc)
target_link_libraries(gtest pthread)

############
add_executable(unit-test 
unit-test/main.cpp 
unit-test/testradar1.cpp
)

target_link_libraries(unit-test gtest haltija)


###########
add_executable(haltijarunner
haltijarunner.cpp)

target_link_libraries(haltijarunner haltija)

