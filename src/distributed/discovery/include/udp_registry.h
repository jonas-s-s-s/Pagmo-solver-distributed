#pragma once
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

#include "lib_loader.h"
#include "udp_base.h"

/**
 * A "service discovery" object implemented as singleton.
 * Allows any part of the application to request and load a UDP (User Defined Problems) from a dynamically loaded library.
 * UDP can be defined in separate DLLs, which can be transferred over the network and loaded at run time.
 */
class udp_registry
{
public:
    // TODO: Change?
    static constexpr std::string_view LIB_CACHE = "./lib_cache/";

    /**
     * Returns a static instance of this object
     */
    static udp_registry& get()
    {
        static udp_registry instance;
        return instance;
    }

    std::shared_ptr<udp_base> construct_udp(const std::string& name);

    using udp_provider = std::function<std::optional<std::vector<std::byte>>(const std::string&)>;
    void register_udp_provider(const udp_provider& providerFunc);

private:
    std::mutex _registryMutex{};

    // Map which stores lib_loader objects of libraries which we have already loaded into memory
    std::unordered_map<std::string, lib_loader<udp_base>> _lib_loaders{};

    // Lambda which returns the DLL / shared library file as an array of bytes
    udp_provider _udp_provider{};

    void _save_lib_into_fs(const std::string& libName, const std::vector<std::byte>& libFile);

    void _load_lib(const std::string& libName);

    udp_registry() = default;

public:
    udp_registry(udp_registry const&) = delete;
    void operator=(udp_registry const&) = delete;
};
