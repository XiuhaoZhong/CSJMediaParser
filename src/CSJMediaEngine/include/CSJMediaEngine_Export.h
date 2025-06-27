#ifndef __CSJMEDIAEGNINE_EXPORT_H__
#define __CSJMEDIAEGNINE_EXPORT_H__

#ifdef _WIN32
    #ifdef CSJMEDIAENGINE_EXPORTS
        #define CSJMEDIAENGINE_API __declspec(dllexport)
    #else 
        #define CSJMEDIAENGINE_API __declspec(dllimport)
    #endif
#else // macOS and other Unix-like system.
    #ifdef CSJMEDIAENGINE_EXPORTS
        #define CSJMEDIAENGINE_API __attribute__((visibility("default")))
    #else
        #define CSJMEDIAENGINE_API
    #endif

#endif // _WIN32

// #ifdef __cplusplus
// extern "C" {
// #endif

// // TODO: Define symbols need to be exported.

// #ifdef __cplusplus
// }
// #endif

#endif // __CSJMEDIAEGNINE_EXPORT_H__