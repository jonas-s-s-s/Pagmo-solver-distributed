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
    std::cout << "Worker thread started. " << std::endl;
    _workerThread = std::thread(
        [this]()
        {
            // TODO
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));

            distributed::pair_socket output{this->_ctx};
            output.connect("inproc://thread_socket");
            // TODO
            output.send(MsgType::WORK_RESULTS);

            std::cout << "Worker thread finished. " << std::endl;
        });
}

void distributed_worker::_handleWorkerSocketMsg()
{
    auto [type, binary] = _workerSocket.receive();

    std::cout << "[" << static_cast<int>(type) << "] from controller" << std::endl;

    switch (type)
    {
    case MsgType::ALLOCATE_WORK:
        {
            // TODO: Handle situation if thread is already running
            _start_worker_thread();
        }
        break;
    default:
        std::cerr << "WARNING: controller sent unhandled message type: " << static_cast<int>(type) << std::endl;
    }
}


void distributed_worker::_handleThreadSocketMsg()
{
    auto [type, binary] = _threadSocket.receive();

    _workerThread.join();

    _workerSocket.send(MsgType::WORK_RESULTS, binary);
}


void distributed_worker::client_loop()
{
    // -1 means no poller timeout
    _poller.wait(std::chrono::milliseconds{-1});
}
