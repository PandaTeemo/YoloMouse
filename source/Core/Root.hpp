#pragma once

// options
//-----------------------------------------------------------------------------
#define OPTION_DEBUG_LOG_LEVEL          2
#define OPTION_DEBUG_LOG_FILE
//#define OPTION_DEBUG_LOG_CONSOLE

// external includes
//-----------------------------------------------------------------------------
#include <new>
#include <assert.h>
#include <windows.h>

// platform defintions
//-----------------------------------------------------------------------------
#ifndef NULL
    #define NULL 0
#endif
#ifdef _WIN64
    #define CPU_64 1
#else
    #define CPU_64 0
#endif

// build defintions
//-----------------------------------------------------------------------------
#ifdef NDEBUG
    #define BUILD_RELEASE
#else
    #define BUILD_DEBUG
#endif
/*
#ifdef BUILD_RELEASE
    #undef  OPTION_DEBUG_LOG_LEVEL
    #define OPTION_DEBUG_LOG_LEVEL       0
#endif
//*/

// type defines
//-----------------------------------------------------------------------------
#define INVALID_ID              (~0)
#define INVALID_INDEX           (~0)
#define INVALID_HANDLE          (~0)

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

#if 0
    /**/
    #ifdef NDEBUG
        #define ASSERT(x) ((void)0)
    #else
        extern void LogAssert( const char* message, const char *file, unsigned line );
        #define ASSERT(x) ((x) ? (void)0 : (LogAssert(#x, __FILE__, __LINE__)))
    #endif

    extern void ExceptionMessage( const char* message, const char *file, unsigned line );
    #define eggs(x) ((x) ? (void)0 : ExceptionMessage(#x, __FILE__, __LINE__))

    /**/
    extern void LogSetPath( const Char* format, ... );
    extern void LogFile( const Char* format, ... );
    #define LOG2 LogFile

    /**/
    extern void LogConsole( const Char* format, ... );
    #define clog LogConsole
#endif

    // debug functions
    //-------------------------------------------------------------------------
    void DebugFatal( const Char* format, ... );
    void DebugLog( const Char* format, ... );
    void DebugAssert( const Char* message, const Char *file, ULong line );
}

// macros: etc
//-----------------------------------------------------------------------------
#define COUNT(array)            (sizeof(array) / sizeof(array[0]))
#define BIT(bit)                (static_cast<unsigned int>(1 << (bit)))
#define KILOBYTES(n)            ((n) * 1024)
#define STRUCT_FIELD(t, f)      (((t*)8)->f) // the nonzero (8) is to avoid a warning
#define STRUCT_OFFSET(t, f)     ((long)(long long*)&STRUCT_FIELD(t,f) - 8)
#define STRUCT_SIZE(t, f)       (sizeof(STRUCT_FIELD(t, f)))

#define LOW32(huge)             ((unsigned long)((huge) & 0xffffffff))
#define HIGH32(huge)            ((unsigned long)(((huge) >> 32) & 0xffffffff))
#define MAKEHUGE(low, high)     ((unsigned long long)(low) | ((unsigned long long)(high) << 32))

// macros: debug
//-----------------------------------------------------------------------------
#define ASSERTF(m)  ((m) ? (void)0 : (Core::DebugFatal(#m, __FILE__, __LINE__)))
#define FATAL       Core::DebugFatal
#define LOG         Core::DebugLog
#ifdef BUILD_RELEASE
    #define DEBUG(s)
    #define ASSERT(x)
    #define ASSERT_TODO
#else
    #define DEBUG(code)     code
    #define ASSERT(m)       ((m) ? (void)0 : (Core::DebugAssert(#m, __FILE__, __LINE__)))
    #define ASSERT_TODO     Core::DebugFatal("NOT_SUPPORTED", __FILE__, __LINE__)
#endif
#if OPTION_DEBUG_LOG_LEVEL >= 1
    #define LOG1 Core::DebugLog
#else
    #define LOG1
#endif
#if OPTION_DEBUG_LOG_LEVEL >= 2
    #define LOG2 Core::DebugLog
#else
    #define LOG2
#endif
#if OPTION_DEBUG_LOG_LEVEL >= 3
    #define LOG3 Core::DebugLog
#else
    #define LOG3
#endif
