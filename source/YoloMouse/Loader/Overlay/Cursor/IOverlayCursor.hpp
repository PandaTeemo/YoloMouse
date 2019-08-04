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
        /**/
        virtual Bool Initialize( RenderContext& render_context ) = 0;
        virtual void Shutdown() = 0;

        /**/
        virtual Bool IsInitialized() const = 0;

        /**/
        virtual Bool SetCursor( CursorId id, CursorVariation variation, CursorSize size ) = 0;
        virtual void SetAspectRatio( Float aspect_ratio ) = 0;

        /**/
        virtual void Draw( const Vector2f& position ) = 0;
    };
}
