project(logger)
file(GLOB SOURCES
        src/*.cpp
        src/*.cc
        src/*.c)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")

add_library(${PROJECT_NAME} STATIC ${SOURCES})

add_executable(${PROJECT_NAME}_test test.cpp)
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME})
