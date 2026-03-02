#pragma once
#include <thread>
#include <unordered_set>

#include "msgpack23.h"
#include "MsgType.h"
#include "router_socket.h"
#include "zmq.hpp"
#include "zmq_addon.hpp"

class distributed_controller
{
    zmq::context_t _ctx;
    distributed::router_socket _workersSocket;

    std::unordered_set<std::string> _freeWorkersPool{};
    std::unordered_set<std::string> _busyWorkersPool{};

    std::thread _serverThread;

    zmq::active_poller_t _poller;


public:
    explicit distributed_controller(const std::string& controllerAddress);

    void run_server();

    ~distributed_controller();
};