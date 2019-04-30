#pragma once
#include <Core/Root.hpp>

namespace Core
{
    // enums
    //-------------------------------------------------------------------------
    enum Bitness
    {
        BITNESS_UNKNOWN,
        BITNESS_32,
        BITNESS_64,
    };

    enum OsVersion
    {
        OSVERSION_UNKNOWN,
        OSVERSION_WIN2K =           0x0500,
        OSVERSION_WINXP =           0x0501,
        OSVERSION_WINXPPRO =        0x0502,
        OSVERSION_WINVISTA =        0x0600,
        OSVERSION_WIN7 =            0x0601,
        OSVERSION_WIN8 =            0x0602,
        OSVERSION_WIN81 =           0x0603,
    };

    // numeric
    //-------------------------------------------------------------------------
    static const ULong STRING_SHORT_SIZE =          16;
    static const ULong STRING_MEDIUM_SIZE =         64;
    static const ULong STRING_MAX_SIZE =            512;
    static const ULong STRING_PATH_SIZE =           STRING_MAX_SIZE;
    static const ULong GAME_WINDOW_MIN_WIDTH =      240;
    static const ULong GAME_WINDOW_MIN_HEIGHT =     240;
}
