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
include src/server/CMakeFiles/server.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/server/CMakeFiles/server.dir/compiler_depend.make

# Include the progress variables for this target.
include src/server/CMakeFiles/server.dir/progress.make

# Include the compile flags for this target's objects.
include src/server/CMakeFiles/server.dir/flags.make

src/server/CMakeFiles/server.dir/main.c.o: src/server/CMakeFiles/server.dir/flags.make
src/server/CMakeFiles/server.dir/main.c.o: ../src/server/main.c
src/server/CMakeFiles/server.dir/main.c.o: src/server/CMakeFiles/server.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dong/ALLCODE/develop/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object src/server/CMakeFiles/server.dir/main.c.o"
	cd /home/dong/ALLCODE/develop/build/src/server && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT src/server/CMakeFiles/server.dir/main.c.o -MF CMakeFiles/server.dir/main.c.o.d -o CMakeFiles/server.dir/main.c.o -c /home/dong/ALLCODE/develop/src/server/main.c

src/server/CMakeFiles/server.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/server.dir/main.c.i"
	cd /home/dong/ALLCODE/develop/build/src/server && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/dong/ALLCODE/develop/src/server/main.c > CMakeFiles/server.dir/main.c.i

src/server/CMakeFiles/server.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/server.dir/main.c.s"
	cd /home/dong/ALLCODE/develop/build/src/server && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/dong/ALLCODE/develop/src/server/main.c -o CMakeFiles/server.dir/main.c.s

# Object files for target server
server_OBJECTS = \
"CMakeFiles/server.dir/main.c.o"

# External object files for target server
server_EXTERNAL_OBJECTS =

../bin/server: src/server/CMakeFiles/server.dir/main.c.o
../bin/server: src/server/CMakeFiles/server.dir/build.make
../bin/server: ../library/liblog.so
../bin/server: src/server/CMakeFiles/server.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/dong/ALLCODE/develop/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable ../../../bin/server"
	cd /home/dong/ALLCODE/develop/build/src/server && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/server.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/server/CMakeFiles/server.dir/build: ../bin/server
.PHONY : src/server/CMakeFiles/server.dir/build

src/server/CMakeFiles/server.dir/clean:
	cd /home/dong/ALLCODE/develop/build/src/server && $(CMAKE_COMMAND) -P CMakeFiles/server.dir/cmake_clean.cmake
.PHONY : src/server/CMakeFiles/server.dir/clean

src/server/CMakeFiles/server.dir/depend:
	cd /home/dong/ALLCODE/develop/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/dong/ALLCODE/develop /home/dong/ALLCODE/develop/src/server /home/dong/ALLCODE/develop/build /home/dong/ALLCODE/develop/build/src/server /home/dong/ALLCODE/develop/build/src/server/CMakeFiles/server.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/server/CMakeFiles/server.dir/depend

