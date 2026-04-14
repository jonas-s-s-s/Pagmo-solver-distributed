#pragma once

#include <string>
#include "logger_init.h"

class logger_control
{
public:
    logger_control();

    virtual ~logger_control();

    // Disable copying
    logger_control(const logger_control&) = delete;
    logger_control& operator=(const logger_control&) = delete;

    logger_control(logger_control&&) = default;
    logger_control& operator=(logger_control&&) = default;

    void enable_logging(const std::string& logFilePath, bool writeToConsole = true);

    void enable_logging(bool writeToConsole = true);

    void disable_logging();

protected:
    // Derived class implements this, which provides us with default log file name
    virtual std::string get_default_log_name() const = 0;

    bool is_logging_enabled() const;

private:
    bool _loggerEnabled = false;
};