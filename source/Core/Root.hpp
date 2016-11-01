#pragma once

// external includes
#include <new>
#include <assert.h>
#include <windows.h>

// platform defintions
#ifndef NULL
    #define NULL 0
#endif
#ifdef _WIN64
    #define CPU_64 1
#else
    #define CPU_64 0
#endif

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

// defines
#define LOG_PATH                "d:\\debug.log"//!!!
#define INVALID_ID              (~0)
#define INVALID_INDEX           (~0)

namespace Core
{
    // forward declarations
    template<typename TYPE> class Enum;

    /*
        primitive size requirements:
        
        Char,Bool,Byte          1
        Short,UShort            2
        Long,ULong,Float        4
        Huge,UHuge,Double       8
    */
    typedef float               Real4;
    typedef double              Real8;

    typedef bool                Bool;
    typedef char                Char;
    typedef unsigned char       Byte;
    typedef short               Short;
    typedef unsigned short      UShort;
    typedef int                 Long;
    typedef unsigned int        ULong;
    typedef long long           Huge;
    typedef unsigned long long  UHuge;
    typedef float               Float;
    typedef double              Double;

    typedef Byte                Byte1;
    typedef UShort              Byte2;
    typedef ULong               Byte4;
    typedef UHuge               Byte8;

    typedef ULong               Bits;
    typedef Long                Id;
    typedef ULong               Index;
    typedef void*               Handle;
    typedef Byte8               Hash;

    #if CPU_64
        typedef Huge            Native;
        typedef UHuge           UNative;
    #else
        typedef Long            Native;
        typedef ULong           UNative;
    #endif

    #ifdef PRECISION_DOUBLE
        typedef Double          Real;
    #else
        typedef Float           Real;
    #endif


    /**/
    #ifdef NDEBUG
        #define xassert(x) ((void)0)
    #else
        extern void LogAssert( const char* message, const char *file, unsigned line );
        #define xassert(x) ((x) ? (void)0 : (LogAssert(#x, __FILE__, __LINE__)))
    #endif

    extern void ExceptionMessage( const char* message, const char *file, unsigned line );
    #define eggs(x) ((x) ? (void)0 : ExceptionMessage(#x, __FILE__, __LINE__))

    /**/
    extern void LogFile( const Char* format, ... );
    #define dlog LogFile
}
