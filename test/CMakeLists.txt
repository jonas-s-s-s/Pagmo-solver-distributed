include(CTest)
enable_testing()
include(Catch)

add_executable(unit_tests
    discovery/lru_cache_test.cpp
    discovery/hash_cache_test.cpp
    discovery/udp_registry_test.cpp
    discovery/lib_loader_test.cpp
    logic/population_tools_test.cpp
    logic/settings_test.cpp
    logic/worker_info_repository_test.cpp
    logic/work_plan_test.cpp
    messages/serialization_test.cpp
    sockets/dealer_socket_test.cpp
    sockets/router_socket_test.cpp
    utils/platform_utils_test.cpp
    integration/basic_system_test.cpp
)

target_link_libraries(unit_tests PRIVATE
    distributed_discovery
    distributed_messages
    distributed_logic
    distributed_sockets
    distributed_utils
    pagmo
    Catch2::Catch2WithMain
)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/distributed/discovery/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/distributed/messages/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/distributed/logic/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/distributed/sockets/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/distributed/utils/include
)

target_compile_definitions(unit_tests PRIVATE TEST_BUILD)

catch_discover_tests(unit_tests)
