#pragma once
#include <Core/Math/Vector2.hpp>
#include <YoloMouse/Loader/Overlay/Rendering/RenderContext.hpp>
#include <YoloMouse/Share/Cursor/CursorInfo.hpp>

namespace Yolomouse
{
    /**/
    class IOverlayCursor
    {
    public:
        // types
        struct InitializeDef
        {
            RenderContext& render_context;
            Float          aspect_ratio;
        };

        struct ResizeDef
        {
            Float aspect_ratio;
        };

        struct UpdateDef
        {
            const Vector2f cursor_position;
        };

        /**/
        virtual Bool Initialize( const InitializeDef& def ) = 0;
        virtual void Shutdown() = 0;

        /**/
        virtual Bool IsInitialized() const = 0;

        /**/
        virtual Bool SetCursor( CursorId id, CursorVariation variation, CursorSize size ) = 0;

        /**/
        virtual void Update( const UpdateDef& def ) = 0;

        /**/
        virtual void Draw() const = 0;

        /**/
        virtual void OnResize( const ResizeDef& def ) = 0;
    };
}
