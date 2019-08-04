#pragma once
#include <Core/Types.hpp>

namespace Core
{
    /**/
    template<typename TYPE>
    class Number
    {
    public:
        /**/
        Number() = default;
        Number( TYPE value );

        /**/
               operator TYPE () const;
        Number operator = ( TYPE value );
        Number operator - () const;

        /**/
        Bool IsNaN() const;

        /**/
        static Number GetRandom( TYPE min, TYPE max );

        /**/
        static void SetRandomSeed( TYPE value );
        /**/

    private:
        // fields
        TYPE _value;
    };
}
