#pragma once
#include <Core/Root.hpp>

namespace Core
{
    /**/
    template<typename TYPE>
    class BaseVector2
    {
    public:
        // constants
        static BaseVector2 ZERO();

        // types
        typedef TYPE Type;

        /**/
        BaseVector2() = default;
        BaseVector2( TYPE xy );
        BaseVector2( TYPE x_, TYPE y_ );
        BaseVector2( const BaseVector2& other );

        /**/
        Bool        operator == ( const BaseVector2& other ) const;
        Bool        operator != ( const BaseVector2& other ) const;
        Bool        operator <  ( const BaseVector2& other ) const;
        Bool        operator <= ( const BaseVector2& other ) const;
        Bool        operator >  ( const BaseVector2& other ) const;
        Bool        operator >= ( const BaseVector2& other ) const;
        BaseVector2 operator +  ( const BaseVector2& other ) const;
        BaseVector2 operator -  ( const BaseVector2& other ) const;
        BaseVector2 operator *  ( const BaseVector2& other ) const;
        BaseVector2 operator /  ( const BaseVector2& other ) const;
        BaseVector2 operator += ( const BaseVector2& other );
        BaseVector2 operator -= ( const BaseVector2& other );
        BaseVector2 operator *= ( const BaseVector2& other );
        BaseVector2 operator /= ( const BaseVector2& other );
        BaseVector2 operator -  () const;

        /**/
        void Set( TYPE xy );
        void Set( TYPE x_, TYPE y_ );

        /**/
        TYPE CrossProduct( const BaseVector2& other ) const;
        TYPE DotProduct( const BaseVector2& other ) const;
        TYPE Sum() const;

        // fields: public
        TYPE x, y;
    };

    // common typedefs
    //-------------------------------------------------------------------------
    typedef BaseVector2<Long>   Vector2l;
    typedef BaseVector2<Float>  Vector2f;
}
