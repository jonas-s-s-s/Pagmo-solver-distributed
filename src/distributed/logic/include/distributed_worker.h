#pragma once
#include <numeric>
#include <thread>

#include "dealer_socket.h"
#include "zmq.hpp"
#include "MsgType.h"
#include "pair_socket.h"
#include "router_socket.h"
#include "zmq_addon.hpp"
#include "pagmo/archipelago.hpp"
#include "pagmo/utils/constrained.hpp"


class distributed_worker
{
    zmq::context_t _ctx;
    distributed::dealer_socket _workerSocket;

    std::string _workerId;

    // With this we can check multiple sockets for events in a callback-style syntax
    zmq::active_poller_t _poller;

    // Work-related code should run on this thread, so we don't block the client message loop
    std::thread _workerThread;
    // Cannot use pair_socket here, because we also need udp_registry's handler to send messages to socket too
    distributed::router_socket _threadSocket;

    // Allows this network client to run in the background (function run_client)
    std::thread _clientThread;

    void _start_worker_thread(pagmo::algorithm& algo, pagmo::population& pop);

    void _handle_Worker_Socket_Msg();
    void _handle_Thread_Socket_Msg();

    /**
     * Spawns a new thread and then simply calls algo.evolve(pop), this causes
     * the evolution algorithm to use only a single CPU core
     */
    void _single_threaded_worker(pagmo::algorithm& algo, pagmo::population& pop);

    /**
     * Spawns a new thread and then uses pagmo::archipelago to evolve the algorithm,
     * causing multiple CPU cores to be used. Multi-objective version.
     */
    void _archipelago_based_worker_multiobjective(pagmo::algorithm& algo, pagmo::population& pop);

    /**
     * Spawns a new thread and then uses pagmo::archipelago to evolve the algorithm,
     * causing multiple CPU cores to be used. Single-objective version.
     */
    void _archipelago_based_worker_singleobjective(pagmo::algorithm& algo, pagmo::population& pop);

    /**
     * Base archipelago worker, needs population sorting function which depends on if the problem is multi-objective
     * or single-objective.
     */
    void _archipelago_based_worker(pagmo::algorithm& algo, pagmo::population& pop,
                                   const std::function<std::vector<pagmo::pop_size_t>
                                       (const std::vector<pagmo::vector_double>&, std::size_t)>& popSorter);

    /**
     * Helper function to figure out the optimal island count for this archipelago worker based on hardware core count
     * @return Optimal number of islands
     */
    static unsigned _compute_optimal_island_count();

    /**
     * Helper function to merge the population of all islands (and their fitness) into two vectors
     * @param archi Input archipelago containing islands
     * @return Tuple of [allPopulations, allFitnesses]
     */
    static std::tuple<std::vector<pagmo::vector_double>, std::vector<pagmo::vector_double>> _merge_populations(
        pagmo::archipelago archi);

    enum worker_mode
    {
        SINGLE_THREADED,
        ARCHIPELAGO_BASED,
    };

    worker_mode _workerMode;

    unsigned _archipelagoEvolutionCount;

    // Helper for the client_loop() function
    bool _firstRun = true;
public:
    /**
     * Constructs a distributed worker, which will connect and accept work from a specified controller
     * @param controllerAddress Controller's address
     * @param workerMode Can be either SINGLE_THREADED or ARCHIPELAGO_BASED (default)
     * @param archipelagoEvolutionCount How many times to run the archipelago evolution (when using ARCHIPELAGO_BASED)
     */
    explicit distributed_worker(const std::string& controllerAddress,
                                worker_mode workerMode = SINGLE_THREADED, //TODO: CHANGE
                                unsigned archipelagoEvolutionCount = 1); //TODO: CHANGE

    void client_loop();
    void run_client();

    ~distributed_worker();

     /**
     * This function exists so udp_registry can call it and get a DLL file from the controller
     * @param lib_name Name of the DLL
     * @return DLL file as vector of bytes, or empty optional if not found
     */
    std::optional<std::vector<std::byte>> get_dll_from_controller(const std::string& lib_name);
};
