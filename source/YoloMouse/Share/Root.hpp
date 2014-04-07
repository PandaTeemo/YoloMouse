#pragma once
#include <Core/Root.hpp>
#include <Core/Support/Debug.hpp>
#include <Hooks/Hook.hpp>

namespace YoloMouse
{
    using namespace Core;
    using namespace Hooks;

    #if 0
        #define xlog(...) ((void)0)
    #else
        #define xlog Debug::Log
    #endif
}
