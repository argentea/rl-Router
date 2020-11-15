set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS filesystem program_options REQUIRED)
find_package(Threads REQUIRED)

file(GLOB_RECURSE SOURCES
        *.cpp
        *.cc
        *.c)

# inhibit all warning messages from rsyn
add_compile_definitions(BOOST_ALLOW_DEPRECATED_HEADERS)
file(GLOB_RECURSE RSYN_SOURCES
        lib/rsyn/*.cpp
        lib/rsyn/*.cc
        lib/rsyn/*.c)
set_source_files_properties(${RSYN_SOURCES} PROPERTIES COMPILE_FLAGS "-w")

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
include_directories(SYSTEM lib/rsyn/src)
include_directories(SYSTEM lib/rsyn/src/rsyn/export)
include_directories(SYSTEM lib/rsyn/include)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

link_directories(lib/rsyn/lib/linux)

add_executable(${PROJECT_NAME} ${SOURCES})
#set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS
#        "-static -Wl,--whole-archive -rdynamic -lpthread -Wl,--no-whole-archive")

target_link_libraries(${PROJECT_NAME} PUBLIC
        ${Boost_LIBRARIES}
        def
        lef
        Threads::Threads)
if (OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
endif ()
