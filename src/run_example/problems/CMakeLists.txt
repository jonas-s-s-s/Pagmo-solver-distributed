add_library(problems SHARED
        example_problem_single_objective.cpp
)
target_include_directories(problems PUBLIC include)

target_link_libraries(problems PUBLIC pagmo island_test distributed_discovery)

# This ensures that the DLL will be in the same directory as main.exe
set_target_properties(problems
        PROPERTIES
        # removes "lib" prefix on GCC based platforms
        PREFIX ""
        # Make sure the file name of this lib is "problems"
        OUTPUT_NAME "problems"

        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/run_example/"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/run_example/"
)