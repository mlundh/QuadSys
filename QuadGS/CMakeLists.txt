# cmake needs this line
cmake_minimum_required(VERSION 3.1)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(CMakePrintHelpers)

# Include all our helper functions and macros.
include(cmake/QuadGSUtils.cmake)

SET(BUILD_TESTS "TRUE" CACHE BOOL "Build Tests")
SET(WITH_VISION "TRUE" CACHE BOOL "Build with vision support.")

#SET(CMAKE_VERBOSE_MAKEFILE ON)
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

IF(CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
ENDIF(CMAKE_COMPILER_IS_GNUCC)
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

# Set some warning flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic -Werror")

# Fix boost linking for log.
add_definitions(-D BOOST_LOG_DYN_LINK)

# Print header
message(STATUS "QuadGS - Ground station for quad copter system based on QuadFC.")
# Find boost
find_package(Boost 1.60.0 REQUIRED COMPONENTS 
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

find_package(Readline REQUIRED)

if(WITH_VISION)
    message(STATUS "Building with vision support.")

    SET(VERSION_REQ_CUDA "8.0")
    SET(VERSION_REQ_OCV "3.1")
    find_package(OpenCV ${VERSION_REQ_OCV} REQUIRED)
    find_package(CUDA ${VERSION_REQ_CUDA} REQUIRED)
    include_directories(${CUDA_INCLUDE_DIRS})
endif()


# Include gtest if tests will be built
if(BUILD_TESTS)
    # Download and unpack googletest at configure time
    configure_file("${CMAKE_CURRENT_LIST_DIR}/cmake/CMakeLists.txt.in" googletest-download/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
      RESULT_VARIABLE result
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
    if(result)
      message(FATAL_ERROR "CMake step for googletest failed: ${result}")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
      RESULT_VARIABLE result
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
    if(result)
      message(FATAL_ERROR "Build step for googletest failed: ${result}")
    endif()
    # Add googletest directly to our build. This defines
    # the gtest and gtest_main targets.
    add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                     ${CMAKE_BINARY_DIR}/googletest-build)
endif()
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
add_subdirectory(UI)
add_subdirectory(top)
add_subdirectory(test)


