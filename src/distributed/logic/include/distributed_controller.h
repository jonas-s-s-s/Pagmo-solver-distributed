#pragma once
#include <unordered_set>

#include "msgpack23.h"
#include "MsgType.h"
#include "zmq.hpp"

class distributed_controller
{
    zmq::context_t _ctx;
    zmq::socket_t _controllerSocket;

    std::unordered_set<std::string> _freeWorkersPool{};
    std::unordered_set<std::string> _busyWorkersPool{};

    /**
     * Receives a message from one of the workers
     * @return Tuple of: {workerId, msgType, payload}
     */
    std::tuple<std::string, MsgType, std::vector<std::byte>> _receive();

    /**
     * Sends a message to a worker, automatically serializes the payload
     * @tparam Packable Object that can be serialized
     * @param workerId Worker's ID, as obtained by the _receive() function
     * @param type Type of the message
     * @param payload Serializable payload of this message
     */
    template <typename Packable>
    void _send(const std::string& workerId, MsgType type, const Packable& payload);

    /**
     * For sending messages with no payload
     * Sends empty string payload, doesn't matter that it cannot be deserialized, as no deserialization is done
     */
    void _send(const std::string& workerId, MsgType type);

public:
    explicit distributed_controller(const std::string& controllerAddress);

    void server_loop();
};

template <typename Packable>
void distributed_controller::_send(const std::string& workerId, MsgType type, const Packable& payload)
{
    std::vector<std::byte> serialized;
    msgpack23::pack(std::back_inserter(serialized), payload);

    const int typeInt = static_cast<int>(type);
    _controllerSocket.send(zmq::buffer(workerId), zmq::send_flags::sndmore);
    _controllerSocket.send(zmq::buffer(&typeInt, sizeof(typeInt)), zmq::send_flags::sndmore);
    _controllerSocket.send(zmq::buffer(serialized), zmq::send_flags::none);
}
