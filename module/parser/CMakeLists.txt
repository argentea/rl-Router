project(parser)
file(GLOB SOURCES
        src/*.cpp
        src/*.cc
        src/*.c)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")

add_library(${PROJECT_NAME} STATIC ${SOURCES})

## logger
target_link_libraries(${PROJECT_NAME} PUBLIC logger)

## topo
## they should use the same basic topo class
target_link_libraries(${PROJECT_NAME} PUBLIC topo)
# boost
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_LIBRARIES})

# Rsyn
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ../../thirdparty/rsyn/src)
target_link_libraries(${PROJECT_NAME} PUBLIC rsyn)

# test
add_executable(${PROJECT_NAME}_test test.cpp)
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME})

