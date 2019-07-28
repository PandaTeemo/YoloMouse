#pragma once
#include <Core/Root.hpp>
#include <windows.h>

namespace Core
{
    /**/
    struct SystemMonitorEvent
    {
        // enums
        enum EventId: Id
        {
            WINDOW_FOREGROUND,
        };

        // fields
        EventId id;
        HWND    hwnd;
    };
}
