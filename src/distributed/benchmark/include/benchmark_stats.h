#pragma once
#include "population_tools.h"
#include "pagmo/problem.hpp"
#include "pagmo/utils/hypervolume.hpp"
#include <chrono>
#include <filesystem>

/**
 * Class for collecting data about benchmarking
 * Usage is simple, first call start_timer(), then perform your calculation, call stop_timer() and then finally add_data_point().
 */
class benchmark_stats
{
    pagmo::problem _problem;

    std::vector<pagmo::vector_double> _dataPoints;
    std::vector<std::string> _pointIDs;
    std::vector<double> _elapsedTimes;

    std::chrono::high_resolution_clock::time_point _timerStart;

    // [csvString, sectionName]
    std::vector<std::tuple<std::string, std::string>> _csvStatsBuffer{};

    /**
     * Primary function providing a report of the benchmark,
     * computes statistics for each collected data point,
     * resulting statistics are returned as a CVS table.
     * @return CSV table
     */
    std::string _get_current_stats_csv();

    /**
     * Used to write the resulting HTML or CSV file to disk
     */
    static void _write_results_to_file(const std::filesystem::path& path, const std::string& content, bool append = false,
                                       bool binary = true);

public:
    /**
     * Constructor accepts the problem used for calculating stats in _get_current_stats_csv()
     */
    explicit benchmark_stats(pagmo::problem problem = {});

    void set_problem(const pagmo::problem& problem);

    /**
     * Stores a timestamp of when we began measuring a data point entry
     */
    void start_timer();

    /**
     * Returns the elapsed time since start_timer() was called
     */
    double stop_timer() const;

    /**
     * Primary function for adding data points to the benchmark,
     * this is intended to store the result of a singe evolve() call.
     */
    void add_data_point(pagmo::vector_double result, std::string pointID, double elapsedTime);

    /**
     * Finalizes the current batch of measurements, stores the results as CSV in _csvStatsBuffer.
     * @param measurementName
     */
    void end_current_measurement(const std::string& measurementName = "");

    /**
     * Clears the data points and time data
     */
    void clear_current_measurement();

    /**
     * Removes all stored CSV results of all measurement batches
     */
    void clear_all_results();

    /**
     * Returns all stored measurement batches as a vector of CSV files
     */
    std::vector<std::string> get_all_results_as_csv();

    /**
     * Returns a vector of tuples in the format: [csvContent, fileName],
     * so the same as get_all_results_as_csv() except with names of each batch.
     */
    std::vector<std::tuple<std::string,std::string>> get_all_results_as_csv_with_names();

    /**
     * Converts all stored benchmark CSV results into a single HTML report.
     */
    std::string get_all_results_as_html();

    void save_all_results_as_csv(const std::filesystem::path& filePath = "./benchmark_results_csv/", bool fileNameWithDate = true);

    void save_all_results_as_html(const std::filesystem::path& filePath = "./benchmark_results_html/", bool fileNameWithDate = true);
};
