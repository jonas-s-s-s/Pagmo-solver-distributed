#pragma once
#include <memory>
#include <string>
#include <spdlog/logger.h>

namespace glog
{
    spdlog::logger& get();

    std::string generate_log_filename(const std::string& basePath);

    void init_file_logger(const std::string& filePath = generate_log_filename("logs/distributed.log"), bool enableConsole = true);

    void disable();

    /**
     * Call this to properly shut down the logger, this should prevent exceptions in spdlog destructors
     * when using libc++ (macOS)
     */
    void shutdown();
}
