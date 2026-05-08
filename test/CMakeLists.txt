include(CTest)
enable_testing()
include(Catch)

add_executable(unit_tests
    # Phase 1: Foundation Data Structures
    discovery/lru_cache_test.cpp
    discovery/hash_cache_test.cpp
    
    # Phase 2: Serialization & Messages
    messages/serialization_test.cpp
    
    # Phase 3: Utilities & Helpers
    logic/population_tools_test.cpp
    logic/settings_test.cpp
    utils/platform_utils_test.cpp
    
    # Phase 4: Core Domain Logic
    logic/worker_info_repository_test.cpp

    # Phase 5: Discovery Layer
    discovery/udp_registry_test.cpp
    
    # Phase 6: Socket Layer
    sockets/socket_roundtrip_test.cpp

)

target_link_libraries(unit_tests PRIVATE
    distributed_discovery
    distributed_logic
    distributed_messages
    distributed_sockets
    distributed_utils
    distributed_dynamic
    Catch2::Catch2WithMain
    pagmo
    cppzmq
    libzmq
)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

# So the exe is besides all other required DLLs
set_target_properties(unit_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

catch_discover_tests(unit_tests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
