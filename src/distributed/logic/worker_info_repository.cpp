#include "worker_info_repository.h"

void worker_info_repository::add_worker_record(const std::string& workerID, worker_info info)
{
    std::scoped_lock lock(_mapMutex);

    _workerRecords.insert({workerID, info});
}

void worker_info_repository::remove_worker_record(const std::string& workerID)
{
    std::scoped_lock lock(_mapMutex);

    _workerRecords.erase(workerID);
}

std::optional<worker_info_repository::worker_info> worker_info_repository::get_worker_record(
    const std::string& workerID)
{
    std::scoped_lock lock(_mapMutex);

    const auto record = _workerRecords.find(workerID);
    if (record != _workerRecords.end())
    {
        return record->second;
    }

    // Record for this id was not found
    return std::nullopt;
}

size_t worker_info_repository::get_worker_count()
{
    std::scoped_lock lock(_mapMutex);

    return _workerRecords.size();
}
