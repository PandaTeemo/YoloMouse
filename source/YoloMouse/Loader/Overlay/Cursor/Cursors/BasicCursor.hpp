#pragma once
#include <YoloMouse/Loader/Overlay/Cursor/Cursors/BaseCursor.hpp>
#include <YoloMouse/Share/Cursor/CursorVault.hpp>

namespace Yolomouse
{
    /**/
    class BasicCursor:
        public BaseCursor
    {
    public:
        /**/
        BasicCursor();
        ~BasicCursor();

        /**/
        Bool SetCursor( CursorId id, CursorVariation variation, CursorSize size );

    private:
        /**/
        Bool _OnInitialize();

        /**/
        void _OnShutdown();

        /**/
        void _OnUpdate( UpdateDef& def );

        /**/
        Bool _InitializeGeometry( const Vector2f& hotspot, const Vector2f& size );

        /**/
        Bool _UpdateTextureFromCursor( HCURSOR hcursor );

        // fields: state
        Bool        _cursor_loaded;
        CursorVault _cursor_vault;
    };
}