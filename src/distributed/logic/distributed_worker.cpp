#include "distributed_worker.h"

#include <random>

std::string distributed_worker::_generate_worker_id()
{
    static std::random_device dev;
    static std::mt19937 rng(dev());
    std::uniform_int_distribution<int> dist(0, 15);
    const char* v = "0123456789abcdef";
    const bool dash[] = {0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0};
    std::string res;
    for (int i = 0; i < 16; i++)
    {
        if (dash[i]) res += "-";
        res += v[dist(rng)];
        res += v[dist(rng)];
    }

    return "worker_" + res;
}

std::tuple<MsgType, std::vector<std::byte>> distributed_worker::_receive()
{
    zmq::message_t typeMsg;
    zmq::message_t payloadMsg;

    // No-discard
    const auto r1 = _workerSocket.recv(typeMsg);
    const auto r2 = _workerSocket.recv(payloadMsg);

    // Identification of the message
    int type;
    std::memcpy(&type, typeMsg.data(), sizeof(type));

    // The serialized message object
    std::vector payloadBinary(
        static_cast<std::byte*>(payloadMsg.data()),
        static_cast<std::byte*>(payloadMsg.data()) + payloadMsg.size()
    );

    return {static_cast<MsgType>(type), payloadBinary};
}

void distributed_worker::_send(MsgType type, const std::vector<std::byte>& payload)
{
    const int typeInt = static_cast<int>(type);
    _workerSocket.send(zmq::buffer(&typeInt, sizeof(typeInt)), zmq::send_flags::sndmore);
    _workerSocket.send(zmq::buffer(payload), zmq::send_flags::none);
}

void distributed_worker::_send(MsgType type)
{
    const int typeInt = static_cast<int>(type);
    _workerSocket.send(zmq::buffer(&typeInt, sizeof(typeInt)), zmq::send_flags::sndmore);
    _workerSocket.send(zmq::buffer(""), zmq::send_flags::none);
}


distributed_worker::distributed_worker(const std::string& controllerAddress) : _workerSocket(
    _ctx, zmq::socket_type::dealer)
{
    _workerId = _generate_worker_id();
    _workerSocket.set(zmq::sockopt::routing_id, _workerId);
    _workerSocket.connect(controllerAddress);

    // Send out initial message to controller
    _send(MsgType::WORK_REQUEST);
}

void distributed_worker::client_loop()
{
}
