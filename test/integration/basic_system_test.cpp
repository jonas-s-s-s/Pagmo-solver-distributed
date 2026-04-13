#include <catch2/catch_test_macros.hpp>
#include "distributed_solver.h"
#include "distributed_worker.h"
#include <thread>
#include <pagmo/problems/rosenbrock.hpp>
#include <pagmo/algorithms/de.hpp>

TEST_CASE("Basic controller+worker integration", "[integration]") {
    // Use a unique port to avoid conflicts
    std::string address = "tcp://127.0.0.1:55557";
    const size_t expected_workers = 1;

    // Start controller (runs in background thread)
    distributed_solver solver(address, expected_workers, load_balancing_strategy::ALL_ISLANDS_EQUAL);
    // Give controller time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Start worker (runs in background thread)
    distributed_worker worker(address, worker_mode::SINGLE_THREADED, 10);
    worker.run_client(); // runs its own thread

    // Wait for worker to connect
    solver.wait_until_workers_connect(1);
    REQUIRE(solver.get_current_worker_count() == 1);

    // Run a tiny evolution
    pagmo::problem prob{pagmo::rosenbrock{2}};
    pagmo::algorithm algo{pagmo::de{10}};
    solver.evolve(prob, {algo}, 20, 1, 4);
    auto best = solver.wait_until_completion();

    // Basic sanity: best should be a 2‑D vector
    REQUIRE(best.size() == 2);
    // The best fitness should be near 0 for Rosenbrock
    auto best_fit = prob.fitness(best);
    REQUIRE(best_fit[0] < 2.0); // not expecting exact zero, but reasonably small

    // Cleanup (destructors will close sockets)
}
