#pragma once
#include <thread>

#include "dealer_socket.h"
#include "zmq.hpp"
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

    // Allows this network client to run in the background (function run_client)
    std::thread _clientThread;

    void _start_worker_thread(pagmo::algorithm& algo, pagmo::population& pop);

    void _handleWorkerSocketMsg();
    void _handleThreadSocketMsg();

    /**
     * Spawns a new thread and then simply calls algo.evolve(pop), this causes
     * the evolution algorithm to use only a single CPU core
     */
    void _single_threaded_worker(pagmo::algorithm& algo, pagmo::population& pop);

    /**
     * Spawns a new thread and then uses pagmo::archipelago to evolve the algorithm,
     * causing multiple CPU cores to be used
     */
    void _archipelago_based_worker_multiobjective(pagmo::algorithm& algo, pagmo::population& pop);

    enum worker_mode
    {
        SINGLE_THREADED,
        ARCHIPELAGO_BASED,
    };

    worker_mode _workerMode;

    unsigned _archipelagoEvolutionCount;

public:
    /**
     * Constructs a distributed worker, which will connect and accept work from a specified controller
     * @param controllerAddress Controller's address
     * @param workerMode Can be either SINGLE_THREADED or ARCHIPELAGO_BASED (default)
     * @param archipelagoEvolutionCount How many times to run the archipelago evolution (when using ARCHIPELAGO_BASED)
     */
    explicit distributed_worker(const std::string& controllerAddress, worker_mode workerMode = ARCHIPELAGO_BASED, unsigned archipelagoEvolutionCount = 4);

    void client_loop();
    void run_client();

    ~distributed_worker();
};
