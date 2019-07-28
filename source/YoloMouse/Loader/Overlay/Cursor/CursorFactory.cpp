#include <YoloMouse/Loader/Overlay/Cursor/CursorFactory.hpp>
#include <YoloMouse/Loader/Overlay/Cursor/Cursors/ArrowCursor.hpp>
#include <YoloMouse/Loader/Overlay/Cursor/Cursors/CircleCursor.hpp>

namespace Yolomouse
{
    // public
    //-------------------------------------------------------------------------
    IOverlayCursor* CursorFactory::CreateCursor( CursorId id )
    {
        switch( id )
        {
        case 0:
            return new ArrowCursor();
        case 1:
            return new CircleCursor();
        default:
            return nullptr;
        }
    }
}
