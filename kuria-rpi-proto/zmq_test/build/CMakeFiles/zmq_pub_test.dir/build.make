# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.6

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/pi/div_dev/zmq_test

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/pi/div_dev/zmq_test/build

# Include any dependencies generated for this target.
include CMakeFiles/zmq_pub_test.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/zmq_pub_test.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/zmq_pub_test.dir/flags.make

CMakeFiles/zmq_pub_test.dir/test_pub.c.o: CMakeFiles/zmq_pub_test.dir/flags.make
CMakeFiles/zmq_pub_test.dir/test_pub.c.o: ../test_pub.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pi/div_dev/zmq_test/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/zmq_pub_test.dir/test_pub.c.o"
	/usr/bin/clang  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/zmq_pub_test.dir/test_pub.c.o   -c /home/pi/div_dev/zmq_test/test_pub.c

CMakeFiles/zmq_pub_test.dir/test_pub.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/zmq_pub_test.dir/test_pub.c.i"
	/usr/bin/clang  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/pi/div_dev/zmq_test/test_pub.c > CMakeFiles/zmq_pub_test.dir/test_pub.c.i

CMakeFiles/zmq_pub_test.dir/test_pub.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/zmq_pub_test.dir/test_pub.c.s"
	/usr/bin/clang  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/pi/div_dev/zmq_test/test_pub.c -o CMakeFiles/zmq_pub_test.dir/test_pub.c.s

CMakeFiles/zmq_pub_test.dir/test_pub.c.o.requires:

.PHONY : CMakeFiles/zmq_pub_test.dir/test_pub.c.o.requires

CMakeFiles/zmq_pub_test.dir/test_pub.c.o.provides: CMakeFiles/zmq_pub_test.dir/test_pub.c.o.requires
	$(MAKE) -f CMakeFiles/zmq_pub_test.dir/build.make CMakeFiles/zmq_pub_test.dir/test_pub.c.o.provides.build
.PHONY : CMakeFiles/zmq_pub_test.dir/test_pub.c.o.provides

CMakeFiles/zmq_pub_test.dir/test_pub.c.o.provides.build: CMakeFiles/zmq_pub_test.dir/test_pub.c.o


# Object files for target zmq_pub_test
zmq_pub_test_OBJECTS = \
"CMakeFiles/zmq_pub_test.dir/test_pub.c.o"

# External object files for target zmq_pub_test
zmq_pub_test_EXTERNAL_OBJECTS =

zmq_pub_test: CMakeFiles/zmq_pub_test.dir/test_pub.c.o
zmq_pub_test: CMakeFiles/zmq_pub_test.dir/build.make
zmq_pub_test: CMakeFiles/zmq_pub_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/pi/div_dev/zmq_test/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable zmq_pub_test"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/zmq_pub_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/zmq_pub_test.dir/build: zmq_pub_test

.PHONY : CMakeFiles/zmq_pub_test.dir/build

CMakeFiles/zmq_pub_test.dir/requires: CMakeFiles/zmq_pub_test.dir/test_pub.c.o.requires

.PHONY : CMakeFiles/zmq_pub_test.dir/requires

CMakeFiles/zmq_pub_test.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/zmq_pub_test.dir/cmake_clean.cmake
.PHONY : CMakeFiles/zmq_pub_test.dir/clean

CMakeFiles/zmq_pub_test.dir/depend:
	cd /home/pi/div_dev/zmq_test/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pi/div_dev/zmq_test /home/pi/div_dev/zmq_test /home/pi/div_dev/zmq_test/build /home/pi/div_dev/zmq_test/build /home/pi/div_dev/zmq_test/build/CMakeFiles/zmq_pub_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/zmq_pub_test.dir/depend

