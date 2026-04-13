#include <catch2/catch_test_macros.hpp>
#include "distributed/messages/include/MsgType.h"
#include "distributed/sockets/include/vector_deserialize.h"
#include "distributed/sockets/include/vector_serialize.h"
#include <vector>
#include <string>

TEST_CASE("serialization: round-trip for all message types", "[serialize]") {
    SECTION("work_container round-trip") {
        pagmo::algorithm algo{pagmo::de(10)};
        pagmo::population pop{pagmo::problem{pagmo::schwefel{5}}, 20};
        work_container original{algo, pop, "preferred_worker_123", 42};
        auto serialized = vector_serialize(original);
        auto deserialized = vector_deserialize<work_container>(serialized);
        REQUIRE(deserialized.preferredWorkerId == "preferred_worker_123");
        REQUIRE(deserialized.cycleCount == 42);
    }

    SECTION("get_dll_request round-trip") {
        get_dll_request req{"my_udp.dll"};
        auto serialized = vector_serialize(req);
        auto deserialized = vector_deserialize<get_dll_request>(serialized);
        REQUIRE(deserialized.dll_name == "my_udp.dll");
    }

    SECTION("dll_binary_container round-trip") {
        std::vector<std::byte> data{std::byte{0xAA}, std::byte{0xBB}};
        dll_binary_container orig{"libname.so", data};
        auto serialized = vector_serialize(orig);
        auto deserialized = vector_deserialize<dll_binary_container>(serialized);
        REQUIRE(deserialized.dll_name == "libname.so");
        REQUIRE(deserialized.dll_file.has_value());
        REQUIRE(deserialized.dll_file.value() == data);
    }

    SECTION("empty buffer throws") {
        REQUIRE_THROWS_AS(vector_deserialize<work_container>({}), std::runtime_error);
    }
}
