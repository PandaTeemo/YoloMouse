#pragma once
#include <YoloMouse/Share/Bindings/CursorBindings.hpp>
#include <Yolomouse/Share/Enums.hpp>

namespace Yolomouse
{
    /**/
    struct InjectSessionEvent
    {
        // enums
        enum EventId: Id
        {
            CURSOR_CHANGING,
            CURSOR_SHOWING0,
        };

        // fields
        EventId id;

        // data
        union U
        {
            // CURSOR_CHANGING
            CursorBindings::Binding  cursor_changing;
            // CURSOR_SHOWING0
            Bool                     cursor_showing;

            U(){}
        }
        u;
    };
}
