#pragma once
#include <Core/Root.hpp>
#include <windows.h>

namespace Core
{
    // forward defs
    //-------------------------------------------------------------------------
    class Window;

    /**/
    struct WindowEvent
    {
        Window* window;
        UINT    msg;
        WPARAM  wparam;
        LPARAM  lparam;
    };
}
