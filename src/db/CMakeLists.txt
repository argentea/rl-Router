project(db)
# test
add_executable(test_db test_db.cpp)
target_link_libraries(test_db 
	global
)

add_subdirectory(layer)
