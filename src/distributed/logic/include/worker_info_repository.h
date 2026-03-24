#pragma once
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

/**
 * - A class intended to provide thread-safe access to information about workers connected to the controller
 * - Controller modifies, other classes can read this object to get details about the currently connected workers
 * - This class should encapsulate all additional information about workers, beyond what is stored in the controller itself
 * - Primary objective should be to store static information only, i.e. we only need to modify this class when a worker joins or leaves
 */
class worker_info_repository
{
    // To protect the unordered_map
    std::mutex _mapMutex{};

    // Notifying when _workerRecords changes
    std::condition_variable _cv;

    /*
     * - Include any constant info about the worker, such as OS, hardware details, etc. in here
     * - For now reserved for future use
     */
    struct worker_info
    {
    };

    std::unordered_map<std::string, worker_info> _workerRecords;

public:

    void add_worker_record(const std::string& workerID, worker_info info);

    void remove_worker_record(const std::string& workerID);

    std::optional<worker_info> get_worker_record(const std::string& workerID);

    size_t get_worker_count();

    void wait_until_worker_count(size_t target);
};
