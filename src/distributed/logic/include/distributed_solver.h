#pragma once
#include "distributed_controller.h"
#include "pagmo/archipelago.hpp"

class distributed_solver
{
    distributed_controller _controller;
    size_t _expectedWorkerCount;

    pagmo::archipelago _archipelago{};

public:
    /**
     * - Constructs the distributed solver and its internal components
     * - The distributed_controller's server is started immediately on another thread, as this object is constructed
     *  - This means that workers can connect as soon as this object is constructed
     * @param controllerAddress URL on which the distributed controller will run (e.g. "tcp://localhost:5000")
     * @param expectedWorkerCount the expected amount of workers, the current amount connected to controller will be used if not specified
     */
    explicit distributed_solver(const std::string& controllerAddress, const size_t expectedWorkerCount = 1);

    void evolve(const pagmo::problem& problem,
                const std::vector<pagmo::algorithm>& algorithms,
                size_t populationSize,
                size_t cycleCount = 1
    );

    pagmo::vector_double wait_until_completion();

    void wait_until_workers_connect(size_t workerCount);

    void wait_until_workers_connect();
};
