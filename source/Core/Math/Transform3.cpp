//TODO: handle scale better
#include <Core/Math/Transform3.hpp>

namespace Core
{
    // explicit instantiations
    //-------------------------------------------------------------------------
    template class BaseTransform3<Float>;

    // statics
    //-------------------------------------------------------------------------
    template<typename TYPE> BaseTransform3<TYPE> BaseTransform3<TYPE>::IDENTITY()
    {
         return BaseTransform3( VectorType::ZERO(), RotationType::IDENTITY() );
    }

    // public
    //-------------------------------------------------------------------------
    template<typename TYPE>
    BaseTransform3<TYPE>::BaseTransform3( VectorType translation_ ):
        translation (translation_),
        orientation (RotationType::IDENTITY())
    {}

    template<typename TYPE>
    BaseTransform3<TYPE>::BaseTransform3( VectorType translation_, RotationType orientation_ ):
        translation(translation_),
        orientation(orientation_)
    {}

    //-------------------------------------------------------------------------
    template<typename TYPE>
    BaseTransform3<TYPE> BaseTransform3<TYPE>::operator * ( const BaseTransform3& other ) const
    {
        return BaseTransform3
        (
            operator * (other.translation),
            orientation * other.orientation
        );
    }
    
    template<typename TYPE>
    typename BaseTransform3<TYPE>::VectorType BaseTransform3<TYPE>::operator * ( VectorType vector ) const
    {
        return translation + orientation * vector;
    }

    //-------------------------------------------------------------------------
    template<typename TYPE>
    BaseTransform3<TYPE> BaseTransform3<TYPE>::Inverse() const
    {
        RotationType iorientation = orientation.Inverse();

        return BaseTransform3(
            iorientation * -translation,
            iorientation
        );
    }
    
    //-------------------------------------------------------------------------
    template<typename TYPE>
    void BaseTransform3<TYPE>::ToMatrix4( Matrix4Type& m ) const
    {
        // apply orientation
        orientation.ToMatrix(m);

        // apply translation
        m.e30 = translation.x;
        m.e31 = translation.y;
        m.e32 = translation.z;
    }
}
