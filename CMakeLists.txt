cmake_minimum_required(VERSION 3.12)

project(rl-router)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, Debug|Release" FORCE)
endif ()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set the output folder where your program will be created
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ferror-limit=1 -Wall -Wextra -Wno-sign-compare")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmax-errors=1 -Wall -Wextra -Wno-sign-compare")
endif ()
set(BOOST_ROOT /home/qutong/boost/)
set(Boost_NO_BOOST_CMAKE TRUE)
#set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_LIBS        OFF) 
find_package(Boost COMPONENTS filesystem program_options REQUIRED)
message(STATUS "Boost_INCLUDE_DIR = ${Boost_INCLUDE_DIR}")
message(STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}")



find_package(Boost COMPONENTS filesystem program_options REQUIRED)
find_package(Threads REQUIRED)

add_executable(test ./main.cpp)

add_subdirectory(module)
add_subdirectory(thirdparty)

set_target_properties(test PROPERTIES LINK_FLAGS "-Wl,--whole-archive -rdynamic -lpthread -Wl,--no-whole-archive")
target_link_libraries(test parser)
target_link_libraries(test database)
target_link_libraries(test router)
target_link_libraries(test utils)

