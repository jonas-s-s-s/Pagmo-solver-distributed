#include <catch2/catch_test_macros.hpp>
#include "distributed/sockets/include/router_socket.h"
#include "distributed/sockets/include/dealer_socket.h"
#include "distributed/messages/include/MsgType.h"
#include <thread>

TEST_CASE("router_socket routing ID preservation", "[socket]") {
    zmq::context_t ctx;
    distributed::router_socket router{ctx};
    router.bind("inproc://router_test");

    distributed::dealer_socket dealer{ctx};
    dealer.set_routing_id("client1");
    dealer.connect("inproc://router_test");

    std::thread t([&]() {
        dealer.send(MsgType::WORKER_JOIN, std::vector<std::byte>{});
        auto [type, _] = dealer.receive();
        REQUIRE(type == MsgType::WORK_RESULTS);
    });

    auto [id, type, payload] = router.receive();
    REQUIRE(id == "client1");
    REQUIRE(type == MsgType::WORKER_JOIN);
    router.send(id, MsgType::WORK_RESULTS, std::vector<std::byte>{});

    t.join();
}
