#include "dll_locator.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "defines.h"

dll_locator::dll_locator(const std::string& local_dll_location) : _local_dll_location(local_dll_location)
{
}

std::optional<std::vector<std::byte>> dll_locator::get_dll(const std::string& libName) const
{
    if (!_local_dll_location.empty() && !std::filesystem::exists(_local_dll_location))
    {
        // The file's parent directory doesn't exist
        return std::nullopt;
    }

    const std::string libPath = _local_dll_location + "/" + libName + portable_dll_extension();
    if (!std::filesystem::exists(libPath))
    {
        // The library file doesn't exist
        return std::nullopt;
    }

    try
    {
        std::basic_ifstream<std::byte> fStream{libName + ".dll", std::ios::binary};
        std::vector<std::byte> fileContent{std::istreambuf_iterator(fStream), {}};
        return fileContent;
    }
    catch (std::exception e)
    {
        std::cerr << "Warning: error when attempting to load file: " << libPath << "Err msg:" << e.what() <<
            " Proceeding as if it doesn't exist." << std::endl;
        return std::nullopt;
    }
    catch (...)
    {
        std::cerr << "Warning: unknown error when attempting to load file: " << libPath <<
            " Proceeding as if it doesn't exist." << std::endl;
        return std::nullopt;
    }
}
