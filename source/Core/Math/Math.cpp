#include <Core/Math/Math.hpp>
#include <cmath>

namespace Core
{
    // explicit instantiations
    //-------------------------------------------------------------------------
    template class Math<Float>;
    template class Math<Long>;
    template class Math<ULong>;

    // statics
    //-------------------------------------------------------------------------
    template<> const Float Math<Float>::PI =    3.14159274f;
    template<> const Long  Math<Long>::PI =     3;
    template<> const ULong Math<ULong>::PI =    3;
    template<> const Float Math<Float>::PI2 =   6.28318530f;
    template<> const Long  Math<Long>::PI2 =    6;
    template<> const ULong Math<ULong>::PI2 =   6;

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
    template<>
    ULong Math<ULong>::SquareRoot( ULong value )
    {
        return 0;
    }

    //-------------------------------------------------------------------------
    template<>
    Float Math<Float>::Absolute( Float value )
    {
        return static_cast<Float>(std::fabs( value ));
    }
    template<>
    Long Math<Long>::Absolute( Long value )
    {
        return static_cast<Long>(std::abs( value ));
    }
    template<>
    ULong Math<ULong>::Absolute( ULong value )
    {
        return value;
    }

    //-------------------------------------------------------------------------
    template<>
    Float Math<Float>::Sin( Float radians )
    {
        return std::sinf(radians);
    }
    template<typename TYPE>
    TYPE Math<TYPE>::Sin( TYPE radians )
    {
        ASSERT_TODO;
        return 0;
    }

    template<>
    Float Math<Float>::ArcSin( Float radians )
    {
        return std::asinf(radians);
    }
    template<typename TYPE>
    TYPE Math<TYPE>::ArcSin( TYPE radians )
    {
        ASSERT_TODO;
        return 0;
    }

    template<>
    Float Math<Float>::Cos( Float radians )
    {
        return std::cosf(radians);
    }
    template<typename TYPE>
    TYPE Math<TYPE>::Cos( TYPE radians )
    {
        ASSERT_TODO;
        return 0;
    }

    template<>
    Float Math<Float>::ArcCos( Float radians )
    {
        return std::acosf(radians);
    }
    template<typename TYPE>
    TYPE Math<TYPE>::ArcCos( TYPE radians )
    {
        ASSERT_TODO;
        return 0;
    }

    template<>
    Float Math<Float>::Tan( Float radians )
    {
        return std::tanf(radians);
    }
    template<typename TYPE>
    TYPE Math<TYPE>::Tan( TYPE radians )
    {
        ASSERT_TODO;
        return 0;
    }

    template<>
    Float Math<Float>::ArcTan( Float x, Float y )
    {
        return std::atan2f(y, x);
    }
    template<typename TYPE>
    TYPE Math<TYPE>::ArcTan( TYPE x, TYPE y )
    {
        ASSERT_TODO;
        return 0;
    }
}
