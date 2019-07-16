#pragma once
#include <Core/Math/Matrix4.hpp>
#include <Core/Math/Quaternion.hpp>
#include <Core/Math/Vector3.hpp>

namespace Core
{
    /**/
    template<typename TYPE>
    class BaseTransform3
    {
    public:
        // types
        typedef BaseVector3<TYPE>    VectorType;
        typedef BaseQuaternion<TYPE> RotationType;
        typedef BaseMatrix4<TYPE>    Matrix4Type;

        // constants
        static BaseTransform3 IDENTITY();

        /**/
        BaseTransform3() = default;
        BaseTransform3( VectorType translation_ );
        BaseTransform3( VectorType translation_, RotationType orientation_ );

        /**/
        BaseTransform3 operator * ( const BaseTransform3& other ) const;
        VectorType     operator * ( VectorType vector ) const;
    
        /**/
        BaseTransform3 Inverse() const;
    
        /**/
        void ToMatrix4( Matrix4Type& m ) const;

        // fields
        VectorType   translation;
        RotationType orientation;
    };

    // common typedefs
    //-------------------------------------------------------------------------
    typedef BaseTransform3<Float> Transform3f;
}
