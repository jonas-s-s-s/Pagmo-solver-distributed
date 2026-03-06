#include "distributed_controller.h"

#include <iostream>

#include "aixlog.hpp"

//#####################################################################################
//# Handling of socket messages
//#####################################################################################

void distributed_controller::_handleWorkersSocketMsg()
{
    auto [workerId, type, binary] = _workersSocket.receive();
    LOG(TRACE) << "Got MsgType[" << static_cast<int>(type) << "] from " << workerId << std::endl;

    switch (type)
    {
    case MsgType::WORKER_JOIN:
        LOG(TRACE) << "Worker " << workerId << " joined" << std::endl;
        _add_free_worker(workerId);
        LOG(TRACE) << "Free workers: " << _freeWorkersPool.size() << std::endl;
        break;

    case MsgType::WORKER_LEAVE:

        _freeWorkersPool.erase(workerId);
        // TODO: Handle busy worker leave - push back into islandsWaitingForAlloc?

        break;
    case MsgType::WORK_RESULTS:
        {
            const auto myIslandId = _workAllocationMap.at(workerId);
            // Pass results from worker to island
            _islandsSocket.send(myIslandId, MsgType::WORK_RESULTS, binary);
            // Remove this {workerId, islandId} work allocation
            _workAllocationMap.erase(workerId);
            // Add worker back into the free pool
            _add_free_worker(workerId);
        }
        break;
    default:
        LOG(WARNING) << workerId << " sent unhandled message type: " << static_cast<int>(type) << std::endl;
    }
}

void distributed_controller::_handleIslandsSocketMsg()
{
    auto [islandId, type, binary] = _islandsSocket.receive();
    LOG(TRACE) << "Got MsgType[" << static_cast<int>(type) << "] from " << islandId << std::endl;

    switch (type)
    {
    case MsgType::ALLOCATE_WORK:
        /*
         * Island Requested Work Allocation
         *
         * Allocate worker to this island if there are free workers,
         * otherwise put island into the _islandsWaitingForAlloc set
        */
        if (_freeWorkersPool.empty())
        {
            _islandsWaitingForAlloc.emplace(islandId, binary);
            LOG(TRACE) << islandId << "is waiting for allocation" << std::endl;
        }
        else
        {
            _allocate_worker_to_island(islandId, binary);
        }
        break;
    default:
        LOG(WARNING) << islandId << " sent unhandled message type: " << static_cast<int>(type) << std::endl;
    }
}

//#####################################################################################
//# Controller data logic
//#####################################################################################

void distributed_controller::_allocate_worker_to_island(const std::string& islandId,
                                                        const std::vector<std::byte>& workData)
{
    const std::string workerId = *_freeWorkersPool.begin();
    _freeWorkersPool.erase(workerId);
    _workAllocationMap.emplace(workerId, islandId);
    LOG(TRACE) << islandId << "has been allocated " << workerId << std::endl;

    _workersSocket.send(workerId, MsgType::ALLOCATE_WORK, workData);
}

std::tuple<std::string, std::vector<std::byte>> distributed_controller::_pop_waiting_island()
{
    const auto islandRecord = _islandsWaitingForAlloc.begin();
    const std::string islandId = islandRecord->first;
    const std::vector<std::byte> workData = islandRecord->second;

    _islandsWaitingForAlloc.erase(islandRecord);
    return {islandId, workData};
}

void distributed_controller::_add_free_worker(const std::string& workerId)
{
    /*
     * A worker is free (joined or finished work)
     *
     * If there are islands waiting for allocation, allocate worker to one of them,
     * otherwise save worker ID into the free workers pool
    */
    _freeWorkersPool.emplace(workerId);

    if (!_islandsWaitingForAlloc.empty())
    {
        const auto [islandId, workData] = _pop_waiting_island();
        _allocate_worker_to_island(islandId, workData);
    }
}

//#####################################################################################
//# Sockets setup & initialization
//#####################################################################################

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

    try
    {
        _workersSocket.bind(controllerAddress);
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Socket failed to bind to address " << controllerAddress << " due to the following error: " << e.
            what() << std::endl;
        LOG(ERROR) << "Check if this port isn't already in use" << std::endl;
        throw e;
    }

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
