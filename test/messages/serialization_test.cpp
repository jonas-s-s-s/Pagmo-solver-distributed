#include <catch2/catch_test_macros.hpp>
#include "distributed/messages/include/MsgType.h"
#include "distributed/sockets/include/vector_serialize.h"
#include "distributed/sockets/include/vector_deserialize.h"
#include <pagmo/algorithm.hpp>
#include <pagmo/population.hpp>
#include <pagmo/problems/schwefel.hpp>

TEST_CASE("serialization round-trip work_container", "[serialize]") {
    pagmo::problem prob{pagmo::schwefel(5)};
    pagmo::algorithm algo{};
    work_container original{algo, pagmo::population{prob, 20}, "preferred_worker", 3};
    auto serialized = vector_serialize(original);
    auto deserialized = vector_deserialize<work_container>(serialized);
    REQUIRE(deserialized.preferredWorkerId == original.preferredWorkerId);
    REQUIRE(deserialized.cycleCount == original.cycleCount);
}

TEST_CASE("serialization round-trip get_dll_request", "[serialize]") {
    get_dll_request req{"test_dll.so"};
    auto serialized = vector_serialize(req);
    auto deserialized = vector_deserialize<get_dll_request>(serialized);
    REQUIRE(deserialized.dll_name == "test_dll.so");
}

TEST_CASE("serialization round-trip dll_binary_container", "[serialize]") {
    std::vector<std::byte> fileData{std::byte(0x01), std::byte(0x02), std::byte(0x03)};
    dll_binary_container original{"my_dll", fileData};
    auto serialized = vector_serialize(original);
    auto deserialized = vector_deserialize<dll_binary_container>(serialized);
    REQUIRE(deserialized.dll_name == "my_dll");
    REQUIRE(deserialized.dll_file.has_value());
    REQUIRE(deserialized.dll_file.value() == fileData);
}

TEST_CASE("serialization empty buffer throws", "[serialize]") {
    REQUIRE_THROWS(vector_deserialize<work_container>({}));
}
