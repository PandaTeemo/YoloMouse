#pragma once
#include <Yolomouse/Share/Root.hpp>

namespace Yolomouse
{
    /**/
    struct OverlayEvent
    {
        // enums
        enum EventId: Id
        {
            WINDOW_HOVER,
        };

        // fields
        EventId id;
        HWND    hwnd;
    };
}
