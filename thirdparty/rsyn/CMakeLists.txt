project(rsyn)
file(GLOB_RECURSE SOURCES
        src/*.cpp
        src/*.cc
        src/*.c)


# inhibit all warning messages from rsyn
add_compile_definitions(BOOST_ALLOW_DEPRECATED_HEADERS)
set_source_files_properties(${SOURCES} PROPERTIES COMPILE_FLAGS "-w")

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC src)
target_include_directories(${PROJECT_NAME} PUBLIC src/rsyn/export)
target_include_directories(${PROJECT_NAME} PUBLIC include)

# lef def
target_link_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/lib/linux)
target_link_libraries(${PROJECT_NAME} PUBLIC def lef)

# boost
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_LIBRARIES})

