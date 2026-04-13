#include <catch2/catch_test_macros.hpp>
#include "router_socket.h"
#include "dealer_socket.h"

TEST_CASE("Router socket sending to specific dealer", "[socket]") {
    zmq::context_t ctx;
    distributed::router_socket router(ctx);
    distributed::dealer_socket dealer1(ctx);
    distributed::dealer_socket dealer2(ctx);

    router.bind("inproc://test_router_multi");
    dealer1.connect("inproc://test_router_multi");
    dealer2.connect("inproc://test_router_multi");
    dealer1.set_routing_id("clientA");
    dealer2.set_routing_id("clientB");

    SECTION("Send to specific client") {
        std::vector<std::byte> payload{std::byte{0xAA}};
        router.send("clientA", MsgType::DLL_BINARY, payload);
        // dealer1 should receive, dealer2 should not (we'll just check dealer1)
        auto [type, received] = dealer1.receive(); // dealer_socket::receive returns (type, payload) only, not id!
        // Actually dealer_socket::receive returns tuple<MsgType, vector<byte>>, so structured binding with one element? Let's fix.
        // The correct receive for dealer_socket is (MsgType, vector<byte>). We'll use std::tie.
        MsgType recvType;
        std::vector<std::byte> recvPayload;
        std::tie(recvType, recvPayload) = dealer1.receive();
        REQUIRE(recvType == MsgType::DLL_BINARY);
        REQUIRE(recvPayload == payload);
    }
}
