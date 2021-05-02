project(grid)
file(GLOB SOURCES
        src/*.cpp
        src/*.cc
        src/*.c)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../..")

add_library(${PROJECT_NAME} STATIC ${SOURCES})

# parser
target_link_libraries(${PROJECT_NAME} PUBLIC parser)
# layer
target_link_libraries(${PROJECT_NAME} PUBLIC layer)


# test
add_executable(${PROJECT_NAME}_test test.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC )
target_link_libraries(${PROJECT_NAME}_test PUBLIC grid)
