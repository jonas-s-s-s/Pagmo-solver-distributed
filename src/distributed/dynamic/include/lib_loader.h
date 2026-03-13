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
    std::string _allocClassSymbol;
    std::string _deleteClassSymbol;
    std::string _runAfterLoadClassSymbol;

    bool _afterLoadExecuted = false;

public:
    explicit lib_loader(std::string pathToLib,
                        std::string allocClassSymbol = "allocator",
                        std::string deleteClassSymbol = "deleter",
                        std::string _runAfterLoadClassSymbol = "run_after_load") :
        _handle(nullptr),
        _pathToLib(std::move(pathToLib)),
        _allocClassSymbol(std::move(allocClassSymbol)),
        _deleteClassSymbol(std::move(deleteClassSymbol)),
        _runAfterLoadClassSymbol(std::move(_runAfterLoadClassSymbol))
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
        using allocClass = T *(*)();
        using deleteClass = void (*)(T*);

        auto allocFunc = reinterpret_cast<allocClass>(portable_dlsym(_handle, _allocClassSymbol.c_str()));
        auto deleteFunc = reinterpret_cast<deleteClass>(portable_dlsym(_handle, _deleteClassSymbol.c_str()));

        // If run_after_load exists, run it (only once)
        if (!_afterLoadExecuted)
        {
            _afterLoadExecuted = true;

            auto runAfterLoadFunc = reinterpret_cast<allocClass>(portable_dlsym(
                _handle, _runAfterLoadClassSymbol.c_str()));
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
