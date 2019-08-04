#include <Core/Math/Number.hpp>
#include <intrin.h>

namespace Core
{
    // explicit instantiations
    //-------------------------------------------------------------------------
    template class Number<Char>;
    template class Number<Byte>;
    template class Number<Long>;
    template class Number<ULong>;
    template class Number<Float>;
    template class Number<Double>;

    // local
    //-------------------------------------------------------------------------
    namespace
    {
        /**/
        template<typename TYPE>
        Index _GetLowBit( TYPE value )
        {
            ASSERT( value != 0 );
            Index index = 0;

            while( true )
            {
                if( value & 1 )
                    return index;
                ++index;
                value >>= 1;
            }
        }

        template<typename TYPE>
        Index _GetHighBit( TYPE value )
        {
            ASSERT( value != 0 );
            Index index = (sizeof(TYPE) * 8) - 1;

            while( value >>= 1 )
                --index;

            return index;
        }
    }

    // public
    //-------------------------------------------------------------------------
    template<typename TYPE>
    Number<TYPE>::Number( TYPE value ):
        _value(value)
    {}

    //-------------------------------------------------------------------------
    template<typename TYPE>
    Number<TYPE>::operator TYPE() const
    {
        return _value;
    }
    template<typename TYPE>
    Number<TYPE> Number<TYPE>::operator=( TYPE value )
    {
        _value = value;
        return *this;
    }

    template<>
    Number<Byte> Number<Byte>::operator - () const
    {
        return 0;
    }
    template<>
    Number<ULong> Number<ULong>::operator - () const
    {
        return 0;
    }
    template<typename TYPE>
    Number<TYPE> Number<TYPE>::operator - () const
    {
        return -_value;
    }

    //-------------------------------------------------------------------------
    template<typename TYPE>
    Bool Number<TYPE>::IsNaN() const
    {
        return _value != _value;
    }

    //-------------------------------------------------------------------------
    template<>
    Number<Float> Number<Float>::GetRandom( Float min, Float max )
    {
        return static_cast<Float>(::rand()) * (max - min) / static_cast<Float>(RAND_MAX) + min;
    }
    template<>
    Number<Double> Number<Double>::GetRandom( Double min, Double max )
    {
        ASSERT_TODO;
        return 0;
    }
    template<typename TYPE>
    Number<TYPE> Number<TYPE>::GetRandom( TYPE min, TYPE max )
    {
        ASSERT( max >= min );
        //TODO3: improve
        return min + static_cast<TYPE>(((TYPE)::rand() * (TYPE)::rand()) % static_cast<TYPE>(max - min + 1));
    }

    //-------------------------------------------------------------------------
    template<typename TYPE>
    void Number<TYPE>::SetRandomSeed( TYPE value )
    {
        return ::srand( static_cast<ULong>(value) );
    }
}
