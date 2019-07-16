#pragma once
#include <Core/Root.hpp>

namespace Core
{
    /**/
    struct Default
    {
        /**/
        template<typename TYPE>
        static Bool Equator( const TYPE& l, const TYPE& r )
        {
            return l == r;
        }

        /**/
        template<typename TYPE>
        static Bool Comparator( const TYPE& l, const TYPE& r )
        {
            return l < r;
        }
    };
}
