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

    if (!_workerRecords.contains(workerID))
    {
        throw std::runtime_error(
            "Cannot perform worker_started_work(), worker " + workerID + " is not in _workerRecords.");
    }

    _workerRecords.at(workerID).lastWorkStartTime = std::chrono::high_resolution_clock::now();
}

void worker_info_repository::worker_finished_work(const std::string& workerID, const size_t processedPopulation,
                                                  const std::string& algoName)
{
    std::scoped_lock lock(_mtx);
    LOG(TRACE) << "Worker finished work - info collected" << std::endl;

    if (!_workerRecords.contains(workerID))
    {
        throw std::runtime_error(
            "Cannot perform worker_finished_work(), worker " + workerID + " is not in _workerRecords.");
    }

    auto& wInfo = _workerRecords.at(workerID);
    const auto startTime = wInfo.lastWorkStartTime;
    const auto endTime = std::chrono::high_resolution_clock::now();
    const auto workTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    wInfo.totalStats.workTime += workTime;
    wInfo.totalStats.processedPopulation += processedPopulation;

    wInfo.statsByAlgorithm[algoName].workTime += workTime;
    wInfo.statsByAlgorithm[algoName].processedPopulation += processedPopulation;
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

const std::unordered_set<std::string>& worker_info_repository::get_connected_workers()
{
    std::scoped_lock lock(_mtx);

    return _connectedWorkers;
}

size_t worker_info_repository::get_worker_count()
{
    std::scoped_lock lock(_mtx);

    return _connectedWorkers.size();
}

void worker_info_repository::wait_until_worker_count(const size_t target)
{
    std::unique_lock lock(_mtx);
    LOG(TRACE) << "Waiting until " << target << " workers connect..." << std::endl;

    _cv.wait(lock, [&]
    {
        return _connectedWorkers.size() >= target;
    });
}
