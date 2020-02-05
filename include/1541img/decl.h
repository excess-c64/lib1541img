#ifndef I1541_DECL_H
#define I1541_DECL_H

/** Common preprocessor declarations for lib1541img.
 * This file defines some macros to control symbol visibility and
 * automatically declare C linkage as needed. If you link against a static
 * version of lib1541img, you need to define STATIC_1541IMG for them to work
 * correctly.
 * @file
 */

#undef i1541___cdecl
#undef SOEXPORT
#undef SOLOCAL
#undef DECLEXPORT

#ifdef __cplusplus
#  define i1541___cdecl extern "C"
#  define DECLDATA
#  define C_CLASS_DECL(t) struct t
#else
#  define i1541___cdecl
#  define DECLDATA extern
#  define C_CLASS_DECL(t) typedef struct t t
#endif

#ifndef __GNUC__
#  define __attribute__(x)
#endif

#ifdef _WIN32
#  define SOLOCAL
#  ifdef STATIC_1541IMG
#    define DECLEXPORT i1541___cdecl
#    define SOEXPORT i1541___cdecl
#  else
#    ifdef BUILDING_1541IMG
#      define DECLEXPORT i1541___cdecl __declspec(dllexport)
#    else
#      define DECLEXPORT i1541___cdecl __declspec(dllimport)
#    endif
#    define SOEXPORT i1541___cdecl __declspec(dllexport)
#  endif
#else
#  define DECLEXPORT i1541___cdecl
#  if __GNUC__ >= 4
#    define SOEXPORT i1541___cdecl __attribute__((visibility("default")))
#    define SOLOCAL __attribute__((visibility("hidden")))
#  else
#    define SOEXPORT i1541___cdecl
#    define SOLOCAL
#  endif
#endif

#endif
