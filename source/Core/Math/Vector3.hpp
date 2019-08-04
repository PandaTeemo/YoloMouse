#pragma once
#include <Core/Root.hpp>

namespace Core
{
    /**/
    template<typename TYPE>
    class BaseVector3
    {
    public:
        // constants
        static BaseVector3 ZERO();

        // types
        typedef TYPE Type;

        /**/
        BaseVector3() = default;
        BaseVector3( TYPE xyz );
        BaseVector3( TYPE x_, TYPE y_, TYPE z_ );
        BaseVector3( const BaseVector3& other );

        /**/
        Bool        operator == ( const BaseVector3& other ) const;
        Bool        operator != ( const BaseVector3& other ) const;
        Bool        operator <  ( const BaseVector3& other ) const;
        Bool        operator <= ( const BaseVector3& other ) const;
        Bool        operator >  ( const BaseVector3& other ) const;
        Bool        operator >= ( const BaseVector3& other ) const;
        BaseVector3 operator +  ( const BaseVector3& other ) const;
        BaseVector3 operator -  ( const BaseVector3& other ) const;
        BaseVector3 operator *  ( const BaseVector3& other ) const;
        BaseVector3 operator /  ( const BaseVector3& other ) const;
        BaseVector3 operator += ( const BaseVector3& other );
        BaseVector3 operator -= ( const BaseVector3& other );
        BaseVector3 operator *= ( const BaseVector3& other );
        BaseVector3 operator /= ( const BaseVector3& other );
        BaseVector3 operator -  () const;

        /**/
        void Set( TYPE xyz );
        void Set( TYPE x_, TYPE y_, TYPE z_ );

        /**/
        TYPE        Length() const;
        TYPE        SquareLength() const;
        BaseVector3 Normal() const;
        BaseVector3 CrossProduct( const BaseVector3& other ) const;
        TYPE        DotProduct( const BaseVector3& other ) const;
        TYPE        Sum() const;

        // fields: public
        TYPE x, y, z;
    };

    // common typedefs
    //-------------------------------------------------------------------------
    typedef BaseVector3<Float> Vector3f;
    typedef BaseVector3<ULong> Vector3u;
}
