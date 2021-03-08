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
target_link_libraries(${PROJECT_NAME} PUBLIC topo)
# net
target_link_libraries(${PROJECT_NAME} PUBLIC net)
# grid
#target_link_libraries(${PROJECT_NAME} PUBLIC grid)


# test
add_executable(${PROJECT_NAME}_test test.cpp)
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME} parser)
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME} database)

add_subdirectory(topo)
add_subdirectory(layer)
add_subdirectory(net)
#add_subdirectory(grid)
