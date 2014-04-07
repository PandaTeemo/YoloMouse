#pragma once
#include <assert.h>

namespace Core
{
    /**/
    #define COUNT(array)            (sizeof(array) / sizeof(array[0]))
    #define BIT(bit)                (static_cast<unsigned int>(1 << (bit)))
    #define KILOBYTES(n)            ((n) * 1024)
    #define STRUCT_FIELD(t, f)      (((t*)8)->f) // the nonzero (8) is to avoid a warning
    #define STRUCT_OFFSET(t, f)     ((long)(long long*)&STRUCT_FIELD(t,f) - 8)
    #define STRUCT_SIZE(t, f)       (sizeof(STRUCT_FIELD(t, f)))

    #define REAL(v)                 static_cast<Real>(v)

    /**/
    #ifdef NDEBUG
        #define xassert(x) ((void)0)
    #else
        extern void xAssert(const char* message, const char *file, unsigned line);
        #define xassert(x) ((x) ? (void)0 : (xAssert(#x, __FILE__, __LINE__)))
    #endif
}
