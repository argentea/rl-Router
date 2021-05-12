project(grid)
file(GLOB SOURCES
        src/*.cpp
        src/*.cc
        src/*.c)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../..)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ../..)

# parser
target_link_libraries(${PROJECT_NAME} PUBLIC parser)
# layer
target_link_libraries(${PROJECT_NAME} PUBLIC layer)
# Rsyn
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ../../../thirdparty/rsyn/src)
target_link_libraries(${PROJECT_NAME} PUBLIC rsyn)


# test
add_executable(${PROJECT_NAME}_test test.cpp)
target_link_libraries(${PROJECT_NAME}_test PUBLIC grid)
