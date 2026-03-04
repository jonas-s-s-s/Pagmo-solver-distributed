#pragma once
#include <vector>

#include "pagmo/algorithm.hpp"
#include "pagmo/population.hpp"

enum class MsgType
{
    WORKER_JOIN,
    WORKER_LEAVE,
    ALLOCATE_WORK,
    WORK_RESULTS,
};

struct AllocateWork
{
    pagmo::algorithm algo;
    pagmo::population pop;

    template<typename T>
    void pack(T &packer) const {
        packer(algo, pop);
    }

    template<typename T>
    void unpack(T &unpacker) {
        unpacker(algo, pop);
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