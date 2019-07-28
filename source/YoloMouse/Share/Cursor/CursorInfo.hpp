#pragma once
#include <YoloMouse/Share/Cursor/CursorTypes.hpp>

namespace Yolomouse
{
    /**/
    struct CursorInfo
    {
        // fields
        CursorType      type;
        CursorId        id;
        CursorVariation variation;
        CursorSize      size;

        /**/
        CursorInfo(
            CursorType      type_ =      CURSOR_TYPE_INVALID, 
            CursorId        id_ =        CURSOR_ID_INVALID,
            CursorVariation variation_ = CURSOR_VARIATION_INVALID,
            CursorSize      size_ =      CURSOR_SIZE_INVALID );

        /**/
        Bool IsValid() const;
    };
}
