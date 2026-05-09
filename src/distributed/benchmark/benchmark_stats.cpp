#include "benchmark_stats.h"

#include <fstream>
#include <iomanip>
#include <sstream>

#include "benchmark_results_html.h"
#include "csv_conversion.h"
#include "path_normalizer.h"

std::string benchmark_stats::_get_current_stats_csv()
{
    std::string output;
    const size_t nObj = _problem.get_nobj();
    const bool isMultiObjective = nObj > 1;

    auto format_double = [](const double value)
    {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(CSV_DOUBLE_PRECISION) << value;
        return ss.str();
    };

    // 1) Calculate fitnesses of all data points
    std::vector<pagmo::vector_double> fitnesses;
    fitnesses.reserve(_dataPoints.size());
    for (const auto& point : _dataPoints)
    {
        fitnesses.emplace_back(_problem.fitness(point));
    }

    // 2) Get indexes of data points sorted from best to worst (works for both MO and SO)
    const std::vector<pagmo::pop_size_t> bestIndividualIndexes = get_best_N_individuals_indexes(
        _problem, fitnesses, _dataPoints.size()
    );

    // 3) Sort the data points, their IDs, fitnesses and elapsed times
    std::vector<pagmo::vector_double> sortedPoints;
    std::vector<pagmo::vector_double> sortedFitnesses;
    std::vector<std::string> sortedPointIDs;
    std::vector<double> sortedElapsedTimes;
    for (const size_t idx : bestIndividualIndexes)
    {
        sortedPoints.emplace_back(_dataPoints[idx]);
        sortedFitnesses.emplace_back(fitnesses[idx]);
        sortedPointIDs.emplace_back(_pointIDs[idx]);
        sortedElapsedTimes.emplace_back(_elapsedTimes[idx]);
    }

    // 4) Multi-objective calculations (ranks + hypervolume)
    std::vector<pagmo::pop_size_t> ranks;
    std::vector<pagmo::pop_size_t> domCounts;
    std::vector<double> crowdDists;
    std::vector<double> hvContributions;

    if (isMultiObjective)
    {
        // Calculate MO metrics
        if (sortedFitnesses.size() >= 2)
        {
            auto fnds = pagmo::fast_non_dominated_sorting(sortedFitnesses);
            domCounts = std::get<2>(fnds);
            ranks = std::get<3>(fnds);
            crowdDists = pagmo::crowding_distance(sortedFitnesses);

            // Hypervolume contributions
            pagmo::hypervolume hv(sortedFitnesses);
            auto ref = hv.refpoint(1.0);
            hvContributions = hv.contributions(ref);
        }
    }

    // 5) Build header
    output += "ID,Value,Fitness,Fitness increase over baseline,ElapsedTime";
    if (isMultiObjective)
    {
        output += ",Rank,DomBy,CrowdDist,HVContribution";
    }
    output += "\n";

    // 6) Build rows
    const auto bestIndividualFitness = sortedFitnesses[0];
    for (int i = 0; i < sortedPoints.size(); ++i)
    {
        const auto& point = sortedPoints[i];
        const auto& fitness = sortedFitnesses[i];
        const auto& pointID = sortedPointIDs[i];

        output += pointID + "," +
            double_vector_to_csv(point) + "," +
            double_vector_to_csv(fitness) + ",";

        // Calculate percentage difference within fitness vectors
        output += "\"";
        for (int j = 0; j < bestIndividualFitness.size(); ++j)
        {
            double percentageDecline = 0.0;
            if (bestIndividualFitness[j] != 0.0)
            {
                double difference = fitness[j] - bestIndividualFitness[j];
                percentageDecline = difference / bestIndividualFitness[j];
            }

            output += format_double(percentageDecline * 100) + "%";
            if (j + 1 < bestIndividualFitness.size())
            {
                output += ";";
            }
        }
        output += "\"";

        // Append elapsed time
        output += "," + format_double(sortedElapsedTimes[i]);

        // Append MO specific columns
        if (isMultiObjective && !ranks.empty())
        {
            output += "," + std::to_string(ranks[i]) +
                "," + std::to_string(domCounts[i]) +
                "," + (std::isinf(crowdDists[i]) ? "Inf" : format_double(crowdDists[i])) +
                "," + format_double(hvContributions[i]);
        }

        output += "\n";
    }

    return output;
}

void benchmark_stats::_write_results_to_file(const std::filesystem::path& path, const std::string& content,
                                              const bool append,
                                              const bool binary)
{
    // Make sure parent dir exists
    if (const auto parent = path.parent_path(); !parent.empty())
        std::filesystem::create_directories(parent);

    const std::ios::openmode mode = std::ios::out | (append ? std::ios::app : std::ios::trunc) | (
        binary ? std::ios::binary : std::ios::openmode{});

    std::ofstream out(win_normalize_filename(path), mode);
    if (!out)
        throw std::runtime_error("_write_results_to_file: failed opening file, path: " + path.string());

    out.write(content.data(), static_cast<std::streamsize>(content.size()));
    if (!out)
        throw std::runtime_error("_write_results_to_file: filed writing to file, path: " + path.string());
}

benchmark_stats::benchmark_stats(pagmo::problem problem) : _problem(std::move(problem))
{
}

void benchmark_stats::set_problem(const pagmo::problem& problem)
{
    _problem = problem;
}

void benchmark_stats::start_timer()
{
    _timerStart = std::chrono::high_resolution_clock::now();
}

double benchmark_stats::stop_timer() const
{
    const auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - _timerStart).count();
}

void benchmark_stats::add_data_point(pagmo::vector_double result, std::string pointID, double elapsedTime)
{
    _dataPoints.emplace_back(std::move(result));
    _pointIDs.emplace_back(std::move(pointID));
    _elapsedTimes.emplace_back(elapsedTime);
}

void benchmark_stats::end_current_measurement(const std::string& measurementName)
{
    std::string name = measurementName.empty()
                           ? _problem.get_name()
                           : measurementName + " (" + _problem.get_name() + ")";

    _csvStatsBuffer.emplace_back(_get_current_stats_csv(), name);
    clear_current_measurement();
}

void benchmark_stats::clear_current_measurement()
{
    _dataPoints.clear();
    _pointIDs.clear();
    _elapsedTimes.clear();
}

void benchmark_stats::clear_all_results()
{
    _csvStatsBuffer.clear();
}

std::vector<std::string> benchmark_stats::get_all_results_as_csv()
{
    std::vector<std::string> output;
    for (const auto& [csv, name] : _csvStatsBuffer)
    {
        output.emplace_back(csv);
    }
    return output;
}

std::vector<std::tuple<std::string, std::string>> benchmark_stats::get_all_results_as_csv_with_names()
{
    return _csvStatsBuffer;
}

std::string benchmark_stats::get_all_results_as_html()
{
    benchmark_results_html html{};
    for (const auto& [csvString, sectionName] : _csvStatsBuffer)
    {
        html.add_section(csv_to_html(csvString), sectionName);
    }

    return html.get_html_page();
}

void benchmark_stats::save_all_results_as_csv(const std::filesystem::path& filePath, const bool fileNameWithDate)
{
    const auto now = std::chrono::system_clock::now();
    std::string currentDate{};
    if (fileNameWithDate)
        currentDate = std::format("{:%Y-%m-%d %H:%M:%S}", now);

    for (const auto& [csvContent, fileName] : get_all_results_as_csv_with_names())
    {
        _write_results_to_file(filePath / std::string(fileName + " " + currentDate + ".csv"), csvContent);
    }
}

void benchmark_stats::save_all_results_as_html(const std::filesystem::path& filePath, const bool fileNameWithDate)
{
    const auto now = std::chrono::system_clock::now();
    std::string currentDate{};
    if (fileNameWithDate)
        currentDate = std::format("{:%Y-%m-%d %H:%M:%S}", now);

    _write_results_to_file(filePath / std::string("solver_benchmark_" + currentDate + ".html"),
                           get_all_results_as_html());
}