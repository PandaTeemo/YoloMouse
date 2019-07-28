#include <YoloMouse/Share/Cursor/CursorInfo.hpp>

namespace Yolomouse
{
    // public
    //-------------------------------------------------------------------------
    CursorInfo::CursorInfo( CursorType type_, CursorId id_, CursorVariation variation_, CursorSize size_ ):
        type     (type_),
        id       (id_),
        variation(variation_),
        size     (size_)
    {
    }

    //-------------------------------------------------------------------------
    Bool CursorInfo::IsValid() const
    {
        // validate type and associated parameters
        switch( type )
        {
        case CURSOR_TYPE_BASIC:
        case CURSOR_TYPE_OVERLAY:
            return id < CURSOR_ID_COUNT && variation < CURSOR_VARIATION_COUNT && size >= CURSOR_SIZE_MIN && size < CURSOR_SIZE_COUNT;
        case CURSOR_TYPE_CLONE:
            return size >= CURSOR_SIZE_MIN && size < CURSOR_SIZE_COUNT;
        default:
            return false;
        }
    }
}
