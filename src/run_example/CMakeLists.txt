add_executable(main main.cpp)

add_subdirectory(algorithms)
add_subdirectory(problems)

target_link_libraries(main island_test distributed pagmo)