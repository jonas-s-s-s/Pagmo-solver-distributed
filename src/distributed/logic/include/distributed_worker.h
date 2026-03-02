#pragma once
#include <thread>

#include "zmq.hpp"
#include "msgpack23.h"
#include "MsgType.h"
#include "zmq_addon.hpp"

class distributed_worker
{
    zmq::context_t _ctx;
    zmq::socket_t _workerSocket;

    std::string _workerId;

    // With this we can check multiple sockets for events in a callback-style syntax
    zmq::active_poller_t _poller;

    // Work-related code should run on this thread, so we don't block the client message loop
    std::thread _workerThread;
    zmq::socket_t _threadSocket;

    void _start_worker_thread();

    void _handleWorkerSocketMsg();
    void _handleThreadSocketMsg();

    std::tuple<MsgType, std::vector<std::byte>> _receive();

    template <typename Packable>
    void _send(MsgType type, const Packable& payload);

    void _send(MsgType type, const std::vector<std::byte>& payload);

    void _send(MsgType type);

    static std::string _generate_worker_id();

public:
    explicit distributed_worker(const std::string& controllerAddress);

    void client_loop();
};

template <typename Packable>
void distributed_worker::_send(MsgType type, const Packable& payload)
{
    std::vector<std::byte> serialized;
    msgpack23::pack(std::back_inserter(serialized), payload);

    const int typeInt = static_cast<int>(type);
    _workerSocket.send(zmq::buffer(&typeInt, sizeof(typeInt)), zmq::send_flags::sndmore);
    _workerSocket.send(zmq::buffer(serialized), zmq::send_flags::none);
}
