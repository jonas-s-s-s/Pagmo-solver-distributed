#include "distributed_worker.h"

#include <iostream>
#include <random>

#include "UUID.h"
#include "vector_deserialize.h"
#include "vector_istreambuf.h"
#include "pagmo/archipelago.hpp"
#include "pagmo/islands/thread_island.hpp"
#include "pagmo/utils/multi_objective.hpp"

//#####################################################################################
//# Handling of socket messages
//#####################################################################################

void distributed_worker::_handle_Worker_Socket_Msg()
{
    auto [type, binary] = _workerSocket.receive();

    std::cout << "[" << static_cast<int>(type) << "] from controller" << std::endl;

    switch (type)
    {
    case MsgType::ALLOCATE_WORK:
        {
            // Deserialize received data
            auto wct = vector_deserialize<work_container>(binary);

            // TODO: Handle situation if thread is already running
            _start_worker_thread(wct.algo, wct.pop);
        }
        break;
    case MsgType::DLL_BINARY:
        // Controller has responded to Worker thread's GET_DLL message.
        // This means we need to forward this to the worker thread via the thread socket
        {
            // TODO: Remove, we can use constant sender ID
            auto dbc = vector_deserialize<dll_binary_container>(binary);

            _threadSocket.send("worker_dll_handler", MsgType::DLL_BINARY, binary);
        }
        break;

    default:
        std::cerr << "WARNING: controller sent unhandled message type: " << static_cast<int>(type) << std::endl;
    }
}

void distributed_worker::_handle_Thread_Socket_Msg()
{
    auto [senderId, type, binary] = _threadSocket.receive();

    switch (type)
    {
    case MsgType::WORK_RESULTS:
        // The standard execution path - worker thread completed work and returned results
        _workerThread.join();
        _workerSocket.send(MsgType::WORK_RESULTS, binary);
        break;

    case MsgType::GET_DLL:
        // Worker thread requests a DLL, which needs to be obtained from the controller.
        // This happens if the DLL is not available locally via udp_registry.
        // We thus forward it via workerSocket to controller, so it can process this request and respond with DLL_BINARY.
        _workerSocket.send(MsgType::GET_DLL, binary);
        break;

    default:
        std::cerr << "WARNING: worker thread socket sent unhandled message type: " << static_cast<int>(type) <<
            std::endl;
    }
}

//#####################################################################################
//# Worker logic
//#####################################################################################

void distributed_worker::_single_threaded_worker(pagmo::algorithm& algo, pagmo::population& pop)
{
    std::cout << "Single-threaded worker started... " << std::endl;
    _workerThread = std::thread(
        [this, algo, pop]()
        {
            distributed::dealer_socket output{this->_ctx};
            output.set_routing_id("worker_main");
            output.connect("ipc://thread_socket");

            std::cout << "Running algorithm: " << algo.get_name() << std::endl;
            const pagmo::population new_pop = algo.evolve(pop);

            output.send(MsgType::WORK_RESULTS, work_container{algo, new_pop});

            std::cout << "Worker thread finished. " << std::endl;
        });
}


unsigned distributed_worker::_compute_optimal_island_count()
{
    unsigned islandCount = std::thread::hardware_concurrency();
    if (islandCount == 0)
    {
        std::cout << "Defaulting to 8 islands (Cannot detect core count)" << std::endl;
        islandCount = 8;
    }
    else
    {
        std::cout << "Using " << islandCount << " islands" << std::endl;
    }
    return islandCount;
}

std::tuple<std::vector<pagmo::vector_double>, std::vector<pagmo::vector_double>> distributed_worker::_merge_populations(
    pagmo::archipelago archi)
{
    std::vector<pagmo::vector_double> allPopulations{};
    std::vector<pagmo::vector_double> allFitnesses{};
    for (const auto& isl : archi)
    {
        auto islPop = isl.get_population().get_x();
        auto islFit = isl.get_population().get_f();
        allPopulations.insert(allPopulations.end(), islPop.begin(), islPop.end());
        allFitnesses.insert(allFitnesses.end(), islFit.begin(), islFit.end());
    }

    return {allPopulations, allFitnesses};
}


void distributed_worker::_archipelago_based_worker(pagmo::algorithm& algo,
                                                   pagmo::population& pop,
                                                   const std::function<std::vector<pagmo::pop_size_t>
                                                       (const std::vector<pagmo::vector_double>&,
                                                        std::size_t)>& popSorter)
{
    _workerThread = std::thread(
        [this, algo, pop, popSorter]()
        {
            // 1) Set up socket for communicating with the parent thread
            distributed::dealer_socket output{this->_ctx};
            output.set_routing_id("worker_main");
            output.connect("ipc://thread_socket");

            // 2) Get island count based on hardware core count
            const unsigned islandCount = _compute_optimal_island_count();

            // 3) Construct and initialize islands with our algorithm and population (pop includes problem)
            pagmo::archipelago archi{}; // TODO: Set archi topology? Maybe divide pop size by coreCount?
            for (int i = 0; i < islandCount; ++i)
            {
                // Uses the EXISTING population as sent from controller, instead of creating a new one
                // UDI will be either thread_island or fork_island (chosen internally)
                archi.push_back(pagmo::island{algo, pop});
            }

            // 4) Run evolution on all islands in parallel
            std::cout << "Using algorithm: " << algo.get_name() << std::endl;
            archi.evolve(_archipelagoEvolutionCount);
            archi.wait_check();

            // 5) Merge individuals (and their fitness) from all islands into two vectors
            const auto [allPopulations, allFitnesses] = _merge_populations(archi);
            std::cout << "Size of allPopulations: " << allPopulations.size() << std::endl;

            // 6) Sort and select POPULATION_SIZE best individuals
            const auto newIndividualsIndexes = popSorter(allFitnesses, pop.size());

            // 7) Construct the new population object, using the first island's state
            auto firstIslPop = archi[0].get_population();
            pagmo::population newPop{
                firstIslPop.get_problem(),
                newIndividualsIndexes.size(),
                firstIslPop.get_seed() // TODO: Maybe remove this so the seed is different each time?
            };

            // 8) Fill the newPop object with new individuals (best POPULATION_SIZE individuals)
            for (std::size_t i = 0; i < newIndividualsIndexes.size(); ++i)
            {
                const auto individualIndex = newIndividualsIndexes[i];
                newPop.set_x(i, allPopulations[individualIndex]);
            }

            // 9) Send the algorithm (taken from the first island) and new population back to controller
            output.send(MsgType::WORK_RESULTS, work_container{archi[0].get_algorithm(), newPop});

            std::cout << "Worker thread finished. " << std::endl;
        });
}

void distributed_worker::_archipelago_based_worker_singleobjective(pagmo::algorithm& algo, pagmo::population& pop)
{
    std::cout << "Archipelago-based worker (singe-objective) started... " << std::endl;

    const auto prob = pop.get_problem();
    const auto singleObjectiveSort = [prob](const std::vector<pagmo::vector_double>& fitness, const std::size_t N)
    {
        if (prob.get_nc() == 0)
        {
            // Non-constrained version
            std::vector<pagmo::pop_size_t> idx(fitness.size());
            std::iota(idx.begin(), idx.end(), static_cast<pagmo::pop_size_t>(0));
            std::sort(idx.begin(), idx.end(), [&fitness](auto a, auto b) { return fitness[a][0] < fitness[b][0]; });
            idx.resize(N);
            return idx;
        }
        // Constrained version
        auto idx = pagmo::sort_population_con(fitness, prob.get_nec(), prob.get_c_tol());
        idx.resize(N);
        return idx;
    };

    _archipelago_based_worker(algo, pop, singleObjectiveSort);
}

void distributed_worker::_archipelago_based_worker_multiobjective(pagmo::algorithm& algo, pagmo::population& pop)
{
    std::cout << "Archipelago-based worker (multi-objective) started... " << std::endl;

    const auto multiObjectiveSort = [](const std::vector<pagmo::vector_double>& fitness, std::size_t N)
    {
        return pagmo::select_best_N_mo(fitness, N);
    };

    _archipelago_based_worker(algo, pop, multiObjectiveSort);
}

void distributed_worker::_start_worker_thread(pagmo::algorithm& algo, pagmo::population& pop)
{
    const bool isMultiObjective = pop.get_problem().get_nobj() > 1;

    if (_workerMode == ARCHIPELAGO_BASED && isMultiObjective)
    {
        _archipelago_based_worker_multiobjective(algo, pop);
    }
    else if (_workerMode == ARCHIPELAGO_BASED && !isMultiObjective)
    {
        _archipelago_based_worker_singleobjective(algo, pop);
    }
    else
    {
        _single_threaded_worker(algo, pop);
    }
}

//#####################################################################################
//# Sockets setup & initialization
//#####################################################################################

distributed_worker::distributed_worker(const std::string& controllerAddress, const worker_mode workerMode,
                                       const unsigned archipelagoEvolutionCount) :
    _workerSocket(_ctx),
    _threadSocket(_ctx),
    _workerMode(workerMode),
    _archipelagoEvolutionCount(archipelagoEvolutionCount)
{
    // Poller callback - worker socket has message
    _poller.add(_workerSocket.get_socket(), zmq::event_flags::pollin,
                [this](zmq::event_flags e)
                {
                    _handle_Worker_Socket_Msg();
                });

    // Poller callback - thread socket has message
    _poller.add(_threadSocket.get_socket(), zmq::event_flags::pollin,
                [this](zmq::event_flags e)
                {
                    _handle_Thread_Socket_Msg();
                });

    // Thread socket always listens on this address
    _threadSocket.bind("ipc://thread_socket");

    // Configure the worker socket so it can communicate with the controller
    _workerId = "worker_" + uuid::v4::UUID::New().String();
    _workerSocket.set_routing_id(_workerId);
    _workerSocket.connect(controllerAddress);
}

void distributed_worker::client_loop()
{
    if (_firstRun)
    {
        // Send out initial message to controller
        _workerSocket.send(MsgType::WORKER_JOIN);
        _firstRun = false;
    }

    // -1 means no poller timeout
    _poller.wait(std::chrono::milliseconds{-1});
}

void distributed_worker::run_client()
{
    _clientThread = std::thread([this]
    {
        // Send out initial message to controller
        _workerSocket.send(MsgType::WORKER_JOIN);
        _firstRun = false;

        for (;;)
        {
            _poller.wait(std::chrono::milliseconds{-1});
        }
    });
}

distributed_worker::~distributed_worker()
{
    if (_clientThread.joinable())
    {
        _clientThread.join();
    }
}

std::optional<std::vector<std::byte>> distributed_worker::get_dll_from_controller(const std::string& lib_name)
{
    const auto myId = "worker_dll_handler";

    distributed::dealer_socket socket{this->_ctx};
    socket.set_routing_id(myId);
    socket.connect("ipc://thread_socket");

    // 1) Send the request
    socket.send(MsgType::GET_DLL, get_dll_request{lib_name, myId});

    // 2) Block until controller eventually replies with DLL_BINARY
    std::tuple<MsgType, std::vector<std::byte>> receivedData{};
    do
    {
        receivedData = socket.receive();
        // TODO: Possibly handle any other Msg Types?
    }
    while (std::get<0>(receivedData) != MsgType::DLL_BINARY);

    // 3) Return the file itself
    auto dbc = vector_deserialize<dll_binary_container>(std::get<1>(receivedData));

    return dbc.dll_file;
}
