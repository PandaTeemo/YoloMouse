#pragma once
#include <Core/Root.hpp>
#include <stdio.h>

namespace Core
{
    /**/
    class Debug
    {
    public:
        /**/
        static void Console( const Char* format, ... );
    };
}
