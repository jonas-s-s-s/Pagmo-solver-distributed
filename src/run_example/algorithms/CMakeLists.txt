add_library(island_test islandTest.cpp dll_problem_wrapper.cpp)
target_include_directories(island_test PUBLIC include)

target_link_libraries(island_test PUBLIC pagmo problems distributed)