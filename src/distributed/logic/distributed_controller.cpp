#include "distributed_controller.h"

#include <iostream>

distributed_controller::distributed_controller(const std::string& controllerAddress) : _workersSocket{_ctx}
{
    _workersSocket.bind(controllerAddress);
}

void distributed_controller::run_server()
{
    _serverThread = std::thread([this]
    {
        for (;;)
        {
            auto [workerId, type, binary] = _workersSocket.receive();

            // Incoming message
            std::cout << "[" << static_cast<int>(type) << "] from: " << workerId << std::endl;

            // Determine controller's action depending on the message type
            switch (type)
            {
            case MsgType::WORKER_JOIN:
                //const auto binRequest = msgpack23::unpack<BinariesRequest>(binary);
                _freeWorkersPool.emplace(workerId);
                std::cout << "Worker " << workerId << " joined" << std::endl;
                std::cout << "Free workers: " << _freeWorkersPool.size() << std::endl;
                break;
            case MsgType::WORKER_LEAVE:
                _freeWorkersPool.erase(workerId);
                // TODO: Handle busy worker leave
                _busyWorkersPool.erase(workerId);
                break;
            default:
                std::cerr << "WARNING: " << workerId << " sent unhandled message type: " << static_cast<int>(type)
                    <<
                    std::endl;
            }
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
