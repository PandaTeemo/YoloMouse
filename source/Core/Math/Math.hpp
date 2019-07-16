#pragma once
#include <Core/Root.hpp>

namespace Core
{
    /**/
    template<typename TYPE>
    class Math
    {
    public:
        // constants
        static const TYPE PI;
        static const TYPE PI2;

        /**/
        static TYPE SquareRoot( TYPE value );

        /**/
        static TYPE Absolute( TYPE value );
    };
}
