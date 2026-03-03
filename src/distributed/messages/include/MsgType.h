#pragma once
#include <vector>

enum class MsgType
{
    WORKER_JOIN,
    WORKER_LEAVE,
    ALLOCATE_WORK,
    WORK_RESULTS,
};

struct AllocateWork
{
    std::vector<double> population;

    template<typename T>
    void pack(T &packer) const {
        packer(population);
    }

    template<typename T>
    void unpack(T &unpacker) {
        unpacker(population);
    }
};

struct WorkResults
{
    std::vector<double> population;

    template<typename T>
    void pack(T &packer) const {
        packer(population);
    }

    template<typename T>
    void unpack(T &unpacker) {
        unpacker(population);
    }
};