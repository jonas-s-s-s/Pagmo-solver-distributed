#pragma once

#include <string>

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <system_error>

using HANDLE_TYPE = HMODULE;

inline HANDLE_TYPE portable_dlopen(const char* path)
{
    return LoadLibraryA(path);
}

inline FARPROC portable_dlsym(HANDLE_TYPE handle, const char* symbol)
{
    return GetProcAddress(handle, symbol);
}

inline int portable_dlclose(HANDLE_TYPE handle)
{
    return FreeLibrary(handle);
}

inline bool has_dlclose_failed(int return_value)
{
    return return_value == 0;
}

inline std::string error_msg()
{
    return std::string("LoadLibrary / CloseLibrary error code: ") + std::to_string(GetLastError());
}

#elif defined(__linux__) || defined(__APPLE__)

#include <dlfcn.h>

using HANDLE_TYPE = void*;

inline HANDLE_TYPE portable_dlopen(const char* path)
{
    return dlopen(path, RTLD_NOW | RTLD_LAZY);
}

inline void* portable_dlsym(HANDLE_TYPE handle, const char* symbol)
{
    return dlsym(handle, symbol);
}

inline int portable_dlclose(HANDLE_TYPE handle)
{
    return dlclose(handle);
}

inline bool has_dlclose_failed(int return_value)
{
    return return_value != 0;
}

inline std::string error_msg()
{
    const char* err = dlerror();
    return err ? std::string(err) : std::string{"Unknown Error"};
}

#else
#error "Dynamic library loader is not supported on this platform (none of the macros: __linux__, __APPLE__ or WIN32 are defined)"
#endif
