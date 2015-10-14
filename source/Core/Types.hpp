#pragma once

#define INVALID_ID          (~0)
#define INVALID_INDEX       (~0)

namespace Core
{
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
}
