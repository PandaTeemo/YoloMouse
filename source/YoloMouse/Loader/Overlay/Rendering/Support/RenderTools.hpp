#pragma once
#include <Core/Math/Matrix4.hpp>
#include <YoloMouse/Share/Root.hpp>

namespace Yolomouse
{
    /**/
    struct RenderTools
    {
        /**/
        static void BuildOrthoProjectionMatrix( Matrix4f& matrix, Float aspect_ratio );
   };
}