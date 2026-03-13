#pragma once

#include <memory>
#include <utility>
#include <string>
#include <stdexcept>
#include "defines.h"

/**
 * - Dynamic library loader providing a POSIX / Win32 interface for loading a C++ dynamic library
 * - The header should have "allocator" and "deleter" functions specified, the "run_after_load" function may be specified as well
 * @tparam T The class contained within this library
 */
template <class T>
class lib_loader
{
private:
    HANDLE_TYPE _handle;
    std::string _pathToLib;
    std::string _allocSymbol;
    std::string _deleteSymbol;
    std::string _runAfterLoadSymbol;

    bool _afterLoadExecuted = false;

public:
    explicit lib_loader(std::string pathToLib,
                        std::string allocSymbol = "allocator",
                        std::string deleteSymbol = "deleter",
                        std::string _runAfterLoadSymbol = "run_after_load") :
        _handle(nullptr),
        _pathToLib(std::move(pathToLib)),
        _allocSymbol(std::move(allocSymbol)),
        _deleteSymbol(std::move(deleteSymbol)),
        _runAfterLoadSymbol(std::move(_runAfterLoadSymbol))
    {
    }

    virtual ~lib_loader()
    {
        close_lib();
    }

    /**
     * Opens the dynamic library (specified by pathToLib in the constructor)
     */
    void open_lib()
    {
        if (!(_handle = portable_dlopen(_pathToLib.c_str())))
        {
            throw std::runtime_error{error_msg()};
        }
    }

    /**
     * Creates an instance of the class contained within this dynamic library
     * @return Shared pointer to an instance of the class
     */
    std::shared_ptr<T> get_instance()
    {
        using allocT = T *(*)();
        using deleteT = void (*)(T*);
        using runAfterLoadT = void (*)();

        auto allocFunc = reinterpret_cast<allocT>(portable_dlsym(_handle, _allocSymbol.c_str()));
        auto deleteFunc = reinterpret_cast<deleteT>(portable_dlsym(_handle, _deleteSymbol.c_str()));

        // If run_after_load exists, run it (only once)
        if (!_afterLoadExecuted)
        {
            _afterLoadExecuted = true;

            auto runAfterLoadFunc = reinterpret_cast<runAfterLoadT>(
                portable_dlsym(_handle, _runAfterLoadSymbol.c_str())
            );
            if (runAfterLoadFunc)
            {
                runAfterLoadFunc();
            }
        }

        if (!allocFunc || !deleteFunc)
        {
            close_lib();
            throw std::runtime_error("Can't find allocator or deleter symbol in " + _pathToLib);
        }

        return std::shared_ptr<T>(allocFunc(), [deleteFunc](T* p) { deleteFunc(p); });
    }

    /**
     * Closes the dynamically loaded library
     */
    void close_lib()
    {
        if (!_handle)
            return;

        if (has_dlclose_failed(portable_dlclose(_handle)))
        {
            throw std::runtime_error{error_msg()};
        }

        _handle = nullptr;
    }
};
