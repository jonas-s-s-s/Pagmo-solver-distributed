#pragma once

#include <memory>
#include <utility>
#include <string>
#include <stdexcept>
#include "defines.h"

/**
 * - Dynamic library loader providing a POSIX / Win32 interface for loading a C++ dynamic library
 * - The header should have these functions specified:
 *      - "allocator" returning a pointer to new instance
 *      - "deleter" deleting the object allocated by allocator
 * - The following functions are optional:
 *      - "run_after_load" which gets executed during the first call to get_instance or clone_instance
 *      - "cloner" which copy-constructs a new object from the passed pointer
 *
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
    std::string _cloneSymbol;

    bool _afterLoadExecuted = false;

    void _ensure_after_load()
    {
        if (_afterLoadExecuted)
            return;

        _afterLoadExecuted = true;

        using runAfterLoadT = void (*)();

        auto runAfterLoadFunc = reinterpret_cast<runAfterLoadT>(
            portable_dlsym(_handle, _runAfterLoadSymbol.c_str())
        );

        if (runAfterLoadFunc)
        {
            runAfterLoadFunc();
        }
    }

public:
    explicit lib_loader(std::string pathToLib,
                        std::string allocSymbol = "allocator",
                        std::string deleteSymbol = "deleter",
                        std::string runAfterLoadSymbol = "run_after_load",
                        std::string cloneSymbol = "cloner") :
        _handle(nullptr),
        _pathToLib(std::move(pathToLib)),
        _allocSymbol(std::move(allocSymbol)),
        _deleteSymbol(std::move(deleteSymbol)),
        _runAfterLoadSymbol(std::move(runAfterLoadSymbol)),
        _cloneSymbol(std::move(cloneSymbol))
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

        auto allocFunc = reinterpret_cast<allocT>(
            portable_dlsym(_handle, _allocSymbol.c_str())
        );
        auto deleteFunc = reinterpret_cast<deleteT>(
            portable_dlsym(_handle, _deleteSymbol.c_str())
        );

        _ensure_after_load();

        if (!allocFunc || !deleteFunc)
        {
            close_lib();
            throw std::runtime_error("Can't find allocator or deleter symbol in " + _pathToLib);
        }

        return std::shared_ptr<T>(
            allocFunc(),
            [deleteFunc](T* p) { deleteFunc(p); }
        );
    }

    /**
     * Clones an existing instance using the library-provided clone function
     */
    std::shared_ptr<T> clone_instance(const std::shared_ptr<T>& other)
    {
        using cloneT = T* (*)(const T*);
        using deleteT = void (*)(T*);

        auto cloneFunc = reinterpret_cast<cloneT>(
            portable_dlsym(_handle, _cloneSymbol.c_str())
        );

        auto deleteFunc = reinterpret_cast<deleteT>(
            portable_dlsym(_handle, _deleteSymbol.c_str())
        );

        _ensure_after_load();

        if (!cloneFunc || !deleteFunc)
        {
            throw std::runtime_error("Can't find clone or deleter symbol in " + _pathToLib);
        }

        return std::shared_ptr<T>(
            cloneFunc(other.get()),
            [deleteFunc](T* p) { deleteFunc(p); }
        );
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