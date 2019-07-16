#include <Core/Math/Math.hpp>
#include <cmath>

namespace Core
{
    // explicit instantiations
    //-------------------------------------------------------------------------
    template class Math<Float>;
    template class Math<Long>;

    // statics
    //-------------------------------------------------------------------------
    template<> const Float Math<Float>::PI =    3.14159274f;
    template<> const Long  Math<Long>::PI =     3;
    template<> const Float Math<Float>::PI2 =   6.28318530f;
    template<> const Long  Math<Long>::PI2 =    6;

    //-------------------------------------------------------------------------
    template<>
    Float Math<Float>::SquareRoot( Float value )
    {
        return std::sqrtf(value);
    }
    template<>
    Long Math<Long>::SquareRoot( Long value )
    {
        return 0;
    }

    //-------------------------------------------------------------------------
    template<typename TYPE>
    TYPE Math<TYPE>::Absolute( TYPE value )
    {
        return static_cast<TYPE>(std::abs( value ));
    }
}
