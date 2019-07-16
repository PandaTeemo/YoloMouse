#include <Core/Math/Vector4.hpp>

namespace Core

{
    // explicit instantiations
    //-------------------------------------------------------------------------
    template class BaseVector4<Float>;

    // statics
    //-------------------------------------------------------------------------
    template<typename TYPE> const BaseVector4<TYPE> BaseVector4<TYPE>::ZERO( 0, 0, 0, 0 );

    // public
    //-------------------------------------------------------------------------
    template<typename TYPE>
    BaseVector4<TYPE>::BaseVector4( TYPE xyzw ):
        x(xyzw),
        y(xyzw),
        z(xyzw),
        w(xyzw)
    {
    }

    template<typename TYPE>
    BaseVector4<TYPE>::BaseVector4( TYPE x_, TYPE y_, TYPE z_, TYPE w_ ):
        x(x_),
        y(y_),
        z(z_),
        w(w_)
    {
    }

    template<typename TYPE>
    BaseVector4<TYPE>::BaseVector4( const BaseVector4& other ):
        x(other.x),
        y(other.y),
        z(other.z),
        w(other.w)
    {
    }

    //-------------------------------------------------------------------------
    template<typename TYPE>
    Bool BaseVector4<TYPE>::operator == ( const BaseVector4& other ) const
    {
        return x == other.x && y == other.y && z == other.z && w == other.w;
    }

    template<typename TYPE>
    Bool BaseVector4<TYPE>::operator != ( const BaseVector4& other ) const
    {
        return !(x == other.x && y == other.y && z == other.z && w == other.w);
    }

    template<typename TYPE>
    Bool BaseVector4<TYPE>::operator < ( const BaseVector4& other ) const
    {
        return x < other.x && y < other.y && z < other.z && w < other.w;
    }

    template<typename TYPE>
    Bool BaseVector4<TYPE>::operator <= ( const BaseVector4& other ) const
    {
        return x <= other.x && y <= other.y && z <= other.z && w <= other.w;
    }

    template<typename TYPE>
    Bool BaseVector4<TYPE>::operator > ( const BaseVector4& other ) const
    {
        return x > other.x && y > other.y && z > other.z && w > other.w;
    }

    template<typename TYPE>
    Bool BaseVector4<TYPE>::operator >= ( const BaseVector4& other ) const
    {
        return x >= other.x && y >= other.y && z >= other.z && w >= other.w;
    }

    template<typename TYPE>
    BaseVector4<TYPE> BaseVector4<TYPE>::operator + ( const BaseVector4& other ) const
    {
        return BaseVector4(x + other.x, y + other.y, z + other.z, w + other.w);
    }

    template<typename TYPE>
    BaseVector4<TYPE> BaseVector4<TYPE>::operator - ( const BaseVector4& other ) const
    {
        return BaseVector4(x - other.x, y - other.y, z - other.z, w - other.w);
    }

    template<typename TYPE>
    BaseVector4<TYPE> BaseVector4<TYPE>::operator *  ( const BaseVector4& other ) const
    {
        return BaseVector4(x * other.x, y * other.y, z * other.z, w * other.w);
    }

    template<typename TYPE>
    BaseVector4<TYPE> BaseVector4<TYPE>::operator /  ( const BaseVector4& other ) const
    {
        return BaseVector4(x / other.x, y / other.y, z / other.z, w / other.w);
    }

    template<typename TYPE>
    BaseVector4<TYPE> BaseVector4<TYPE>::operator += ( const BaseVector4& other )
    {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        return *this;
    }

    template<typename TYPE>
    BaseVector4<TYPE> BaseVector4<TYPE>::operator -= ( const BaseVector4& other )
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
    }

    template<typename TYPE>
    BaseVector4<TYPE> BaseVector4<TYPE>::operator *= ( const BaseVector4& other )
    {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        w *= other.w;
        return *this;
    }

    template<typename TYPE>
    BaseVector4<TYPE> BaseVector4<TYPE>::operator /= ( const BaseVector4& other )
    {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        w /= other.w;
        return *this;
    }

    //-------------------------------------------------------------------------
    template<typename TYPE>
    void BaseVector4<TYPE>::Set( TYPE xyzw )
    {
        x = xyzw;
        y = xyzw;
        z = xyzw;
        w = xyzw;
    }

    template<typename TYPE>
    void BaseVector4<TYPE>::Set( TYPE x_, TYPE y_, TYPE z_, TYPE w_ )
    {
        x = x_;
        y = y_;
        z = z_;
        w = w_;
    }
}
