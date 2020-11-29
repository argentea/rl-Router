project(layer)
file(GLOB SOURCES 
	"${CMAKE_CURRENT_SOURCE_DIR}/*.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
	)
add_library(${PROJECT_NAME} STATIC ${SOURCES})

# test
add_executable(test_layer test_layer.cpp)
target_link_libraries(test_layer ${PROJECT_NAME})
