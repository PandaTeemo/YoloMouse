#include <Core/Math/Vector2.hpp>

namespace Core
{
    // explicit instantiations
    //-------------------------------------------------------------------------
    template class BaseVector2<Long>;
    template class BaseVector2<Float>;

    // statics
    //-------------------------------------------------------------------------
    template<typename TYPE> BaseVector2<TYPE> BaseVector2<TYPE>::ZERO()
    {
        return BaseVector2(0, 0);
    }

    // public
    //-------------------------------------------------------------------------
    template<typename TYPE>
    BaseVector2<TYPE>::BaseVector2( TYPE xy ):
        x(xy),
        y(xy)
    {
    }

    template<typename TYPE>
    BaseVector2<TYPE>::BaseVector2( TYPE x_, TYPE y_ ):
        x(x_),
        y(y_)
    {
    }

    template<typename TYPE>
    BaseVector2<TYPE>::BaseVector2( const BaseVector2& other ):
        x(other.x),
        y(other.y)
    {
    }

    //-------------------------------------------------------------------------
    template<typename TYPE>
    Bool BaseVector2<TYPE>::operator == ( const BaseVector2& other ) const
    {
        return x == other.x && y == other.y;
    }

    template<typename TYPE>
    Bool BaseVector2<TYPE>::operator != ( const BaseVector2& other ) const
    {
        return !(x == other.x && y == other.y);
    }

    template<typename TYPE>
    Bool BaseVector2<TYPE>::operator < ( const BaseVector2& other ) const
    {
        return x < other.x && y < other.y;
    }

    template<typename TYPE>
    Bool BaseVector2<TYPE>::operator <= ( const BaseVector2& other ) const
    {
        return x <= other.x && y <= other.y;
    }

    template<typename TYPE>
    Bool BaseVector2<TYPE>::operator > ( const BaseVector2& other ) const
    {
        return x > other.x && y > other.y;
    }

    template<typename TYPE>
    Bool BaseVector2<TYPE>::operator >= ( const BaseVector2& other ) const
    {
        return x >= other.x && y >= other.y;
    }

    template<typename TYPE>
    BaseVector2<TYPE> BaseVector2<TYPE>::operator + ( const BaseVector2& other ) const
    {
        return BaseVector2(x + other.x, y + other.y);
    }

    template<typename TYPE>
    BaseVector2<TYPE> BaseVector2<TYPE>::operator - ( const BaseVector2& other ) const
    {
        return BaseVector2(x - other.x, y - other.y);
    }

    template<typename TYPE>
    BaseVector2<TYPE> BaseVector2<TYPE>::operator *  ( const BaseVector2& other ) const
    {
        return BaseVector2(x * other.x, y * other.y);
    }

    template<typename TYPE>
    BaseVector2<TYPE> BaseVector2<TYPE>::operator /  ( const BaseVector2& other ) const
    {
        return BaseVector2(x / other.x, y / other.y);
    }

    template<typename TYPE>
    BaseVector2<TYPE> BaseVector2<TYPE>::operator += ( const BaseVector2& other )
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    template<typename TYPE>
    BaseVector2<TYPE> BaseVector2<TYPE>::operator -= ( const BaseVector2& other )
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    template<typename TYPE>
    BaseVector2<TYPE> BaseVector2<TYPE>::operator *= ( const BaseVector2& other )
    {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    template<typename TYPE>
    BaseVector2<TYPE> BaseVector2<TYPE>::operator /= ( const BaseVector2& other )
    {
        x /= other.x;
        y /= other.y;
        return *this;
    }

    template<typename TYPE>
    BaseVector2<TYPE> BaseVector2<TYPE>::operator - () const
    {
        return BaseVector2(-x, -y);
    }

    //-------------------------------------------------------------------------
    template<typename TYPE>
    void BaseVector2<TYPE>::Set( TYPE xy )
    {
        x = xy;
        y = xy;
    }

    template<typename TYPE>
    void BaseVector2<TYPE>::Set( TYPE x_, TYPE y_ )
    {
        x = x_;
        y = y_;
    }

    //-------------------------------------------------------------------------
    template<typename TYPE>
    TYPE BaseVector2<TYPE>::CrossProduct( const BaseVector2& other ) const
    {
        return x * other.y - y * other.x;
    }


    template<typename TYPE>
    TYPE BaseVector2<TYPE>::DotProduct( const BaseVector2& other ) const
    {
        return x * other.x + y * other.y;
    }

    template<typename TYPE>
    TYPE BaseVector2<TYPE>::Sum() const
    {
        return x + y;
    }
}
