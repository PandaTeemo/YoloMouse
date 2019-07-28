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

        /**/
        static TYPE Sin( TYPE radians );
        static TYPE ArcSin( TYPE radians );

        static TYPE Cos( TYPE radians );
        static TYPE ArcCos( TYPE radians );

        static TYPE Tan( TYPE radians );
        static TYPE ArcTan( TYPE x, TYPE y );
    };
}
