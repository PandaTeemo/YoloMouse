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
        Int,UInt                native
    */
    typedef char                Int1;
    typedef short               Int2;
    typedef int                 Int4;
    typedef long long           Int8;
    typedef unsigned char       UInt1;
    typedef unsigned short      UInt2;
    typedef unsigned int        UInt4;
    typedef unsigned long long  UInt8;
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
    typedef int                 Int;
    typedef unsigned int        UInt;

    typedef Byte                Byte1;
    typedef UShort              Byte2;
    typedef ULong               Byte4;
    typedef UHuge               Byte8;

    typedef ULong               Bits;
    typedef Long                Id;
    typedef ULong               Index;
    typedef void*               Handle;
    typedef Byte8               Hash;

    #ifdef PRECISION_DOUBLE
        typedef Double          Real;
    #else
        typedef Float           Real;
    #endif
}
