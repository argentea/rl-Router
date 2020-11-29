#########
# Setup #
#########

# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.8)

# Message
message(STATUS "rlrouter develop version")
message(STATUS ${CMAKE_CURRENT_SOURCE_DIR})

# Find includes in the build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Project's name
project(rlrouter)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
message(${CMAKE_EXPORT_COMPILE_COMMANDS})
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set (CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install")
endif()
message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(src)


