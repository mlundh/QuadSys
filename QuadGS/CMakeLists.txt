# cmake needs this line
cmake_minimum_required(VERSION 3.1)
include(CMakePrintHelpers)
# Define project name
project(QuadGS VERSION 0.1)
# Set project properties
set(CMAKE_CXX_STANDARD 11) # C++11...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(SEND_ERROR "In-source builds are not allowed.")
endif ()

# Set some warning flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic -Werror")

# Fix boost linking for log.
add_definitions(-DBOOST_LOG_DYN_LINK)

# Include all our helper functions and macros.
include(QuadGSUtils.cmake)

# Print header
message(STATUS "QuadGS - Ground station for quad copter system based on QuadFC.")

# Find boost
find_package(Boost 1.58.0 REQUIRED COMPONENTS 
                    thread
                    program_options
                    system
                    locale
                    log
                    filesystem
                    log_setup
                    regex
                    date_time)
include_directories(${Boost_INCLUDE_DIRS})

# All libs will be built as shared due to this.
SET(BUILD_SHARED_LIBS ON)

# Use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# When building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

# Add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


# The RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "../lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "../lib")
ENDIF("${isSystemDir}" STREQUAL "-1")

# Add includes to the three main components.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/IO)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/UI)


# Targets that we develop
enable_testing()
add_subdirectory(Core)
add_subdirectory(IO)
add_subdirectory(Log)
add_subdirectory(test)
add_subdirectory(UI)
add_subdirectory(top)


