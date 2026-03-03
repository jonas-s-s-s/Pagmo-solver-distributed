#include "distributed_worker.h"

#include <iostream>
#include <random>

#include "UUID.h"

distributed_worker::distributed_worker(const std::string& controllerAddress) :
    _workerSocket(_ctx),
    _threadSocket(_ctx)
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


void distributed_worker::_start_worker_thread()
{
    _workerThread = std::thread(
        [this]()
        {
            /*
            const auto result = _workerHandlers.doWork(solverID);
            std::vector<std::byte> serialized;
            msgpack23::pack(std::back_inserter(serialized), result);

            // Pass serialized results back to main thread (ZeroMQ "recommended" way)
            zmq::socket_t output(this->_ctx, zmq::socket_type::pair);
            output.connect("inproc://thread_socket");
            output.send(zmq::buffer(serialized), zmq::send_flags::none);
            */

            std::cout << "Worker thread finished. " << std::endl;
        });
}

void distributed_worker::_handleWorkerSocketMsg()
{
    auto [type, binary] = _workerSocket.receive();

    std::cout << "[" << static_cast<int>(type) << "] from controller" << std::endl;

    // switch (type)
    // {
    // case MsgType::FARMER_DATA:
    //     {
    //         //const auto data = msgpack23::unpack<FarmerData>(binary);
    //
    //         //_send(MsgType::BINARIES_REQUEST, BinariesRequest(data.solverID, data.problemGUID));
    //     }
    //     break;
    // case MsgType::FARMER_BINARIES:
    //     {
    //         //_start_worker_thread(binaries.solverID);
    //     }
    //     break;
    // default: ;
    // }
}


void distributed_worker::_handleThreadSocketMsg()
{
    zmq::message_t results;
    auto [type, binary] = _threadSocket.receive();

    _workerThread.join();

    // // Send back WorkerResults
    // _send(MsgType::WORKER_RESULTS, std::vector<std::byte>{
    //           static_cast<std::byte*>(results.data()),
    //           static_cast<std::byte*>(results.data()) + results.size()
    //       });
    //
    // _myState = WorkerState::IDLE;
    // // Ask farmer for more work
    // _send(MsgType::WORK_REQUEST);
}


void distributed_worker::client_loop()
{
    // -1 means no poller timeout
    _poller.wait(std::chrono::milliseconds{-1});
}
