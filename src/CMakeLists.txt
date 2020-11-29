project(global)
add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
	INTERFACE
		${PROJECT_SOURCE_DIR}
)

# no test
add_subdirectory(db)
