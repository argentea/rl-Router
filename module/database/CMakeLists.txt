cmake_minimum_required(VERSION 3.12)
project(database)
file(GLOB SOURCES
        src/*.cpp
        src/*.cc
        src/*.c)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/topo/src")


message(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${PROJECT_NAME} STATIC ${SOURCES})

# parser
target_link_libraries(${PROJECT_NAME} PUBLIC parser)
# topo
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ./topo/src)
target_link_libraries(${PROJECT_NAME} PUBLIC topo)
# net
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ./net/src)
target_link_libraries(${PROJECT_NAME} PUBLIC net)
# grid
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ./grid/src)
target_link_libraries(${PROJECT_NAME} PUBLIC grid)
# Rsyn
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ../../thirdparty/rsyn/src)
target_link_libraries(${PROJECT_NAME} PUBLIC rsyn)



# test
add_executable(${PROJECT_NAME}_test test.cpp)
target_link_libraries(${PROJECT_NAME}_test parser)
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME})

add_subdirectory(topo)
add_subdirectory(layer)
add_subdirectory(net)
add_subdirectory(grid)
