#pragma once
#include <Core/Root.hpp>

namespace Core
{
    /**/
    template<typename TYPE>
    class BaseVector4
    {
    public:
        // constants
        static const BaseVector4 ZERO;

        // types
        typedef TYPE Type;

        /**/
        BaseVector4() = default;
        BaseVector4( TYPE xyzw );
        BaseVector4( TYPE x_, TYPE y_, TYPE z_, TYPE w_ );
        BaseVector4( const BaseVector4& other );

        /**/
        Bool        operator == ( const BaseVector4& other ) const;
        Bool        operator != ( const BaseVector4& other ) const;
        Bool        operator <  ( const BaseVector4& other ) const;
        Bool        operator <= ( const BaseVector4& other ) const;
        Bool        operator >  ( const BaseVector4& other ) const;
        Bool        operator >= ( const BaseVector4& other ) const;
        BaseVector4 operator +  ( const BaseVector4& other ) const;
        BaseVector4 operator -  ( const BaseVector4& other ) const;
        BaseVector4 operator *  ( const BaseVector4& other ) const;
        BaseVector4 operator /  ( const BaseVector4& other ) const;
        BaseVector4 operator += ( const BaseVector4& other );
        BaseVector4 operator -= ( const BaseVector4& other );
        BaseVector4 operator *= ( const BaseVector4& other );
        BaseVector4 operator /= ( const BaseVector4& other );

        /**/
        void Set( TYPE xyzw );
        void Set( TYPE x_, TYPE y_, TYPE z_, TYPE w_ );

        // fields: public
        TYPE x, y, z, w;
    };

    // common typedefs
    //-------------------------------------------------------------------------
    typedef BaseVector4<ULong> Vector4u;
    typedef BaseVector4<Float> Vector4f;
}
