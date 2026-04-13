#include <catch2/catch_test_macros.hpp>
#include "distributed/sockets/include/dealer_socket.h"
#include "distributed/sockets/include/router_socket.h"
#include "distributed/messages/include/MsgType.h"
#include <thread>
#include <pagmo/problems/schwefel.hpp>

TEST_CASE("dealer_socket round-trip via inproc", "[socket]") {
    zmq::context_t ctx;
    distributed::router_socket router{ctx};
    router.bind("inproc://dealer_test");

    distributed::dealer_socket dealer{ctx};
    dealer.set_routing_id("test_dealer");
    dealer.connect("inproc://dealer_test");

    std::thread server([&]() {
        auto [id, type, payload] = router.receive();
        REQUIRE(type == MsgType::ALLOCATE_WORK);
        router.send(id, MsgType::WORK_RESULTS, payload);
    });

    pagmo::problem prob{pagmo::schwefel(3)};
    work_container wc{pagmo::algorithm{}, pagmo::population{prob, 10}};
    dealer.send(MsgType::ALLOCATE_WORK, wc);
    auto [type, payload] = dealer.receive();
    REQUIRE(type == MsgType::WORK_RESULTS);

    server.join();
}
