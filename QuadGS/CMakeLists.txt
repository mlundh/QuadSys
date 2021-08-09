# cmake needs this line
cmake_minimum_required(VERSION 3.1)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(CMakePrintHelpers)

# Include all our helper functions and macros.
include(cmake/QuadGSUtils.cmake)

SET(BUILD_TESTS "TRUE" CACHE BOOL "Build Tests")
SET(WITH_VISION "TRUE" CACHE BOOL "Build with vision support.")
SET(ASAN "TRUE" CACHE BOOL "Build with vision support.")
SET(STOP_ON_ERROR "FALSE" CACHE BOOL "Stop build on first error.")

#SET(CMAKE_VERBOSE_MAKEFILE ON)
# Define project name
project(QuadGS VERSION 0.1)

# Set project properties
set(CMAKE_CXX_STANDARD 14) # C++14...
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

if(ASAN)
  set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
  set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
endif()

# Set some warning flags -pedantic should be added but current google test does not work with it.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -Werror")

#Stop on first error if needed
if(STOP_ON_ERROR)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfatal-errors")
endif()

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

find_package(editline REQUIRED)

if(WITH_VISION)
    message(STATUS "Building with vision support.")
    SET(VERSION_REQ_CUDA "9.0")
    SET(VERSION_REQ_OCV "3.1")
    SET(VERSION_REQ_ZED "2")
    
    find_package(ZED ${VERSION_REQ_ZED} REQUIRED)
    find_package(OpenCV ${VERSION_REQ_OCV} REQUIRED)
    find_package(CUDA ${VERSION_REQ_CUDA} REQUIRED)
    find_package(GLUT REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(OpenGL REQUIRED)

    include_directories(SYSTEM ${CUDA_INCLUDE_DIRS})
    include_directories(SYSTEM ${OPENCV_INCLUDE_DIRS})
    include_directories(SYSTEM ${ZED_INCLUDE_DIRS})
    include_directories(${GLEW_INCLUDE_DIRS})
    include_directories(${GLUT_INCLUDE_PATH})

    link_directories(${ZED_LIBRARY_DIR})
    link_directories(${OpenCV_LIBRARY_DIRS})
    link_directories(${CUDA_LIBRARY_DIRS})
    link_directories(${GLEW_LIBRARY_DIRS})
    link_directories(${GLUT_LIBRARY_DIRS})
    link_directories(${OpenGL_LIBRARY_DIRS})

    add_definitions(-DWITH_VISION)
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


include_directories(Interfaces)

# Targets that we develop
enable_testing()
add_subdirectory(Modules)
add_subdirectory(Common)
add_subdirectory(top)



