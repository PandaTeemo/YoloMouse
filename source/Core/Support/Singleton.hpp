#pragma once
#include <Core/Root.hpp>

namespace Core
{
    /**/
    template<typename TYPE>
    class Singleton
    {
    public:
        /**/
        static TYPE& Instance()
        {
            static TYPE instance;
            return instance;
        }
    };
}
