// Adapted from doldecomp/ogws include/types.h (CC0-1.0); see include/rvl/README.md.
// Deviations from the original, all to keep this project's sources compiling identically:
//  - u32/s32 are int-based (the original uses long), the same as the typedefs this project already uses;
//  - no MSL <cstdarg>/<cstddef>/<new> includes (the project builds without an MSL include path): size_t is
//    declared here (the same typedef as before), va_list is not provided.
#ifndef TYPES_H
#define TYPES_H

#include <decomp/compat.h>
#include <decomp/decomp.h>
#include <decomp/lang.h>

#include <macros.h>

typedef unsigned long long u64;
typedef signed long long s64;

typedef unsigned int u32;
typedef signed int s32;

typedef unsigned short u16;
typedef signed short s16;

typedef unsigned char u8;
typedef signed char s8;

typedef float f32;
typedef double f64;

typedef unsigned long size_t;

typedef int UNKWORD;
typedef void UNKTYPE;

enum {
    FALSE,
    TRUE
};
typedef int BOOL;

typedef void (*funcptr_t)(void);

#endif
