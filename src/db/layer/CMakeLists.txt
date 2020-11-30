project(layer)
file(GLOB SOURCES 
	"${CMAKE_CURRENT_SOURCE_DIR}/*.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
	)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test_layer.cpp)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/LayerList.*)
add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME} global)

# test
add_executable(test_layer test_layer.cpp)
target_link_libraries(test_layer global layer)
