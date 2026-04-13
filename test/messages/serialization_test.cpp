#include <catch2/catch_test_macros.hpp>
#include "MsgType.h"
#include "vector_serialize.h"
#include "vector_deserialize.h"
#include <pagmo/problems/null_problem.hpp>
#include <pagmo/algorithms/null_algorithm.hpp>

TEST_CASE("Serialization round-trip", "[serialize]") {
    pagmo::problem prob{pagmo::null_problem{2}};
    pagmo::population pop{prob, 10};
    pagmo::algorithm algo{pagmo::null_algorithm{}};

    SECTION("work_container") {
        work_container original{algo, pop, "worker123", 5};
        auto serialized = vector_serialize(original);
        auto deserialized = vector_deserialize<work_container>(serialized);

        REQUIRE(deserialized.algo.get_name() == original.algo.get_name());
        REQUIRE(deserialized.pop.size() == original.pop.size());
        REQUIRE(deserialized.preferredWorkerId == original.preferredWorkerId);
        REQUIRE(deserialized.cycleCount == original.cycleCount);
    }

    SECTION("get_dll_request") {
        get_dll_request original{"my_udp.dll"};
        auto serialized = vector_serialize(original);
        auto deserialized = vector_deserialize<get_dll_request>(serialized);
        REQUIRE(deserialized.dll_name == original.dll_name);
    }

    SECTION("dll_binary_container") {
        std::vector<std::byte> fake_dll{std::byte{0x01}, std::byte{0x02}};
        dll_binary_container original{"lib.so", fake_dll};
        auto serialized = vector_serialize(original);
        auto deserialized = vector_deserialize<dll_binary_container>(serialized);
        REQUIRE(deserialized.dll_name == original.dll_name);
        REQUIRE(deserialized.dll_file.has_value());
        REQUIRE(deserialized.dll_file.value() == fake_dll);
    }

    SECTION("empty buffer throws") {
        std::vector<std::byte> empty;
        REQUIRE_THROWS_AS(vector_deserialize<work_container>(empty), std::runtime_error);
    }
}
