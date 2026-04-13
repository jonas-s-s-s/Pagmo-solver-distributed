#include <catch2/catch_test_macros.hpp>
#include "dealer_socket.h"
#include "router_socket.h"

TEST_CASE("Dealer‑Router round‑trip via inproc", "[socket]") {
    zmq::context_t ctx;
    distributed::router_socket router(ctx);
    distributed::dealer_socket dealer(ctx);

    router.bind("inproc://test_router");
    dealer.connect("inproc://test_router");
    dealer.set_routing_id("dealer1");

    SECTION("Send and receive message") {
        std::vector<std::byte> payload{std::byte{0x42}, std::byte{0x43}};
        dealer.send(MsgType::ALLOCATE_WORK, payload);

        auto [id, type, received] = router.receive();
        REQUIRE(type == MsgType::ALLOCATE_WORK);
        REQUIRE(received == payload);
    }

    SECTION("Multipart message integrity") {
        std::vector<std::byte> payload{std::byte{0x01}, std::byte{0x02}};
        dealer.send(MsgType::WORK_RESULTS, payload);
        auto [id, type, received] = router.receive();
        REQUIRE(type == MsgType::WORK_RESULTS);
        REQUIRE(received.size() == 2);
        REQUIRE(received[0] == std::byte{0x01});
    }

    SECTION("Routing ID preservation") {
        dealer.set_routing_id("custom_id");
        dealer.send(MsgType::WORKER_JOIN, {});
        auto [id, type, _] = router.receive();
        REQUIRE(id == "custom_id");
    }
}
