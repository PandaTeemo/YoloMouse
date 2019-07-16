#include <Core/Math/Math.hpp>
#include <Core/Math/Vector3.hpp>

namespace Core
{
    // explicit instantiations
    //-------------------------------------------------------------------------
    template class BaseVector3<Float>;

    // statics
    //-------------------------------------------------------------------------
    template<typename TYPE> BaseVector3<TYPE> BaseVector3<TYPE>::ZERO()
    {
        return BaseVector3( 0, 0, 0 );
    }

    // public
    //-------------------------------------------------------------------------
    template<typename TYPE>
    BaseVector3<TYPE>::BaseVector3( TYPE xyz ):
        x(xyz),
        y(xyz),
        z(xyz)
    {
    }

    template<typename TYPE>
    BaseVector3<TYPE>::BaseVector3( TYPE x_, TYPE y_, TYPE z_ ):
        x(x_),
        y(y_),
        z(z_)
    {
    }

    template<typename TYPE>
    BaseVector3<TYPE>::BaseVector3( const BaseVector3& other ):
        x(other.x),
        y(other.y),
        z(other.z)
    {
    }

    //-------------------------------------------------------------------------
    template<typename TYPE>
    Bool BaseVector3<TYPE>::operator == ( const BaseVector3& other ) const
    {
        return x == other.x && y == other.y && z == other.z;
    }

    template<typename TYPE>
    Bool BaseVector3<TYPE>::operator != ( const BaseVector3& other ) const
    {
        return !(x == other.x && y == other.y && z == other.z);
    }

    template<typename TYPE>
    Bool BaseVector3<TYPE>::operator < ( const BaseVector3& other ) const
    {
        return x < other.x && y < other.y && z < other.z;
    }

    template<typename TYPE>
    Bool BaseVector3<TYPE>::operator <= ( const BaseVector3& other ) const
    {
        return x <= other.x && y <= other.y && z <= other.z;
    }

    template<typename TYPE>
    Bool BaseVector3<TYPE>::operator > ( const BaseVector3& other ) const
    {
        return x > other.x && y > other.y && z > other.z;
    }

    template<typename TYPE>
    Bool BaseVector3<TYPE>::operator >= ( const BaseVector3& other ) const
    {
        return x >= other.x && y >= other.y && z >= other.z;
    }

    template<typename TYPE>
    BaseVector3<TYPE> BaseVector3<TYPE>::operator + ( const BaseVector3& other ) const
    {
        return BaseVector3(x + other.x, y + other.y, z + other.z);
    }

    template<typename TYPE>
    BaseVector3<TYPE> BaseVector3<TYPE>::operator - ( const BaseVector3& other ) const
    {
        return BaseVector3(x - other.x, y - other.y, z - other.z);
    }

    template<typename TYPE>
    BaseVector3<TYPE> BaseVector3<TYPE>::operator *  ( const BaseVector3& other ) const
    {
        return BaseVector3(x * other.x, y * other.y, z * other.z);
    }

    template<typename TYPE>
    BaseVector3<TYPE> BaseVector3<TYPE>::operator /  ( const BaseVector3& other ) const
    {
        return BaseVector3(x / other.x, y / other.y, z / other.z);
    }

    template<typename TYPE>
    BaseVector3<TYPE> BaseVector3<TYPE>::operator += ( const BaseVector3& other )
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    template<typename TYPE>
    BaseVector3<TYPE> BaseVector3<TYPE>::operator -= ( const BaseVector3& other )
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    template<typename TYPE>
    BaseVector3<TYPE> BaseVector3<TYPE>::operator *= ( const BaseVector3& other )
    {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        return *this;
    }

    template<typename TYPE>
    BaseVector3<TYPE> BaseVector3<TYPE>::operator /= ( const BaseVector3& other )
    {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        return *this;
    }

    template<typename TYPE>
    BaseVector3<TYPE> BaseVector3<TYPE>::operator - () const
    {
        return BaseVector3(-x, -y, -z);
    }

    //-------------------------------------------------------------------------
    template<typename TYPE>
    void BaseVector3<TYPE>::Set( TYPE xyz )
    {
        x = xyz;
        y = xyz;
        z = xyz;
    }

    template<typename TYPE>
    void BaseVector3<TYPE>::Set( TYPE x_, TYPE y_, TYPE z_ )
    {
        x = x_;
        y = y_;
        z = z_;
    }

    //-------------------------------------------------------------------------
    template<typename TYPE>
    TYPE BaseVector3<TYPE>::Length() const
    {
        return Math<TYPE>::SquareRoot(SquareLength());
    }

    template<typename TYPE>
    TYPE BaseVector3<TYPE>::SquareLength() const
    {
        return x * x + y * y + z * z;
    }

    template<typename TYPE>
    BaseVector3<TYPE> BaseVector3<TYPE>::Normal() const
    {
        TYPE length = Length();
        return BaseVector3(
            x / length,
            y / length,
            z / length);
    }

    template<typename TYPE>
    BaseVector3<TYPE> BaseVector3<TYPE>::CrossProduct( const BaseVector3& other ) const
    {
        return BaseVector3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x);
    }

    template<typename TYPE>
    TYPE BaseVector3<TYPE>::DotProduct( const BaseVector3& other ) const
    {
        return x * other.x + y * other.y + z * other.z;
    }

    template<typename TYPE>
    TYPE BaseVector3<TYPE>::Sum() const
    {
        return x + y + z;
    }
}
