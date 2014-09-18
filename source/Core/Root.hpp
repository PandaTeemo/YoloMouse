#pragma once

// external includes
#include <new>
#include <assert.h>
#include <windows.h>

// platform defintions
#ifndef NULL
    #define NULL 0
#endif

// default includes
#include <Core/Types.hpp>

// macros
#define COUNT(array)            (sizeof(array) / sizeof(array[0]))
#define BIT(bit)                (static_cast<unsigned int>(1 << (bit)))
#define KILOBYTES(n)            ((n) * 1024)
#define STRUCT_FIELD(t, f)      (((t*)8)->f) // the nonzero (8) is to avoid a warning
#define STRUCT_OFFSET(t, f)     ((long)(long long*)&STRUCT_FIELD(t,f) - 8)
#define STRUCT_SIZE(t, f)       (sizeof(STRUCT_FIELD(t, f)))

#define LOW32(huge)             ((unsigned long)((huge) & 0xffffffff))
#define HIGH32(huge)            ((unsigned long)(((huge) >> 32) & 0xffffffff))
#define MAKEHUGE(low, high)     ((unsigned long long)(low) | ((unsigned long long)(high) << 32))

namespace Core
{
    /**/
    #ifdef NDEBUG
        #define xassert(x) ((void)0)
    #else
        extern void LogAssert(const char* message, const char *file, unsigned line);
        #define xassert(x) ((x) ? (void)0 : (LogAssert(#x, __FILE__, __LINE__)))
    #endif

    extern void ExceptionMessage( const char* message, const char *file, unsigned line );
    #define eggs(x) ((x) ? (void)0 : ExceptionMessage(#x, __FILE__, __LINE__))

    /**/
    #if 0
        #define xlog(...) ((void)0)
    #else
        #define xlog Core::Debug::Log
    #endif

    // forward declarations
    template<typename TYPE> class Enum;
}
