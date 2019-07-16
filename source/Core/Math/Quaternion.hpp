#pragma once
#include <Core/Math/Vector3.hpp>
#include <Core/Math/Matrix4.hpp>

namespace Core
{
    /*
        yaw:    Z
        pitch:  Y
        roll:   X
    */
    template<typename TYPE>
    class BaseQuaternion
    {
    public:
        // types
        typedef BaseVector3<TYPE> VectorType;

        // constants
        static BaseQuaternion IDENTITY();

        /**/
        BaseQuaternion() = default;
        BaseQuaternion( TYPE x, TYPE y, TYPE z, TYPE w );
        BaseQuaternion( TYPE yaw, TYPE pitch, TYPE roll );

        /**/
        BaseQuaternion operator * ( const BaseQuaternion& other ) const;
        VectorType     operator * ( const VectorType& vector ) const;
        BaseQuaternion operator - ( const BaseQuaternion& other ) const;

        /**/
        TYPE GetX() const;
        TYPE GetY() const;
        TYPE GetZ() const;
        TYPE GetW() const;

        /**/
        void FromAngleAxis( const TYPE angle, const VectorType& axis );
        void FromEuler( TYPE yaw, TYPE pitch, TYPE roll );

        /**/
        void ToAngleAxis( TYPE& angle, VectorType& axis );
        void ToEuler( VectorType& euler ) const;
        void ToMatrix( BaseMatrix4<TYPE>& matrix ) const;

        /**/
        BaseQuaternion Inverse() const;

        /**/
        void Normalize();

    private:
        // fields
        TYPE _x, _y, _z, _w;
    };

    // common typedefs
    //-------------------------------------------------------------------------
    typedef BaseQuaternion<Float> Quaternionf;
}
