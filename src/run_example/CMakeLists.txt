add_executable(distributed-main-example main.cpp)
add_executable(distributed-main-benchmark benchmark_main.cpp)

add_subdirectory(problems)

target_link_libraries(distributed-main-example distributed pagmo)
set_target_properties(distributed-main-example
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${DISTRIBUTED_SOLVER_RUNTIME_OUTPUT_DIR}"
)

target_link_libraries(distributed-main-benchmark distributed pagmo)
set_target_properties(distributed-main-benchmark
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${DISTRIBUTED_SOLVER_RUNTIME_OUTPUT_DIR}"
)
