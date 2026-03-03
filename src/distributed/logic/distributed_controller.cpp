#include "distributed_controller.h"

#include <iostream>

void distributed_controller::_handleWorkersSocketMsg()
{
    auto [workerId, type, binary] = _workersSocket.receive();
    std::cout << "[" << static_cast<int>(type) << "] from worker" << std::endl;

    switch (type)
    {
    case MsgType::WORKER_JOIN:
        std::cout << "Worker " << workerId << " joined" << std::endl;
        _freeWorkersPool.emplace(workerId);

        if (!_islandsWaitingForAlloc.empty())
        {
            const auto islandId = *_islandsWaitingForAlloc.begin();
            _islandsWaitingForAlloc.erase(islandId);
            _allocate_worker_to_island(islandId);
        }

        std::cout << "Free workers: " << _freeWorkersPool.size() << std::endl;
        break;
    case MsgType::WORKER_LEAVE:
        //const auto binRequest = msgpack23::unpack<BinariesRequest>(binary);

        _freeWorkersPool.erase(workerId);
        // TODO: Handle busy worker leave
        //_busyWorkersPool.erase(senderId);
        break;
    case MsgType::WORK_RESULTS:
        {
            const auto myIslandId = _workAllocationMap.at(workerId);
            // Remove this {workerId, islandId} work allocation
            _workAllocationMap.erase(workerId);
            // Add worker back into the free pool
            _freeWorkersPool.emplace(workerId);
            // Pass results from worker to island
            _islandsSocket.send(myIslandId, MsgType::WORK_RESULTS, binary);
        }
        break;
    default:
        std::cerr << "WARNING: " << workerId << " sent unhandled message type: " << static_cast<int>(type) << std::endl;
    }
}

void distributed_controller::_allocate_worker_to_island(const std::string& islandId)
{
    const std::string workerId = *_freeWorkersPool.begin();
    _freeWorkersPool.erase(workerId);
    _workAllocationMap.emplace(workerId, islandId);
    std::cout << "Island " << islandId << "has been allocated " << workerId << std::endl;

    // TODO: Work data must be sent
    _workersSocket.send(workerId, MsgType::ALLOCATE_WORK);
}

void distributed_controller::_handleIslandsSocketMsg()
{
    auto [islandId, type, binary] = _islandsSocket.receive();
    std::cout << "[" << static_cast<int>(type) << "] from island" << std::endl;

    switch (type)
    {
        case MsgType::ALLOCATE_WORK:
            if (_freeWorkersPool.empty())
            {
                _islandsWaitingForAlloc.emplace(islandId);
                std::cout << "Island " << islandId << "is waiting for allocation" << std::endl;
            } else
            {
                _allocate_worker_to_island(islandId);
            }
        break;
    default:
        std::cerr << "WARNING: " << islandId << " sent unhandled message type: " << static_cast<int>(type) << std::endl;
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
    _islandsSocket.bind("ipc://distributed_controller_islands_socket");
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
