project(global)
add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
	INTERFACE
	${PROJECT_SOURCE_DIR})


add_subdirectory(logger)
add_subdirectory(parser)
add_subdirectory(database)
add_subdirectory(router)
