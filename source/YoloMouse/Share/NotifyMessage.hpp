#pragma once
#include <YoloMouse/Share/Constants.hpp>

namespace YoloMouse
{
    /*
        with 64bit fields for 32/64 bit coexistence
    */
    struct NotifyMessage
    {
        // NotifyId
        Byte8 id;

        // additional parameter
        Byte8 parameter;
    };
}
