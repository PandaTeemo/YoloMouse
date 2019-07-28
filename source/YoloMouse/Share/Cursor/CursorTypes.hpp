#pragma once
#include <YoloMouse/Share/Root.hpp>

namespace Yolomouse
{
    // enums
    //-------------------------------------------------------------------------
    enum CursorType: Byte
    {
        CURSOR_TYPE_BASIC,      // one of preinstalled (or user provided) cur/ani files
        CURSOR_TYPE_CLONE,      // clone of existing application cursor, used to modding that cursor such as resize
        CURSOR_TYPE_OVERLAY,    // a directx overlay cursor, this has a little lag despite methods to reduce it, but its much prettier!
        CURSOR_TYPE_INVALID
    };

    typedef Byte CursorId;
    enum: Byte
    {
        CURSOR_ID_COUNT =           10,
        CURSOR_ID_INVALID,
        CURSOR_ID_OVERLAY_COUNT =   2,
    };

    typedef Byte CursorVariation;
    enum: Byte
    {
        CURSOR_VARIATION_COUNT =    10,
        CURSOR_VARIATION_INVALID
    };

    typedef Char CursorSize;
    enum: Char
    {
        CURSOR_SIZE_INVALID,
        CURSOR_SIZE_MIN =           1,
        CURSOR_SIZE_DEFAULT =       7,
        CURSOR_SIZE_COUNT =         16
    };

    typedef Byte CursorUpdateFlags;
    enum: Byte
    {
        CURSOR_UPDATE_INCREMENT_TYPE =         BIT(0),
        CURSOR_UPDATE_INCREMENT_ID =           BIT(1),
        CURSOR_UPDATE_INCREMENT_VARIATION =    BIT(2),
        CURSOR_UPDATE_INCREMENT_SIZE =         BIT(3),
        CURSOR_UPDATE_DECREMENT_SIZE =         BIT(4),
    };

    typedef Byte CursorIterateFlags;
    enum: Byte
    {
        CURSOR_ITERATE_TYPE =         BIT(0),
        CURSOR_ITERATE_ID =           BIT(1),
        CURSOR_ITERATE_VARIATION =    BIT(2),
        CURSOR_ITERATE_SIZE =         BIT(3),
    };
}
