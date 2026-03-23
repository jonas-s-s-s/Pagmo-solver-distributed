#include "udp_registry.h"

#include <filesystem>
#include <fstream>
#include <iostream>

//#####################################################################################
//# Public member functions
//#####################################################################################

void udp_registry::initialize_udp(const std::string& name)
{
    std::scoped_lock lock(_registryMutex);

    std::cout << "udp_registry initializing udp library: " << name << std::endl;

    // If this lib is already loaded, do nothing
    if (_lib_loaders.contains(name))
    {
        return;
    }

    // Check if the lib exists in local cache, if yes, load it
    if (_is_lib_in_cache(name))
    {
        _load_lib(name);
        return;
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
            return;
        }
    }

    throw std::runtime_error("Error: cannot initialize udp: " + name + ", this dynamic library was not found.");
}

std::shared_ptr<udp_base> udp_registry::construct_udp(const std::string& name)
{
    // Anything calling this needs to first acquire the mutex, without this there could be a race condition on if (_udp_provider)
    std::scoped_lock lock(_registryMutex);

    std::cout << "udp_registry constructing an instance of: " << name << std::endl;

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

std::shared_ptr<udp_base> udp_registry::clone_udp(const std::shared_ptr<udp_base>& other)
{
    std::scoped_lock lock(_registryMutex);

    std::cout << "udp_registry cloning an instance of: " << other->get_lib_file_name() << std::endl;

    // We get the lib_loader associated with this UDP and call its clone function
    const std::string libName = other->get_lib_file_name();
    if (_lib_loaders.contains(libName))
    {
        return _lib_loaders.at(libName).clone_instance(other);
    }

    throw std::runtime_error("Error: cannot clone udp: " + libName + ", this dynamic library was not found.");
}

void udp_registry::register_udp_provider(const udp_provider& providerFunc)
{
    // This will prevent provider from changing if any thread is executing inside construct_udp()
    std::scoped_lock lock(_registryMutex);

    std::cout << "udp_provider has been registered in udp_registry" << std::endl;

    _udp_provider = providerFunc;
}

void udp_registry::set_local_cache_dir(const std::filesystem::path& directory)
{
    std::scoped_lock lock(_registryMutex);

    std::cout << "udp_registry cache has been set to: " << directory << std::endl;

    _local_cache = directory;
}

void udp_registry::use_in_memory_cache(bool value)
{
    std::scoped_lock lock(_registryMutex);

    if (!value)
    {
        _libFilesBuffer.clear();
    }
    _use_in_memory_cache = value;
}

std::optional<std::vector<std::byte>> udp_registry::get_lib_as_file(const std::string& libName)
{
    std::scoped_lock lock(_registryMutex);

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

    const std::filesystem::path libPath = _get_lib_path(libName);

    // Check our LRU buffer if this file isn't already cached in-memory
    if (_use_in_memory_cache && _libFilesBuffer.contains(libPath.string()))
    {
        return _libFilesBuffer.get(libPath.string());
    }

    // Load file from FS
    try
    {
        std::ifstream fStream{libPath, std::ios::binary};
        if (!fStream)
        {
            return std::nullopt;
        }

        fStream.seekg(0, std::ios::end);
        std::size_t size = fStream.tellg();
        fStream.seekg(0, std::ios::beg);

        std::vector<std::byte> fileContent(size);

        fStream.read(reinterpret_cast<char*>(fileContent.data()), size);

        if (_use_in_memory_cache)
        {
            _libFilesBuffer.put(libPath.string(), fileContent);
        }

        return fileContent;
    }
    catch (const std::exception& e)
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

//#####################################################################################
//# Private member functions
//#####################################################################################

void udp_registry::_save_lib_into_fs(const std::string& libName, const std::vector<std::byte>& libFile)
{
    std::cout << "udp_registry saving lib file: " << libName << std::endl;

    const std::filesystem::path libPath = _get_lib_path(libName);

    // We're assuming the parent directory exists
    if (!std::filesystem::exists(_local_cache))
        std::filesystem::create_directory(_local_cache);

    std::ofstream file(libPath, std::ios::binary);
    if (!file)
        throw std::runtime_error("Error: Failed to open file " + libPath.string());

    file.write(reinterpret_cast<const char*>(libFile.data()), libFile.size());

    if (!file)
        throw std::runtime_error("Error: Failed write to file " + libPath.string());
}

void udp_registry::_load_lib(const std::string& libName)
{
    std::cout << "udp_registry loading lib file: " << libName << std::endl;

    _lib_loaders.insert({libName, lib_loader<udp_base>{_get_lib_path(libName).string()}});
    _lib_loaders.at(libName).open_lib();
}

bool udp_registry::_is_lib_in_cache(const std::string& libName) const
{
    return std::filesystem::exists(_get_lib_path(libName));
}

std::filesystem::path udp_registry::_get_lib_path(const std::string& libName) const
{
    return _local_cache / std::string{libName + portable_dll_extension()};
}
