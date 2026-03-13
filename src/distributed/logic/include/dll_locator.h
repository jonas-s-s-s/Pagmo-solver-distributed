#pragma once
#include <optional>
#include <string>
#include <vector>

/**
 * A class that encapsulates all logic related to finding DLLs (or shared libs on UNIX-like OS) in the local FS.
 * These DLLs can contain UDPs (User Defined Problems) and can then be loaded at run time by udp_registry.
 * Although this could all be implemented in distributed_worker directly, using a separate class is a cleaner approach.
 */
class dll_locator
{
    // This is the directory where DLLs are stored, empty string means the same dir as the executable
    std::string _local_dll_location = "";

    // TODO: Add a cache for files to prevent repeated loading from the filesystem?

public:
    explicit dll_locator(const std::string& local_dll_location = "");

    std::optional<std::vector<std::byte>> get_dll(const std::string& libName) const;

};