#pragma once

#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)

#if defined(__GNUC__)
  #define DLL_PUBLIC __attribute__((dllexport))
#else
  #define DLL_PUBLIC __declspec(dllexport)
#endif

#define DLL_LOCAL

#else

#if defined(__GNUC__) && __GNUC__ >= 4
  #define DLL_PUBLIC __attribute__((visibility("default")))
  #define DLL_LOCAL  __attribute__((visibility("hidden")))
#else
  #define DLL_PUBLIC
  #define DLL_LOCAL
#endif

#endif