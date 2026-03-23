add_executable(main main.cpp)

add_subdirectory(problems)

target_link_libraries(main distributed pagmo)