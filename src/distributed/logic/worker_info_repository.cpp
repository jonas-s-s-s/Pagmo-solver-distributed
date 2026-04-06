#include "worker_info_repository.h"

void worker_info_repository::worker_joined(const std::string& workerID, worker_info info)
{
    {
        std::scoped_lock lock(_mtx);
        LOG(TRACE) << "Worker joined - info collected" << std::endl;

        if (!_workerRecords.contains(workerID))
        {
            _workerRecords.insert({workerID, info});
        }
        _connectedWorkers.insert(workerID);
    }
    _cv.notify_all();
}

void worker_info_repository::worker_left(const std::string& workerID)
{
    {
        std::scoped_lock lock(_mtx);
        LOG(TRACE) << "Worker left - info collected" << std::endl;

        _connectedWorkers.erase(workerID);
    }
    _cv.notify_all();
}

void worker_info_repository::worker_started_work(const std::string& workerID)
{
    std::scoped_lock lock(_mtx);
    LOG(TRACE) << "Worker started work - info collected" << std::endl;

    if (_workerRecords.contains(workerID))
    {
        _workerRecords.at(workerID).lastWorkStartTime = std::chrono::high_resolution_clock::now();
        return;
    }

    throw std::runtime_error(
        "Cannot perform worker_started_work(), worker " + workerID + " is not in _workerRecords.");
}

void worker_info_repository::worker_finished_work(const std::string& workerID, const size_t processedPopulation)
{
    std::scoped_lock lock(_mtx);
    LOG(TRACE) << "Worker finished work - info collected" << std::endl;

    if (_workerRecords.contains(workerID))
    {
        const auto start = _workerRecords.at(workerID).lastWorkStartTime;
        const auto end = std::chrono::high_resolution_clock::now();
        const auto workTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        _workerRecords.at(workerID).workTime += workTime;
        _workerRecords.at(workerID).processedPopulation += processedPopulation;

        return;
    }

    throw std::runtime_error(
        "Cannot perform worker_finished_work(), worker " + workerID + " is not in _workerRecords.");
}

std::optional<worker_info> worker_info_repository::get_worker_info(
    const std::string& workerID)
{
    std::scoped_lock lock(_mtx);
    LOG(TRACE) << "Worker info requested" << std::endl;

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
    std::scoped_lock lock(_mtx);

    return _connectedWorkers.size();
}

void worker_info_repository::wait_until_worker_count(const size_t target)
{
    std::unique_lock lock(_mtx);

    _cv.wait(lock, [&]
    {
        return _connectedWorkers.size() >= target;
    });
}
