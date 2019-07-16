#pragma once
#include <Core/Math/Vector2.hpp>
#include <YoloMouse/Loader/Overlay/Shaders/ShaderTypes.hpp>
#include <YoloMouse/Share/Enums.hpp>
#include <d3d11.h>

namespace Yolomouse
{
    /**/
    class IOverlayCursor
    {
    public:
        /**/
        virtual Bool IsInitialized() const = 0;

    protected:
        friend class Overlay;

        // types
        struct InitializeDef
        {
            ID3D11Device&        device;
            ID3D11DeviceContext& device_context;
            Float                aspect_ratio;
        };

        struct ResizeDef
        {
            Float                aspect_ratio;
        };

        struct UpdateDef
        {
            Float                      frame_time;
            const Vector2f             cursor_position;
            VertexShaderConstantValue& vs_constant_value;
            PixelShaderConstantValue&  ps_constant_value;
        };

        /**/
        virtual Bool Initialize( const InitializeDef& def ) = 0;
        virtual void Shutdown() = 0;

        /**/
        virtual void SetSceneSize( const ResizeDef& def ) = 0;
        virtual void SetCursorVariation( CursorVariation variation ) = 0;
        virtual void SetCursorSize( CursorSize size ) = 0;

        /**/
        virtual void Update( const UpdateDef& def ) = 0;

        /**/
        virtual void Render() const = 0;
   };
}