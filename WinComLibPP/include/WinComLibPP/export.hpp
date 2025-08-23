#ifndef EXPORT_HPP
#define EXPORT_HPP


#if defined(_WIN32)
  #if defined(MYLIB_BUILD_SHARED)
    #define WINCOMLIBPP_API __declspec(dllexport)
  #elif defined(MYLIB_USE_SHARED)
    #define WINCOMLIBPP_API __declspec(dllimport)
  #else
    #define WINCOMLIBPP_API
  #endif
#else
  #if defined(WINCOMLIBPP_BUILD_SHARED)
    #define WINCOMLIBPP_API __attribute__((visibility("default")))
  #else
    #define WINCOMLIBPP_API
  #endif
#endif


#endif // EXPORT_HPP
