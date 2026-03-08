add_library(problems SHARED
        example_problem_single_objective.cpp
        base_problem.cpp
)
target_include_directories(problems PUBLIC include)

target_link_libraries(problems PUBLIC pagmo distributed)

# This ensures that the DLL will be in the same directory as main.exe
set_target_properties(problems
        PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/run_example/"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/run_example/"
)