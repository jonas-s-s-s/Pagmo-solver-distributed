add_library(problems SHARED
        example_problem_single_objective.cpp
)
target_include_directories(problems PUBLIC include)

target_link_libraries(problems PUBLIC pagmo distributed_discovery)

# This ensures that the DLL will be in the same directory as main.exe
set_target_properties(problems
        PROPERTIES
        # removes "lib" prefix on GCC based platforms
        PREFIX ""
        # Make sure the file name of this lib is "problems"
        OUTPUT_NAME "problems"

        LIBRARY_OUTPUT_DIRECTORY "${DISTRIBUTED_SOLVER_RUNTIME_OUTPUT_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${DISTRIBUTED_SOLVER_RUNTIME_OUTPUT_DIR}"
)



add_library(schwefel_udp SHARED
        schwefel_udp.cpp
)
target_include_directories(schwefel_udp PUBLIC include)

target_link_libraries(schwefel_udp PUBLIC pagmo distributed_discovery)

# This ensures that the DLL will be in the same directory as main.exe
set_target_properties(schwefel_udp
        PROPERTIES
        # removes "lib" prefix on GCC based platforms
        PREFIX ""
        # Make sure the file name of this lib is "schwefel_udp"
        OUTPUT_NAME "schwefel_udp"

        LIBRARY_OUTPUT_DIRECTORY "${DISTRIBUTED_SOLVER_RUNTIME_OUTPUT_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${DISTRIBUTED_SOLVER_RUNTIME_OUTPUT_DIR}"
)
