#pragma once
#include <thread>

#include "dealer_socket.h"
#include "zmq.hpp"
#include "msgpack23.h"
#include "MsgType.h"
#include "pair_socket.h"
#include "zmq_addon.hpp"

class distributed_worker
{
    zmq::context_t _ctx;
    distributed::dealer_socket _workerSocket;

    std::string _workerId;

    // With this we can check multiple sockets for events in a callback-style syntax
    zmq::active_poller_t _poller;

    // Work-related code should run on this thread, so we don't block the client message loop
    std::thread _workerThread;
    distributed::pair_socket _threadSocket;

    void _start_worker_thread();

    void _handleWorkerSocketMsg();
    void _handleThreadSocketMsg();

public:
    explicit distributed_worker(const std::string& controllerAddress);

    void client_loop();
};