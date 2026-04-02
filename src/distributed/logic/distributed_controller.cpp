#include "distributed_controller.h"
#include "aixlog.hpp"

#include <iostream>

#include "vector_deserialize.h"
#include "vector_istreambuf.h"
#include "../discovery/include/udp_registry.h"

//#####################################################################################
//# Handling of socket messages
//#####################################################################################

void distributed_controller::_handle_Workers_Socket_Msg()
{
    auto [workerId, type, binary] = _workersSocket.receive();

    LOG(TRACE) << "[" << static_cast<int>(type) << "] from worker" << std::endl;

    switch (type)
    {
    case MsgType::WORKER_JOIN:
        // Do nothing if we already have the worker saved as "joined"
        if (_freeWorkersPool.contains(workerId))
            break;

        LOG(TRACE) << "Worker " << workerId << " joined" << std::endl;
        _add_free_worker(workerId);
        _workerInfoRepository.add_worker_record(workerId, {});
        LOG(DEBUG) << "Free workers: " << _freeWorkersPool.size() << std::endl;
        break;

    case MsgType::WORKER_LEAVE:

        _freeWorkersPool.erase(workerId);
        _workerInfoRepository.remove_worker_record(workerId);
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

    case MsgType::GET_DLL:
        {
            // Deserialize the DLL request so we can get the name of this DLL
            const auto dll_request = vector_deserialize<get_dll_request>(binary);

            // Get the DLL file via udp_registry (returns std::optional, Worker's udp_registry will throw if it's nullopt)
            const auto file = udp_registry::get().get_lib_as_file(dll_request.dll_name);
            _workersSocket.send(workerId,
                                MsgType::DLL_BINARY,
                                dll_binary_container{dll_request.dll_name, file}
            );
        }
        break;

    default:
        LOG(WARNING) << "WARNING: " << workerId << " sent unhandled message type: " << static_cast<int>(type) <<
            std::endl;
    }
}

void distributed_controller::_handle_Islands_Socket_Msg()
{
    auto [islandId, type, binary] = _islandsSocket.receive();
    LOG(TRACE) << "[" << static_cast<int>(type) << "] from island" << std::endl;

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
            LOG(DEBUG) << "Island " << islandId << "is waiting for allocation" << std::endl;
        }
        else
        {
            _allocate_worker_to_island(islandId, binary);
        }
        break;
    default:
        LOG(WARNING) << "WARNING: " << islandId << " sent unhandled message type: " << static_cast<int>(type) <<
            std::endl;
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
    // Set ping interval and ping timeout for the controller->workers socket
    _workersSocket.get_socket().set(zmq::sockopt::heartbeat_ivl, HEARTBEAT_INTERVAL);
    _workersSocket.get_socket().set(zmq::sockopt::heartbeat_timeout, HEARTBEAT_TIMEOUT);

    // Set disconnect message for workers (will be sent to us if worker is disconnected)
    int disconnectMsg = static_cast<int>(MsgType::WORKER_LEAVE);
    _workersSocket.get_socket().set(zmq::sockopt::disconnect_msg, zmq::buffer(&disconnectMsg, sizeof(disconnectMsg)));

    _poller.add(_workersSocket.get_socket(), zmq::event_flags::pollin,
                [this](zmq::event_flags e)
                {
                    _handle_Workers_Socket_Msg();
                });

    _poller.add(_islandsSocket.get_socket(), zmq::event_flags::pollin,
                [this](zmq::event_flags e)
                {
                    _handle_Islands_Socket_Msg();
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

    // TODO: Possibly define the ipc:// address as some global constant? Make the address unique per process so we can run multiple controllers?
    _islandsSocket.bind("ipc://distributed_controller_islands_socket");
}

void distributed_controller::run_server()
{
    _serverThread = std::thread([this]
    {
        try
        {
            for (;;)
            {
                _poller.wait(std::chrono::milliseconds{-1});
            }
        }
        catch (...)
        {
            // poller throws an exception if it's interrupted, see destructor, this way we can shut down the thread
            return;
        }
    });
}

distributed_controller::~distributed_controller()
{
    if (_serverThread.joinable())
    {
        // This should shut down the poller and its thread
        _workersSocket.get_socket().close();
        _islandsSocket.get_socket().close();
        _ctx.shutdown();

        _serverThread.join();
    }
}

//####################################################################################
//# Other public member functions
//#####################################################################################

worker_info_repository& distributed_controller::get_worker_info_repository()
{
    return _workerInfoRepository;
}
