include(CTest)
enable_testing()

include(Catch)

add_executable(unit_tests
    # Foundations
    discovery/lru_cache_test.cpp
    discovery/hash_cache_test.cpp
)

# Link libraries we are testing + Catch2
target_link_libraries(unit_tests PRIVATE
    distributed_discovery
    distributed_logic
    distributed_messages
    distributed_sockets
    distributed_utils
    Catch2::Catch2WithMain
    pagmo
)

target_link_libraries(unit_tests PRIVATE
    distributed_discovery
    Catch2::Catch2WithMain
    pagmo

)

# Auto-discover test cases using Catch2's --list-tests command
target_include_directories(unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
catch_discover_tests(unit_tests)


