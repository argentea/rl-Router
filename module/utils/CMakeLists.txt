project(utils)
file(GLOB SOURCES
        src/*.cpp
        src/*.cc
        src/*.c)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/.." "${CMAKE_CURRENT_SOURCE_DIR}/../..")

add_library(${PROJECT_NAME} STATIC ${SOURCES})

# Rsyn
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ../../thirdparty/rsyn/src)
target_link_libraries(${PROJECT_NAME} PUBLIC rsyn)
