#include "distributed_worker.h"

#include <iostream>
#include <random>

#include "UUID.h"
#include "vector_istreambuf.h"
#include "pagmo/archipelago.hpp"
#include "pagmo/utils/multi_objective.hpp"

//#####################################################################################
//# Handling of socket messages
//#####################################################################################

void distributed_worker::_handleWorkerSocketMsg()
{
    auto [type, binary] = _workerSocket.receive();

    std::cout << "[" << static_cast<int>(type) << "] from controller" << std::endl;

    switch (type)
    {
    case MsgType::ALLOCATE_WORK:
        {
            vector_istreambuf ibuf(binary);
            std::istream is(&ibuf);
            boost::archive::binary_iarchive ia(is);

            // Deserialize received data
            work_container work_input{};
            ia >> work_input;

            // TODO: Handle situation if thread is already running
            _start_worker_thread(work_input.algo, work_input.pop);
        }
        break;
    default:
        std::cerr << "WARNING: controller sent unhandled message type: " << static_cast<int>(type) << std::endl;
    }
}

void distributed_worker::_handleThreadSocketMsg()
{
    auto [type, work_output] = _threadSocket.receive();

    _workerThread.join();

    _workerSocket.send(MsgType::WORK_RESULTS, work_output);
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
            distributed::pair_socket output{this->_ctx};
            output.connect("inproc://thread_socket");

            std::cout << "Running algorithm: " << algo.get_name() << std::endl;
            const pagmo::population new_pop = algo.evolve(pop);

            output.send(MsgType::WORK_RESULTS, work_container{algo, new_pop});

            std::cout << "Worker thread finished. " << std::endl;
        });
}

void distributed_worker::_archipelago_based_worker(pagmo::algorithm& algo,
                                                   pagmo::population& pop,
                                                   std::function<std::vector<pagmo::pop_size_t>
                                                       (const std::vector<pagmo::vector_double>&,
                                                        std::size_t)> popSorter)
{
    _workerThread = std::thread(
        [this, algo, pop, popSorter]()
        {
            distributed::pair_socket output{this->_ctx};
            output.connect("inproc://thread_socket");

            unsigned coreCount = std::thread::hardware_concurrency();
            if (coreCount == 0)
            {
                std::cout << "Defaulting to 8 islands (Cannot detect core count)" << std::endl;
                coreCount = 8;
            }
            else
            {
                std::cout << "Using " << coreCount << " islands" << std::endl;
            }
            std::cout << "Using algorithm: " << algo.get_name() << std::endl;

            // 1) Run the evolution on multiple parallel islands and wait for it to finish
            // TODO: Set topology?
            // TODO: Maybe divide pop size by coreCount?
            pagmo::archipelago archi{coreCount, algo, pop.get_problem(), pop.size()};
            archi.evolve(_archipelagoEvolutionCount);
            archi.wait_check();

            // 2) Collect individuals of all island's populations into this vector, together with their fitness
            std::vector<pagmo::vector_double> allPopulations{};
            std::vector<pagmo::vector_double> allFitnesses{};
            for (const auto& isl : archi)
            {
                auto islPop = isl.get_population().get_x();
                auto islFit = isl.get_population().get_f();
                allPopulations.insert(allPopulations.end(), islPop.begin(), islPop.end());
                allFitnesses.insert(allFitnesses.end(), islFit.begin(), islFit.end());
            }
            std::cout << "Size of allPopulations: " << allPopulations.size() << std::endl;

            // 3) Sort and select POPULATION_SIZE best individuals
            const auto newIndividualsIndexes = popSorter(allFitnesses, pop.size());

            // 4) Construct the new population object, using the first island's state
            auto firstIslPop = archi[0].get_population();
            pagmo::population newPop{
                firstIslPop.get_problem(),
                newIndividualsIndexes.size(),
                firstIslPop.get_seed() // TODO: Maybe remove this so the seed is different each time?
            };

            // 5) Fill the newPop object with new individuals
            for (std::size_t i = 0; i < newIndividualsIndexes.size(); ++i)
            {
                const auto individualIndex = newIndividualsIndexes[i];
                newPop.set_x(i, allPopulations[individualIndex]);
            }

            // 6) Send the algorithm (taken from the first island) and new population back to controller
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
                    _handleWorkerSocketMsg();
                });

    // Poller callback - thread socket has message
    _poller.add(_threadSocket.get_socket(), zmq::event_flags::pollin,
                [this](zmq::event_flags e)
                {
                    _handleThreadSocketMsg();
                });

    // Thread socket always listens on this address
    _threadSocket.bind("inproc://thread_socket");

    // Configure the worker socket so it can communicate with the controller
    _workerId = "worker_" + uuid::v4::UUID::New().String();
    _workerSocket.set_routing_id(_workerId);
    _workerSocket.connect(controllerAddress);

    // Send out initial message to controller
    _workerSocket.send(MsgType::WORKER_JOIN);
}

void distributed_worker::client_loop()
{
    // -1 means no poller timeout
    _poller.wait(std::chrono::milliseconds{-1});
}

void distributed_worker::run_client()
{
    _clientThread = std::thread([this]
    {
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
