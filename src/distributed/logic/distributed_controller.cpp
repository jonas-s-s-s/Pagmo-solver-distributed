#include "distributed_controller.h"

#include <iostream>

void distributed_controller::_handleWorkersSocketMsg()
{
    auto [senderId, type, binary] = _workersSocket.receive();
    std::cout << "[" << static_cast<int>(type) << "] from worker" << std::endl;

    switch (type)
    {
    case MsgType::WORKER_JOIN:
        //const auto binRequest = msgpack23::unpack<BinariesRequest>(binary);
        _freeWorkersPool.emplace(senderId);
        std::cout << "Worker " << senderId << " joined" << std::endl;
        std::cout << "Free workers: " << _freeWorkersPool.size() << std::endl;
        break;
    case MsgType::WORKER_LEAVE:
        _freeWorkersPool.erase(senderId);
        // TODO: Handle busy worker leave
        _busyWorkersPool.erase(senderId);
        break;
    default:
        std::cerr << "WARNING: " << senderId << " sent unhandled message type: " << static_cast<int>(type) << std::endl;
    }
}

void distributed_controller::_handleIslandsSocketMsg()
{
    auto [senderId, type, binary] = _islandsSocket.receive();
    std::cout << "[" << static_cast<int>(type) << "] from island" << std::endl;

    switch (type)
    {

    }
}

distributed_controller::distributed_controller(const std::string& controllerAddress) : _workersSocket{_ctx},
    _islandsSocket{_ctx}
{
    _poller.add(_workersSocket.get_socket(), zmq::event_flags::pollin,
                [this](zmq::event_flags e)
                {
                    _handleWorkersSocketMsg();
                });

    _poller.add(_islandsSocket.get_socket(), zmq::event_flags::pollin,
                [this](zmq::event_flags e)
                {
                    _handleIslandsSocketMsg();
                });

    _workersSocket.bind(controllerAddress);
    _islandsSocket.bind("inproc://distributed_controller_islands_socket");
}

void distributed_controller::run_server()
{
    _serverThread = std::thread([this]
    {
        for (;;)
        {
            _poller.wait(std::chrono::milliseconds{-1});
        }
    });
}

distributed_controller::~distributed_controller()
{
    if (_serverThread.joinable())
    {
        _serverThread.join();
    }
}
