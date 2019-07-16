#include <Core/Math/Quaternion.hpp>
#include <math.h>

namespace Core
{
    // explicit instantiations
    //-------------------------------------------------------------------------
    template class BaseQuaternion<Float>;

    // statics
    //-------------------------------------------------------------------------
    template<typename TYPE> BaseQuaternion<TYPE> BaseQuaternion<TYPE>::IDENTITY()
    {
        return BaseQuaternion(0, 0, 0, 1);
    }

    // public
    //-------------------------------------------------------------------------
    template<typename TYPE>
    BaseQuaternion<TYPE>::BaseQuaternion( TYPE x, TYPE y, TYPE z, TYPE w ):
        _x(x),
        _y(y),
        _z(z),
        _w(w)
    {}

    template<typename TYPE>
    BaseQuaternion<TYPE>::BaseQuaternion( TYPE yaw, TYPE pitch, TYPE roll )
    {
        FromEuler( yaw, pitch, roll );
    }

    //-------------------------------------------------------------------------
    template<typename TYPE>
    BaseQuaternion<TYPE> BaseQuaternion<TYPE>::operator*( const BaseQuaternion& other ) const
    {
        return BaseQuaternion
        (
            _w * other._x + _x * other._w + _y * other._z - _z * other._y,
            _w * other._y + _y * other._w + _z * other._x - _x * other._z,
            _w * other._z + _z * other._w + _x * other._y - _y * other._x,
            _w * other._w - _x * other._x - _y * other._y - _z * other._z
        );
    }

    template<typename TYPE>
    typename BaseQuaternion<TYPE>::VectorType BaseQuaternion<TYPE>::operator*( const VectorType& vector ) const
    {
        VectorType qv(_x, _y, _z);
        VectorType uv = qv.CrossProduct(vector) * 2;

        return vector + (uv * _w) + qv.CrossProduct(uv);
    }

    template<typename TYPE>
    BaseQuaternion<TYPE> BaseQuaternion<TYPE>::operator-( const BaseQuaternion& other ) const
    {
        return BaseQuaternion( _x - other._x, _y - other._y, _z - other._z, _w - other._w );
    }

    //-------------------------------------------------------------------------
    template<typename TYPE>
    TYPE BaseQuaternion<TYPE>::GetX() const
    {
        return _x;
    }

    template<typename TYPE>
    TYPE BaseQuaternion<TYPE>::GetY() const
    {
        return _y;
    }

    template<typename TYPE>
    TYPE BaseQuaternion<TYPE>::GetZ() const
    {
        return _z;
    }

    template<typename TYPE>
    TYPE BaseQuaternion<TYPE>::GetW() const
    {
        return _w;
    }

    //-------------------------------------------------------------------------
    template<typename TYPE>
    void BaseQuaternion<TYPE>::FromAngleAxis( const TYPE angle, const VectorType& axis )
    {
        // q = cos(A/2) + sin(A/2) * (_x*i + _y*j + _z*k)

        TYPE hangle = static_cast<TYPE>(0.5) * angle;

        TYPE s = ::sinf(hangle);
        _w = ::cosf(hangle);

        _x = s * axis.x;
        _y = s * axis.y;
        _z = s * axis.z;
    }

    template<typename TYPE>
    void BaseQuaternion<TYPE>::FromEuler( TYPE yaw, TYPE pitch, TYPE roll )
    {
        // precompute common variables
        TYPE cy = ::cosf(static_cast<TYPE>(0.5) * yaw);
        TYPE sy = ::sinf(static_cast<TYPE>(0.5) * yaw);
        TYPE cp = ::cosf(static_cast<TYPE>(0.5) * pitch);
        TYPE sp = ::sinf(static_cast<TYPE>(0.5) * pitch);
        TYPE cr = ::cosf(static_cast<TYPE>(0.5) * roll);
        TYPE sr = ::sinf(static_cast<TYPE>(0.5) * roll);

        // compute quaternion
        _w = cy * cp * cr + sy * sp * sr;
        _x = cy * cp * sr - sy * sp * cr;
        _y = sy * cp * sr + cy * sp * cr;
        _z = sy * cp * cr - cy * sp * sr;
    }

    //-------------------------------------------------------------------------
    template<typename TYPE>
    void BaseQuaternion<TYPE>::ToAngleAxis( TYPE& angle, VectorType& axis )
    {
        TYPE t = ::acosf(_w);

        // precompute inverse sin theta
        TYPE si = static_cast<TYPE>(1) / ::sinf(t);

        // compute axis
        axis.x = _x * si;
        axis.y = _y * si;
        axis.z = _z * si;

        // compute angle
        angle = t * static_cast<TYPE>(2);
    }

    template<typename TYPE>
    void BaseQuaternion<TYPE>::ToEuler( VectorType& euler ) const
    {
        TYPE sqw = _w * _w;
        TYPE sqx = _x * _x;
        TYPE sqy = _y * _y;
        TYPE sqz = _z * _z;

        // compute euler
        euler.x = ::atan2f(static_cast<TYPE>(1) - static_cast<TYPE>(2) * (sqx + sqy), static_cast<TYPE>(2) * (_z * _y + _x * _w));
        euler.y = ::asinf(static_cast<TYPE>(-2) * (_x * _z - _y * _w));
        euler.z = ::atan2f(static_cast<TYPE>(1) - static_cast<TYPE>(2) * (sqy + sqz), static_cast<TYPE>(2) * (_x * _y + _z * _w));
    }

    template<typename TYPE>
    void BaseQuaternion<TYPE>::ToMatrix( BaseMatrix4<TYPE>& mrot ) const
    {
        TYPE tx  = _x + _x;
        TYPE ty  = _y + _y;
        TYPE tz  = _z + _z;
        TYPE twx = tx * _w;
        TYPE twy = ty * _w;
        TYPE twz = tz * _w;
        TYPE txx = tx * _x;
        TYPE txy = ty * _x;
        TYPE txz = tz * _x;
        TYPE tyy = ty * _y;
        TYPE tyz = tz * _y;
        TYPE tzz = tz * _z;

        mrot.e00 = static_cast<TYPE>(1) - (tyy + tzz);
        mrot.e10 = txy - twz;
        mrot.e20 = txz + twy;
        mrot.e30 = 0;

        mrot.e01 = txy + twz;
        mrot.e11 = static_cast<TYPE>(1) - (txx + tzz);
        mrot.e21 = tyz - twx;
        mrot.e31 = 0;

        mrot.e02 = txz - twy;
        mrot.e12 = tyz + twx;
        mrot.e22 = static_cast<TYPE>(1) - (txx + tyy);
        mrot.e32 = 0;

        mrot.e03 = 0;
        mrot.e13 = 0;
        mrot.e23 = 0;
        mrot.e33 = static_cast<TYPE>(1);
    }

    //-------------------------------------------------------------------------
    template<typename TYPE>
    BaseQuaternion<TYPE> BaseQuaternion<TYPE>::Inverse() const
    {
        //TODO: assert check is normalized
        return BaseQuaternion(-_x, -_y, -_z, _w);
    }

    //-------------------------------------------------------------------------
    template<typename TYPE>
    void BaseQuaternion<TYPE>::Normalize()
    {
        TYPE m = ::sqrtf( _x * _x + _y * _y + _z * _z + _w * _w );
        if( m )
        {
            TYPE im = 1 / m;
            _x *= im;
            _y *= im;
            _z *= im;
            _w *= im;
        }
    }
}
