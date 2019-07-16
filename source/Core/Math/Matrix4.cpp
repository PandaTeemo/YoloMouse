#include <Core/Math/Matrix4.hpp>

namespace Core
{
    // explicit instantiations
    //-------------------------------------------------------------------------
    template class BaseMatrix4<Float>;

    // statics
    //-------------------------------------------------------------------------
    template<typename TYPE> BaseMatrix4<TYPE> BaseMatrix4<TYPE>::IDENTITY()
    {
        return BaseMatrix4(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
    }
    template<typename TYPE> BaseMatrix4<TYPE> BaseMatrix4<TYPE>::ZERO()
    {
        return BaseMatrix4(
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0
        );
    }

    // public
    //-------------------------------------------------------------------------
    template<typename TYPE>
    BaseMatrix4<TYPE>::BaseMatrix4(
        TYPE e00_, TYPE e01_, TYPE e02_, TYPE e03_,
        TYPE e10_, TYPE e11_, TYPE e12_, TYPE e13_,
        TYPE e20_, TYPE e21_, TYPE e22_, TYPE e23_,
        TYPE e30_, TYPE e31_, TYPE e32_, TYPE e33_ ):
        e00(e00_), e01(e01_), e02(e02_), e03(e03_),
        e10(e10_), e11(e11_), e12(e12_), e13(e13_),
        e20(e20_), e21(e21_), e22(e22_), e23(e23_),
        e30(e30_), e31(e31_), e32(e32_), e33(e33_)
    {}

    //-------------------------------------------------------------------------
    template<typename TYPE>
    TYPE* BaseMatrix4<TYPE>::operator[]( Index row )
    {
        ASSERT( row < 4 );
        return m2[row];
    }

    template<typename TYPE>
    const TYPE* BaseMatrix4<TYPE>::operator[]( Index row ) const
    {
        ASSERT( row < 4 );
        return m2[row];
    }

    template<typename TYPE>
    BaseMatrix4<TYPE> BaseMatrix4<TYPE>::operator * ( const BaseMatrix4 &o ) const
    {
        BaseMatrix4 r;

        r.e00 = o.e00 * e00 + o.e01 * e10 + o.e02 * e20 + o.e03 * e30;
        r.e01 = o.e00 * e01 + o.e01 * e11 + o.e02 * e21 + o.e03 * e31;
        r.e02 = o.e00 * e02 + o.e01 * e12 + o.e02 * e22 + o.e03 * e32;
        r.e03 = o.e00 * e03 + o.e01 * e13 + o.e02 * e23 + o.e03 * e33;

        r.e10 = o.e10 * e00 + o.e11 * e10 + o.e12 * e20 + o.e13 * e30;
        r.e11 = o.e10 * e01 + o.e11 * e11 + o.e12 * e21 + o.e13 * e31;
        r.e12 = o.e10 * e02 + o.e11 * e12 + o.e12 * e22 + o.e13 * e32;
        r.e13 = o.e10 * e03 + o.e11 * e13 + o.e12 * e23 + o.e13 * e33;

        r.e20 = o.e20 * e00 + o.e21 * e10 + o.e22 * e20 + o.e23 * e30;
        r.e21 = o.e20 * e01 + o.e21 * e11 + o.e22 * e21 + o.e23 * e31;
        r.e22 = o.e20 * e02 + o.e21 * e12 + o.e22 * e22 + o.e23 * e32;
        r.e23 = o.e20 * e03 + o.e21 * e13 + o.e22 * e23 + o.e23 * e33;

        r.e30 = o.e30 * e00 + o.e31 * e10 + o.e32 * e20 + o.e33 * e30;
        r.e31 = o.e30 * e01 + o.e31 * e11 + o.e32 * e21 + o.e33 * e31;
        r.e32 = o.e30 * e02 + o.e31 * e12 + o.e32 * e22 + o.e33 * e32;
        r.e33 = o.e30 * e03 + o.e31 * e13 + o.e32 * e23 + o.e33 * e33;

        return r;
    }

    template<typename TYPE>
    typename BaseMatrix4<TYPE>::VectorType BaseMatrix4<TYPE>::operator*( const VectorType& v ) const
    {
        return VectorType(
            e00 * v.x + e10 * v.y + e20 * v.z,
            e01 * v.x + e11 * v.y + e21 * v.z,
            e02 * v.x + e12 * v.y + e22 * v.z
        );
    }

    template<typename TYPE>
    BaseMatrix4<TYPE> BaseMatrix4<TYPE>::operator + ( const BaseMatrix4 &w ) const
    {
        BaseMatrix4 r;

        r.e00 = e00 + w.e00;
        r.e01 = e01 + w.e01;
        r.e02 = e02 + w.e02;
        r.e03 = e03 + w.e03;

        r.e10 = e10 + w.e10;
        r.e11 = e11 + w.e11;
        r.e12 = e12 + w.e12;
        r.e13 = e13 + w.e13;

        r.e20 = e20 + w.e20;
        r.e21 = e21 + w.e21;
        r.e22 = e22 + w.e22;
        r.e23 = e23 + w.e23;

        r.e30 = e30 + w.e30;
        r.e31 = e31 + w.e31;
        r.e32 = e32 + w.e32;
        r.e33 = e33 + w.e33;

        return r;
    }

    template<typename TYPE>
    BaseMatrix4<TYPE> BaseMatrix4<TYPE>::operator - ( const BaseMatrix4 &w ) const
    {
        BaseMatrix4 r;

        r.e00 = e00 - w.e00;
        r.e01 = e01 - w.e01;
        r.e02 = e02 - w.e02;
        r.e03 = e03 - w.e03;

        r.e10 = e10 - w.e10;
        r.e11 = e11 - w.e11;
        r.e12 = e12 - w.e12;
        r.e13 = e13 - w.e13;

        r.e20 = e20 - w.e20;
        r.e21 = e21 - w.e21;
        r.e22 = e22 - w.e22;
        r.e23 = e23 - w.e23;

        r.e30 = e30 - w.e30;
        r.e31 = e31 - w.e31;
        r.e32 = e32 - w.e32;
        r.e33 = e33 - w.e33;

        return r;
    }

    //-------------------------------------------------------------------------
    template<typename TYPE>
    void BaseMatrix4<TYPE>::Set(
        TYPE e00_, TYPE e01_, TYPE e02_, TYPE e03_,
        TYPE e10_, TYPE e11_, TYPE e12_, TYPE e13_,
        TYPE e20_, TYPE e21_, TYPE e22_, TYPE e23_,
        TYPE e30_, TYPE e31_, TYPE e32_, TYPE e33_ )
    {
        e00 = e00_; e01 = e01_; e02 = e02_; e03 = e03_;
        e10 = e10_; e11 = e11_; e12 = e12_; e13 = e13_;
        e20 = e20_; e21 = e21_; e22 = e22_; e23 = e23_;
        e30 = e30_; e31 = e31_; e32 = e32_; e33 = e33_;
    }

    //-------------------------------------------------------------------------
    template<typename TYPE>
    void BaseMatrix4<TYPE>::Translate( TYPE x, TYPE y, TYPE z )
    {
        e03 = x;
        e13 = y;
        e23 = z;
    }

    template<typename TYPE>
    void BaseMatrix4<TYPE>::Scale( TYPE x, TYPE y, TYPE z )
    {
        e00 = x;
        e11 = y;
        e22 = z;
    }

    //-------------------------------------------------------------------------
    template<typename TYPE>
    BaseMatrix4<TYPE> BaseMatrix4<TYPE>::Transpose(void) const
    {
        return BaseMatrix4(
            e00, e10, e20, e30,
            e01, e11, e21, e31,
            e02, e12, e22, e32,
            e03, e13, e23, e33
        );
    }

    /**/
    template<typename TYPE>
    BaseMatrix4<TYPE> BaseMatrix4<TYPE>::Inverse() const
    {
        TYPE v0 = e20 * e31 - e21 * e30;
        TYPE v1 = e20 * e32 - e22 * e30;
        TYPE v2 = e20 * e33 - e23 * e30;
        TYPE v3 = e21 * e32 - e22 * e31;
        TYPE v4 = e21 * e33 - e23 * e31;
        TYPE v5 = e22 * e33 - e23 * e32;

        TYPE t00 = +(v5 * e11 - v4 * e12 + v3 * e13);
        TYPE t10 = -(v5 * e10 - v2 * e12 + v1 * e13);
        TYPE t20 = +(v4 * e10 - v2 * e11 + v0 * e13);
        TYPE t30 = -(v3 * e10 - v1 * e11 + v0 * e12);

        TYPE idet = 1 / (t00 * e00 + t10 * e01 + t20 * e02 + t30 * e03);

        TYPE d00 = t00 * idet;
        TYPE d10 = t10 * idet;
        TYPE d20 = t20 * idet;
        TYPE d30 = t30 * idet;

        TYPE d01 =- (v5 * e01 - v4 * e02 + v3 * e03) * idet;
        TYPE d11 =+ (v5 * e00 - v2 * e02 + v1 * e03) * idet;
        TYPE d21 =- (v4 * e00 - v2 * e01 + v0 * e03) * idet;
        TYPE d31 =+ (v3 * e00 - v1 * e01 + v0 * e02) * idet;

        v0 = e10 * e31 - e11 * e30;
        v1 = e10 * e32 - e12 * e30;
        v2 = e10 * e33 - e13 * e30;
        v3 = e11 * e32 - e12 * e31;
        v4 = e11 * e33 - e13 * e31;
        v5 = e12 * e33 - e13 * e32;

        TYPE d02 =+ (v5 * e01 - v4 * e02 + v3 * e03) * idet;
        TYPE d12 =- (v5 * e00 - v2 * e02 + v1 * e03) * idet;
        TYPE d22 =+ (v4 * e00 - v2 * e01 + v0 * e03) * idet;
        TYPE d32 =- (v3 * e00 - v1 * e01 + v0 * e02) * idet;

        v0 = e21 * e10 - e20 * e11;
        v1 = e22 * e10 - e20 * e12;
        v2 = e23 * e10 - e20 * e13;
        v3 = e22 * e11 - e21 * e12;
        v4 = e23 * e11 - e21 * e13;
        v5 = e23 * e12 - e22 * e13;

        TYPE d03 =- (v5 * e01 - v4 * e02 + v3 * e03) * idet;
        TYPE d13 =+ (v5 * e00 - v2 * e02 + v1 * e03) * idet;
        TYPE d23 =- (v4 * e00 - v2 * e01 + v0 * e03) * idet;
        TYPE d33 =+ (v3 * e00 - v1 * e01 + v0 * e02) * idet;

        return BaseMatrix4(
            d00, d01, d02, d03,
            d10, d11, d12, d13,
            d20, d21, d22, d23,
            d30, d31, d32, d33
        );
    }
}
