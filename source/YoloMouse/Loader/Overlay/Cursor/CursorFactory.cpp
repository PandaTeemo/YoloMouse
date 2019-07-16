#include <YoloMouse/Loader/Overlay/Cursor/CursorFactory.hpp>
#include <YoloMouse/Loader/Overlay/Cursor/Cursors/ArrowCursor.hpp>

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
        default:
            return nullptr;
        }
    }
}
