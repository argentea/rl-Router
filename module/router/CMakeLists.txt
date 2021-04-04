project(router)
file(GLOB SOURCES
        src/*.cpp
        src/*.cc
        src/*.c)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")


add_library(${PROJECT_NAME} STATIC ${SOURCES})

# database
target_link_libraries(${PROJECT_NAME} PUBLIC database)

# test
add_executable(${PROJECT_NAME}_test test.cpp)
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME} fmt::fmt)
