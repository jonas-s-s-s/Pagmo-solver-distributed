#pragma once
#include "population_tools.h"
#include "pagmo/problem.hpp"
#include "pagmo/utils/hypervolume.hpp"
#include <chrono>
#include <filesystem>

/**
 * Class for benchmarking various algorithms or solvers.
 * Usage is simple, first call start_timer(), then perform your calculation, call stop_timer() and then finally add_data_point().
 */
class solver_benchmark
{
    pagmo::problem _problem;

    std::vector<pagmo::vector_double> _dataPoints;
    std::vector<std::string> _pointIDs;
    std::vector<double> _elapsedTimes;

    std::chrono::high_resolution_clock::time_point _timerStart;

    // [csvString, sectionName]
    std::vector<std::tuple<std::string, std::string>> _csvStatsBuffer{};

    std::string _get_current_stats_csv();

    static void _write_results_to_file(const std::filesystem::path& path, const std::string& content, bool append = false,
                                       bool binary = true);

public:
    explicit solver_benchmark(pagmo::problem problem);

    void start_timer();

    double stop_timer() const;

    void add_data_point(pagmo::vector_double result, std::string pointID, double elapsedTime);

    void end_current_measurement();

    void clear_current_measurement();

    void clear_all_results();

    std::vector<std::string> get_all_results_as_csv();

    /**
     * Returns a vector of tuples in the format: [csvContent, fileName]
     */
    std::vector<std::tuple<std::string,std::string>> get_all_results_as_csv_with_names();

    std::string get_all_results_as_html();

    void save_all_results_as_csv(const std::filesystem::path& filePath = "./benchmark_results_csv/", bool fileNameWithDate = true);

    void save_all_results_as_html(const std::filesystem::path& filePath = "./benchmark_results_html/", bool fileNameWithDate = true);
};
