#include "distributed_controller.h"

#include <iostream>

std::tuple<std::string, MsgType, std::vector<std::byte>> distributed_controller::_receive()
{
    zmq::message_t identity;
    zmq::message_t typeMsg;
    zmq::message_t payloadMsg;

    // No-discard
    const auto r0 = _controllerSocket.recv(identity);
    const auto r1 = _controllerSocket.recv(typeMsg);
    const auto r2 = _controllerSocket.recv(payloadMsg);

    // Identification of the message
    int type;
    std::memcpy(&type, typeMsg.data(), sizeof(type));

    // The serialized message object
    std::vector payloadBinary(
        static_cast<std::byte*>(payloadMsg.data()),
        static_cast<std::byte*>(payloadMsg.data()) + payloadMsg.size()
    );

    return {identity.to_string(), static_cast<MsgType>(type), payloadBinary};
}

void distributed_controller::_send(const std::string& workerId, MsgType type)
{
    const int typeInt = static_cast<int>(type);
    _controllerSocket.send(zmq::buffer(workerId), zmq::send_flags::sndmore);
    _controllerSocket.send(zmq::buffer(&typeInt, sizeof(typeInt)), zmq::send_flags::sndmore);
    _controllerSocket.send(zmq::buffer(""), zmq::send_flags::none);
}

distributed_controller::distributed_controller(const std::string& controllerAddress) : _controllerSocket{
    _ctx, zmq::socket_type::router
}
{
    _controllerSocket.bind(controllerAddress);
}

void distributed_controller::server_loop()
{
    auto [workerId, type, binary] = _receive();

    // Incoming message
    std::cout << "[" << static_cast<int>(type) << "] from: " << workerId << std::endl;

    // Determine controller's action depending on the message type
    switch (type)
    {
    case MsgType::WORKER_JOIN:
        //const auto binRequest = msgpack23::unpack<BinariesRequest>(binary);
        _freeWorkersPool.emplace(workerId);
        std::cout << "Worker " << workerId << " joined" << std::endl;
        std::cout << "Free workers: " << _freeWorkersPool.size() << std::endl;
        break;
    case MsgType::WORKER_LEAVE:
        _freeWorkersPool.erase(workerId);
        // TODO: Handle busy worker leave
        _busyWorkersPool.erase(workerId);
        break;
    default:
        std::cerr << "WARNING: " << workerId << " sent unhandled message type: " << static_cast<int>(type) << std::endl;
    }
}
