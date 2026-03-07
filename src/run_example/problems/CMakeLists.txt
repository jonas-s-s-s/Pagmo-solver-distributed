add_library(problems example_problem.cpp)
target_include_directories(problems PUBLIC include)

target_link_libraries(problems PUBLIC pagmo distributed)