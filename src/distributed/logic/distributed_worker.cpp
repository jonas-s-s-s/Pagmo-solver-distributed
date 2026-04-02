#include "distributed_worker.h"

#include "aixlog.hpp"

#include <iostream>

#include "population_tools.h"
#include "UUID.h"
#include "vector_deserialize.h"
#include "pagmo/archipelago.hpp"
#include "pagmo/islands/thread_island.hpp"
#include "pagmo/utils/multi_objective.hpp"

//#####################################################################################
//# Handling of socket messages
//#####################################################################################

void distributed_worker::_handle_Worker_Socket_Msg()
{
    auto [type, binary] = _workerSocket.receive();

    LOG(TRACE) << "[" << static_cast<int>(type) << "] from controller" << std::endl;

    switch (type)
    {
    case MsgType::ALLOCATE_WORK:
        {
            // Do not deserialize here, because it can possibly create an instance of udp_dll_wrapper,
            // which blocks the thread by calling socket.receive in get_dll_from_controller().
            _start_worker_thread(binary);
        }
        break;
    case MsgType::DLL_BINARY:
        // Controller has responded to Worker thread's GET_DLL message.
        // This means we need to forward this to the worker thread via the thread socket
        _threadSocket.send("worker_dll_handler", MsgType::DLL_BINARY, binary);
        break;

    default:
        LOG(WARNING) << "WARNING: controller sent unhandled message type: " << static_cast<int>(type) << std::endl;
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
        LOG(WARNING) << "WARNING: worker thread socket sent unhandled message type: " << static_cast<int>(type) <<
            std::endl;
    }
}

//#####################################################################################
//# Worker logic
//#####################################################################################

void distributed_worker::_single_threaded_worker(pagmo::algorithm& algo, pagmo::population& pop)
{
    LOG(TRACE) << "Single-threaded worker started... " << std::endl;

    distributed::dealer_socket output{this->_ctx};
    output.set_routing_id("worker_main");
    output.connect("inproc://thread_socket");

    LOG(TRACE) << "Running algorithm: " << algo.get_name() << std::endl;
    const pagmo::population new_pop = algo.evolve(pop);

    output.send(MsgType::WORK_RESULTS, work_container{algo, new_pop});
}


unsigned distributed_worker::_compute_optimal_island_count()
{
    unsigned islandCount = std::thread::hardware_concurrency();
    if (islandCount == 0)
    {
        LOG(TRACE) << "Defaulting to 8 islands (Cannot detect core count)" << std::endl;
        islandCount = 8;
    }
    else
    {
        LOG(TRACE) << "Using " << islandCount << " islands" << std::endl;
    }
    return islandCount;
}

void distributed_worker::_archipelago_based_worker(pagmo::algorithm& algo, pagmo::population& pop)
{
    LOG(TRACE) << "Archipelago-based worker started... " << std::endl;

    // 1) Set up socket for communicating with the parent thread
    distributed::dealer_socket output{this->_ctx};
    output.set_routing_id("worker_main");
    output.connect("inproc://thread_socket");

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
    LOG(TRACE) << "Using algorithm: " << algo.get_name() << std::endl;
    archi.evolve(_archipelagoEvolutionCount);
    archi.wait_check();

    // 5) Merge individuals (and their fitness) from all islands into two vectors
    const auto [allPopulations, allFitnesses] = merge_populations(archi);
    LOG(TRACE) << "Size of allPopulations: " << allPopulations.size() << std::endl;

    // 6) Build a new population containing only the best POPULATION_SIZE individuals
    auto firstIslPop = archi[0].get_population();
    const auto newPop = select_best_N_into_new_population(
        firstIslPop.get_problem(),
        allPopulations,
        allFitnesses,
        pop.size(),
        firstIslPop.get_seed() // TODO: Maybe remove this so the seed is different each time?
    );

    // 7) Send the algorithm (taken from the first island) and new population back to controller
    output.send(MsgType::WORK_RESULTS, work_container{archi[0].get_algorithm(), newPop});
}

void distributed_worker::_start_worker_thread(const std::vector<std::byte>& workData)
{
    // We start the new thread right here, any further blocking calls won't affect the main thread
    _workerThread = std::thread(
        [this, workData]()
        {
            auto wct = vector_deserialize<work_container>(workData);

            if (_workerMode == ARCHIPELAGO_BASED)
            {
                _archipelago_based_worker(wct.algo, wct.pop);
            }
            else
            {
                _single_threaded_worker(wct.algo, wct.pop);
            }

            LOG(TRACE) << "Worker thread finished. " << std::endl;
        });
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
    _threadSocket.bind("inproc://thread_socket");

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

        try
        {
            for (;;)
            {
                _poller.wait(std::chrono::milliseconds{-1});
            }
        } catch (...)
        {
            // poller throws an exception if it's interrupted, see destructor, this way we can shut down the thread
            return;
        }
    });
}

distributed_worker::~distributed_worker()
{
    if (_clientThread.joinable())
    {
        _workerSocket.get_socket().close();
        _threadSocket.get_socket().close();
        _ctx.shutdown();

        _clientThread.join();
    }
}

//#####################################################################################
//# Other public member functions
//#####################################################################################

std::optional<std::vector<std::byte>> distributed_worker::get_dll_from_controller(const std::string& lib_name)
{
    const auto myId = "worker_dll_handler";

    distributed::dealer_socket socket{this->_ctx};
    socket.set_routing_id(myId);
    socket.connect("inproc://thread_socket");

    // 1) Send the request
    socket.send(MsgType::GET_DLL, get_dll_request{lib_name});

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
