#ifndef EXPORT_HPP
#define EXPORT_HPP


#if defined(_WIN32)
  #if defined(MYLIB_BUILD_SHARED)
    #define COMLIBPP_API __declspec(dllexport)
  #elif defined(MYLIB_USE_SHARED)
    #define COMLIBPP_API __declspec(dllimport)
  #else
    #define COMLIBPP_API
  #endif
#else
  #if defined(COMLIBPP_BUILD_SHARED)
    #define COMLIBPP_API __attribute__((visibility("default")))
  #else
    #define COMLIBPP_API
  #endif
#endif


#endif // EXPORT_HPP
