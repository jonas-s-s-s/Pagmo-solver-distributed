#pragma once
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <unordered_map>
#include <any>

#include "lib_loader.h"
#include "lru_cache.h"
#include "udp_base.h"

/**
 * A "service discovery" object implemented as singleton.
 * Allows any part of the application to request and load a UDP (User Defined Problems) from a dynamically loaded library.
 * UDP can be defined in separate DLLs, which can be transferred over the network and loaded at run time.
 */
class udp_registry
{
public:
    /**
     * Returns a static instance of this object
     */
    static udp_registry& get()
    {
        static udp_registry instance;
        return instance;
    }

    /**
     * Loads the dynamic library without constructing any object
     * @param name Name of the UDP's library
     */
    void initialize_udp(const std::string& name);

    /**
     * Constructs a UDP, which is contained within some dynamic library
     * @param name Name of the UDP's library
     * @param params std::any containing a value passed to the UDP's constructor
     * @return Shared ptr pointing to a newly constructed instance of the UDP
     */
    std::shared_ptr<udp_base> construct_udp(const std::string& name, const std::any& params = std::any());

    /**
     * Clones a UDP which was previously created using construct_udp
     * @param other pointer to the object that's meant to be cloned
     * @return Shared ptr pointing to the cloned object
     */
    std::shared_ptr<udp_base> clone_udp(const std::shared_ptr<udp_base>& other);


    using udp_provider = std::function<std::optional<std::vector<std::byte>>(const std::string&)>;

    /**
     * Registers a function which will be called if the udp_registry can't find a DLL locally
     * @param providerFunc Function that returns the DLL file as byte vector, or an empty std::optional if it doesn't exist
     */
    void register_udp_provider(const udp_provider& providerFunc);

    void set_local_cache_dir(const std::filesystem::path& directory);

    void use_in_memory_cache(bool value);

    std::optional<std::vector<std::byte>> get_lib_as_file(const std::string& libName);

private:
    std::mutex _registryMutex{};

    // Where to store or look for DLL files
    std::filesystem::path _local_cache = ".";

    // An in-memory cache which prevents repeated reading of the DLL file from FS
    bool _use_in_memory_cache = true;
    lru_cache<std::vector<std::byte>> _libFilesBuffer{5};

    // Map which stores lib_loader objects of libraries which we have already loaded into memory
    std::unordered_map<std::string, lib_loader<udp_base>> _lib_loaders{};

    // Lambda which returns the DLL / shared library file as an array of bytes
    udp_provider _udp_provider{};

    void _save_lib_into_fs(const std::string& libName, const std::vector<std::byte>& libFile);

    void _load_lib(const std::string& libName);

    bool _is_lib_in_cache(const std::string& libName) const;

    /**
     * Returns a full path to a library file located in local cache (including extension)
     */
    std::filesystem::path _get_lib_path(const std::string& libName) const;

    udp_registry() = default;

public:
    udp_registry(udp_registry const&) = delete;
    void operator=(udp_registry const&) = delete;
};
