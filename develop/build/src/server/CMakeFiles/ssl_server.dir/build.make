# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/dong/ALLCODE/develop

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/dong/ALLCODE/develop/build

# Include any dependencies generated for this target.
include src/server/CMakeFiles/ssl_server.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/server/CMakeFiles/ssl_server.dir/compiler_depend.make

# Include the progress variables for this target.
include src/server/CMakeFiles/ssl_server.dir/progress.make

# Include the compile flags for this target's objects.
include src/server/CMakeFiles/ssl_server.dir/flags.make

src/server/CMakeFiles/ssl_server.dir/ssl_server.c.o: src/server/CMakeFiles/ssl_server.dir/flags.make
src/server/CMakeFiles/ssl_server.dir/ssl_server.c.o: ../src/server/ssl_server.c
src/server/CMakeFiles/ssl_server.dir/ssl_server.c.o: src/server/CMakeFiles/ssl_server.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dong/ALLCODE/develop/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object src/server/CMakeFiles/ssl_server.dir/ssl_server.c.o"
	cd /home/dong/ALLCODE/develop/build/src/server && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT src/server/CMakeFiles/ssl_server.dir/ssl_server.c.o -MF CMakeFiles/ssl_server.dir/ssl_server.c.o.d -o CMakeFiles/ssl_server.dir/ssl_server.c.o -c /home/dong/ALLCODE/develop/src/server/ssl_server.c

src/server/CMakeFiles/ssl_server.dir/ssl_server.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/ssl_server.dir/ssl_server.c.i"
	cd /home/dong/ALLCODE/develop/build/src/server && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/dong/ALLCODE/develop/src/server/ssl_server.c > CMakeFiles/ssl_server.dir/ssl_server.c.i

src/server/CMakeFiles/ssl_server.dir/ssl_server.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/ssl_server.dir/ssl_server.c.s"
	cd /home/dong/ALLCODE/develop/build/src/server && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/dong/ALLCODE/develop/src/server/ssl_server.c -o CMakeFiles/ssl_server.dir/ssl_server.c.s

# Object files for target ssl_server
ssl_server_OBJECTS = \
"CMakeFiles/ssl_server.dir/ssl_server.c.o"

# External object files for target ssl_server
ssl_server_EXTERNAL_OBJECTS =

../bin/ssl_server: src/server/CMakeFiles/ssl_server.dir/ssl_server.c.o
../bin/ssl_server: src/server/CMakeFiles/ssl_server.dir/build.make
../bin/ssl_server: ../library/liblog.so
../bin/ssl_server: ../library/libssl.so
../bin/ssl_server: ../library/libnetwork.so
../bin/ssl_server: src/server/CMakeFiles/ssl_server.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/dong/ALLCODE/develop/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable ../../../bin/ssl_server"
	cd /home/dong/ALLCODE/develop/build/src/server && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ssl_server.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/server/CMakeFiles/ssl_server.dir/build: ../bin/ssl_server
.PHONY : src/server/CMakeFiles/ssl_server.dir/build

src/server/CMakeFiles/ssl_server.dir/clean:
	cd /home/dong/ALLCODE/develop/build/src/server && $(CMAKE_COMMAND) -P CMakeFiles/ssl_server.dir/cmake_clean.cmake
.PHONY : src/server/CMakeFiles/ssl_server.dir/clean

src/server/CMakeFiles/ssl_server.dir/depend:
	cd /home/dong/ALLCODE/develop/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/dong/ALLCODE/develop /home/dong/ALLCODE/develop/src/server /home/dong/ALLCODE/develop/build /home/dong/ALLCODE/develop/build/src/server /home/dong/ALLCODE/develop/build/src/server/CMakeFiles/ssl_server.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/server/CMakeFiles/ssl_server.dir/depend

