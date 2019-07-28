#pragma once
#include <YoloMouse/Share/Root.hpp>
#include <Core/Math/Matrix4.hpp>
#include <Core/Math/Vector2.hpp>
#include <Core/Math/Vector3.hpp>
#include <Core/Math/Vector4.hpp>

namespace Yolomouse
{
    // types
    struct ShaderVertex
    {
        Vector3f position;
        Vector4f color;
        Vector3f normal;
        Vector2f uv;
    };

    struct VertexShaderConstantValue
    {
        Matrix4f mvp =              Matrix4f::IDENTITY();
        Matrix4f model =            Matrix4f::IDENTITY();
        Vector4f variation_color =  Vector4f(1,1,1,1);
    };

    struct PixelShaderConstantValue
    {
        Vector3f light_vector =     Vector3f(0,0,1);
        Byte4    texturing =        false;
    };
}
