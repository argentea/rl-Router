project(layer)
file(GLOB SOURCES
        src/*.cpp
        src/*.cc
        src/*.c)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

# parser
target_link_libraries(${PROJECT_NAME} STATIC parser)

# test
add_executable(${PROJECT_NAME}_test test.cpp)
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME})
