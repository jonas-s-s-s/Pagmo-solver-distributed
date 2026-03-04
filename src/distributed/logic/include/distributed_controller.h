#pragma once
#include <thread>
#include <unordered_set>

#include "router_socket.h"
#include "zmq.hpp"
#include "zmq_addon.hpp"



class distributed_controller
{
    // Helper struct to group UDI and its work together
    struct work_allocation_unit
    {
        std::string islandId;
        std::vector<std::byte> workData;
    };

    zmq::context_t _ctx;
    zmq::active_poller_t _poller;

    distributed::router_socket _workersSocket;
    distributed::router_socket _islandsSocket;
    std::thread _serverThread;

    std::unordered_map<std::string, std::vector<std::byte>> _islandsWaitingForAlloc{};
    std::unordered_set<std::string> _freeWorkersPool{};
    // Pairs of {workerID, islandID} indicate which worker is currently being used by which island
    std::unordered_map<std::string, std::string> _workAllocationMap{};

    void _handleWorkersSocketMsg();
    void _handleIslandsSocketMsg();

    void _allocate_worker_to_island(const std::string& islandId, const std::vector<std::byte>& workData);

public:
    explicit distributed_controller(const std::string& controllerAddress);

    void run_server();

    ~distributed_controller();
};
