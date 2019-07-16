#pragma once
#include <YoloMouse/Loader/Overlay/Cursor/IOverlayCursor.hpp>
#include <YoloMouse/Share/Enums.hpp>

namespace Yolomouse
{
    /**/
    class CursorFactory
    {
    public:
        /**/
        static IOverlayCursor* CreateCursor( CursorId id );
    };
}