#pragma once
#include <memory>
#include <string>
#include <spdlog/logger.h>

namespace glog
{
    std::shared_ptr<spdlog::logger> get();

    void init_file_logger(const std::string& filePath = "logs/distributed.log", bool enableConsole = true);

    void disable();
}
