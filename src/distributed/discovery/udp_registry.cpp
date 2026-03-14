#include "udp_registry.h"

#include <filesystem>
#include <fstream>
#include <iostream>

std::shared_ptr<udp_base> udp_registry::construct_udp(const std::string& name)
{
    std::cout << "udp_registry constructing an instance of: " << name << std::endl;

    // Anything calling this needs to first acquire the mutex, without this there could be a race condition on if (_udp_provider)
    std::scoped_lock lock(_registryMutex);

    // If this lib is already loaded, simply construct a new object
    if (_lib_loaders.contains(name))
    {
        return _lib_loaders.at(name).get_instance();
    }

    // Check if the lib exists in local cache, if yes, load it and return instance
    if (_is_lib_in_cache(name))
    {
        _load_lib(name);
        return _lib_loaders.at(name).get_instance();
    }

    // If lib is not loaded or present in cache, try to get it via provider
    if (_udp_provider)
    {
        const auto libFile = _udp_provider(name);
        if (libFile.has_value())
        {
            // The lib file was found and was passed to us as vector of bytes
            _save_lib_into_fs(name, libFile.value());
            _load_lib(name);
            // The loader should now be ready to construct an instance
            return _lib_loaders.at(name).get_instance();
        }
    }

    throw std::runtime_error("Error: cannot construct udp: " + name + ", this dynamic library was not found.");
}

void udp_registry::register_udp_provider(const udp_provider& providerFunc)
{
    std::cout << "udp_provider has been registered in udp_registry" << std::endl;

    // This will prevent provider from changing if any thread is executing inside construct_udp()
    std::scoped_lock lock(_registryMutex);
    _udp_provider = providerFunc;
}

void udp_registry::set_lib_cache(const std::string& directory)
{
    std::cout << "udp_registry cache has been set to:" << directory << std::endl;

    _lib_cache = directory;
}

void udp_registry::_save_lib_into_fs(const std::string& libName, const std::vector<std::byte>& libFile)
{
    std::cout << "udp_registry saving lib file: " << libName << std::endl;

    // TODO: Convert this to std::filesystem path
    const std::string path = _lib_cache + "/" + libName + portable_dll_extension();

    // We're assuming the parent directory exists
    if (!std::filesystem::exists(_lib_cache))
        std::filesystem::create_directory(_lib_cache);

    std::ofstream file(path, std::ios::binary);
    if (!file)
        throw std::runtime_error("Error: Failed to open file " + path);

    file.write(reinterpret_cast<const char*>(libFile.data()), libFile.size());

    if (!file)
        throw std::runtime_error("Error: Failed write to file " + path);
}

void udp_registry::_load_lib(const std::string& libName)
{
    std::cout << "udp_registry loading lib file: " << libName << std::endl;

    const std::string path = _lib_cache + "/" + libName + portable_dll_extension();

    _lib_loaders.insert({libName, lib_loader<udp_base>{path}});
    _lib_loaders.at(libName).open_lib();
}

bool udp_registry::_is_lib_in_cache(const std::string& libName) const
{
    // TODO: Convert this to std::filesystem path
    const std::string path = _lib_cache + "/" + libName + portable_dll_extension();

    return std::filesystem::exists(path);
}

std::optional<std::vector<std::byte>> udp_registry::get_lib_as_file(const std::string& libName) const
{
    // TODO: Add a cache for files to prevent repeated loading from the filesystem?
    std::cout << "get_lib_as_file: " << libName << std::endl;

    if (!_is_lib_in_cache(libName))
    {
        // If the file doesn't exist locally, we can try using udp provider
        if (_udp_provider)
        {
            return _udp_provider(libName);
        }
        return std::nullopt;
    }

    // TODO: Convert this to std::filesystem path
    const std::string libPath = _lib_cache + "/" + libName + portable_dll_extension();
    try
    {
        std::basic_ifstream<std::byte> fStream{libPath, std::ios::binary};
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

