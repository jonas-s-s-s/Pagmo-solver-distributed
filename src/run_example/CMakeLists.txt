add_executable(distributed-main-example main.cpp)

add_subdirectory(problems)

target_link_libraries(distributed-main-example distributed pagmo)
set_target_properties(distributed-main-example
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${DISTRIBUTED_SOLVER_RUNTIME_OUTPUT_DIR}"
)


add_executable(main-e1 main_e1.cpp)
target_link_libraries(main-e1 distributed pagmo)
set_target_properties(main-e1
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${DISTRIBUTED_SOLVER_RUNTIME_OUTPUT_DIR}"
)
