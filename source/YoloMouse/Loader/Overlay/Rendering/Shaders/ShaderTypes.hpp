#pragma once
#include <YoloMouse/Share/Root.hpp>
#include <Core/Math/Matrix4.hpp>
#include <Core/Math/Vector2.hpp>
#include <Core/Math/Vector3.hpp>
#include <Core/Math/Vector4.hpp>

namespace Yolomouse
{
    // enum
    // TODO3: convert to flags
    enum PixelShaderType : Byte4
    {
        PIXELSHADERTYPE_UNLIT_BASIC =   0,
        PIXELSHADERTYPE_LIT_BASIC =     1,
        PIXELSHADERTYPE_UNLIT_TEXTURE = 2,
    };

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
        Matrix4f mvp;
        Matrix4f model;
        Vector4f variation_color;
    };

    struct PixelShaderConstantValue
    {
        Vector3f        light_vector;
        PixelShaderType type;
    };
}
