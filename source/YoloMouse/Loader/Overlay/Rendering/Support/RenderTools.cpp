#include <YoloMouse/Loader/Overlay/Rendering/Support/RenderTools.hpp>

namespace Yolomouse
{
    // public
    //-------------------------------------------------------------------------
    void RenderTools::BuildOrthoProjectionMatrix( Matrix4f& matrix, Float aspect_ratio )
    {
        // calculate x,y cells. scale to NDC (normalized device coordinates)
        Float xx = 2.0f / aspect_ratio;
        Float yy = 2.0f;

        // update projection matrix
        matrix.Set(
            xx,  0,   0,   0,
            0,  -yy,  0,   0,
            0,   0,   1,   0,
            0,   0,   0,   1
        );
    }
}
