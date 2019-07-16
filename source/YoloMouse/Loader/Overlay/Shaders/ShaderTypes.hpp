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
        Vector3f    position;
        Vector3f    normal;
        Vector4f    color;
    };

    struct VertexShaderConstantValue
    {
        Matrix4f mvp;
        Matrix4f model;
    };

    struct PixelShaderConstantValue
    {
        Vector4f light_color;
    };
}
