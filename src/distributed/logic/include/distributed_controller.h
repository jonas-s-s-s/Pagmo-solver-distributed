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
    zmq::active_poller_t _poller;

    distributed::router_socket _workersSocket;
    distributed::router_socket _islandsSocket;
    std::thread _serverThread;

    std::unordered_set<std::string> _freeWorkersPool{};
    std::unordered_set<std::string> _busyWorkersPool{};

    void _handleWorkersSocketMsg();
    void _handleIslandsSocketMsg();

public:
    explicit distributed_controller(const std::string& controllerAddress);

    void run_server();

    ~distributed_controller();
};