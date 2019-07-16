#pragma once
#include <Core/Math/Vector3.hpp>

namespace Core
{
    /**/
    template<typename TYPE>
    class BaseMatrix4
    {
    public:
        // types
        typedef TYPE              Type;
        typedef BaseVector3<TYPE> VectorType;

        // constants
        static BaseMatrix4 IDENTITY();
        static BaseMatrix4 ZERO();

        /**/
        BaseMatrix4() = default;
        BaseMatrix4(
            TYPE e00_, TYPE e01_, TYPE e02_, TYPE e03_,
            TYPE e10_, TYPE e11_, TYPE e12_, TYPE e13_,
            TYPE e20_, TYPE e21_, TYPE e22_, TYPE e23_,
            TYPE e30_, TYPE e31_, TYPE e32_, TYPE e33_ );

        /**/
        TYPE*       operator[]( Index row );
        const TYPE* operator[]( Index row ) const;
        BaseMatrix4 operator * ( const BaseMatrix4 &o ) const;
        VectorType  operator * ( const VectorType& v ) const;
        BaseMatrix4 operator + ( const BaseMatrix4 &w ) const;
        BaseMatrix4 operator - ( const BaseMatrix4 &w ) const;

        /**/
        void Set(
            TYPE e00_, TYPE e01_, TYPE e02_, TYPE e03_,
            TYPE e10_, TYPE e11_, TYPE e12_, TYPE e13_,
            TYPE e20_, TYPE e21_, TYPE e22_, TYPE e23_,
            TYPE e30_, TYPE e31_, TYPE e32_, TYPE e33_ );

        /**/
        void Translate( TYPE x, TYPE y, TYPE z );
        void Scale( TYPE x, TYPE y, TYPE z );

        /**/
        BaseMatrix4 Transpose(void) const;
        BaseMatrix4 Inverse() const;

        // fields
        union
        {
            TYPE m1[16];
            TYPE m2[4][4];
            struct
            {
                TYPE e00; TYPE e01; TYPE e02; TYPE e03;
                TYPE e10; TYPE e11; TYPE e12; TYPE e13;
                TYPE e20; TYPE e21; TYPE e22; TYPE e23;
                TYPE e30; TYPE e31; TYPE e32; TYPE e33;
            };
        };
    };

    // common typedefs
    //-------------------------------------------------------------------------
    typedef BaseMatrix4<Float> Matrix4f;
}
