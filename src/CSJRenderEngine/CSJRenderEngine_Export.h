#ifndef __CSJRENDEREGNINE_EXPORT_H__
#define __CSJRENDEREGNINE_EXPORT_H__

#ifdef _WIN32
    #ifdef CSJRENDERENGINE_EXPORTS
        #define CSJRENDERENGINE_API __declspec(dllexport)
    #else 
        #define CSJRENDERENGINE_API __declspec(dllimport)
    #endif
#else // macOS and other Unix-like system.
    #ifdef CSJRENDERENGINE_EXPORTS
        #define CSJRENDERENGINE_API __attribute__((visibility("default")))
    #else
        #define CSJRENDERENGINE_API
    #endif

#endif // _WIN32

// #ifdef __cplusplus
// extern "C" {
// #endif

// TODO: Define symbols need to be exported.

// #ifdef __cplusplus
// }
// #endif

#endif // __CSJRENDEREGNINE_EXPORT_H__