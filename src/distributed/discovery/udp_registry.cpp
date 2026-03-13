#include "udp_registry.h"

#include <filesystem>
#include <fstream>

std::shared_ptr<udp_base> udp_registry::construct_udp(const std::string& name)
{
    // If this lib is already loaded, simply construct a new object
    if (_lib_loaders.contains(name))
    {
        return _lib_loaders.at(name).get_instance();
    }

    // If lib is not loaded, try to get it via provider
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
    _udp_provider = providerFunc;
}

void udp_registry::_save_lib_into_fs(const std::string& libName, const std::vector<std::byte>& libFile)
{
    const std::string path = std::string(LIB_CACHE) + libName + portable_dll_extension();

    // We're assuming the parent directory exists
    if (!std::filesystem::exists(LIB_CACHE))
        std::filesystem::create_directory(LIB_CACHE);

    std::ofstream file(path, std::ios::binary);
    if (!file)
        throw std::runtime_error("Error: Failed to open file " + path);

    file.write(reinterpret_cast<const char*>(libFile.data()), libFile.size());

    if (!file)
        throw std::runtime_error("Error: Failed write to file " + path);
}

void udp_registry::_load_lib(const std::string& libName)
{
    const std::string path = std::string(LIB_CACHE) + libName + portable_dll_extension();
    _lib_loaders.insert({libName, lib_loader<udp_base>{path}});
    _lib_loaders.at(libName).open_lib();
}
