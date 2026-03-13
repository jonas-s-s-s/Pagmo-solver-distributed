#pragma once
#include <thread>
#include <unordered_set>

#include "dll_locator.h"
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

    // Islands which cannot be allocated to any worker (_freeWorkersPool is empty) are stored here along with their data
    std::unordered_map<std::string, std::vector<std::byte>> _islandsWaitingForAlloc{};
    // Worker nodes ready to be allocated to an island
    std::unordered_set<std::string> _freeWorkersPool{};
    // Pairs of {workerID, islandID} indicate which worker is currently being used by which island
    std::unordered_map<std::string, std::string> _workAllocationMap{};

    void _add_free_worker(const std::string& workerId);
    void _handle_Workers_Socket_Msg();
    void _handle_Islands_Socket_Msg();

    void _allocate_worker_to_island(const std::string& islandId, const std::vector<std::byte>& workData);
    std::tuple<std::string, std::vector<std::byte>> _pop_waiting_island();

    // Helper class for finding and reading local DLL files (containing UDPs)
    dll_locator _dll_locator{};
public:
    explicit distributed_controller(const std::string& controllerAddress);

    void run_server();

    ~distributed_controller();
};
