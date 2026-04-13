#include "global_logger.h"
#include <vector>
#include <mutex>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/null_sink.h>

namespace glog
{
    static std::mutex g_mutex;
    static std::shared_ptr<spdlog::logger> g_logger;

    std::shared_ptr<spdlog::logger> get()
    {
        std::lock_guard lock(g_mutex);
        if (!g_logger)
        {
            auto sink = std::make_shared<spdlog::sinks::null_sink_mt>();
            g_logger = std::make_shared<spdlog::logger>("null_logger", sink);
            g_logger->set_level(spdlog::level::off);
        }

        return g_logger;
    }

    void init_file_logger(const std::string& filePath, const bool enableConsole)
    {
        std::lock_guard lock(g_mutex);

        std::vector<spdlog::sink_ptr> sinks;

        if (enableConsole)
        {
            const auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_pattern("[%H:%M:%S] [%^%l%$] %v");
            sinks.push_back(console_sink);
        }

        const auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filePath, true);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");
        sinks.push_back(file_sink);

        g_logger = std::make_shared<spdlog::logger>("global_logger", sinks.begin(), sinks.end());
        g_logger->set_level(spdlog::level::trace);
        g_logger->flush_on(spdlog::level::warn);

        spdlog::set_default_logger(g_logger);
    }

    void disable()
    {
        std::lock_guard lock(g_mutex);

        auto sink = std::make_shared<spdlog::sinks::null_sink_mt>();
        g_logger = std::make_shared<spdlog::logger>("null_logger", sink);
        g_logger->set_level(spdlog::level::off);

        spdlog::set_default_logger(g_logger);
    }
}
