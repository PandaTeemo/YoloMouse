#include <Core/Math/Math.hpp>
#include <Core/Math/Transform3.hpp>
#include <Core/Windows/WindowTools.hpp>
#include <YoloMouse/Loader/Core/App.hpp>
#include <YoloMouse/Loader/Overlay/Cursor/Cursors/BasicCursor.hpp>

namespace Yolomouse
{
    // public
    //-------------------------------------------------------------------------
    BasicCursor::BasicCursor():
        _cursor_loaded(false)
    {
    }

    BasicCursor::~BasicCursor()
    {
    }

    //-------------------------------------------------------------------------
    Bool BasicCursor::SetCursor( CursorId id, CursorVariation variation, CursorSize size )
    {
        // if cursor already loaded
        if( _cursor_loaded )
        {
            // unload last cursor
            _cursor_vault.UnloadBasic( GetId(), GetVariation(), GetSize() );

            // reset cursor loaded
            _cursor_loaded = false;
        }

        // load vault cursor
        HCURSOR hcursor = _cursor_vault.LoadBasic(id, variation, size);
        if( hcursor != NULL )
        {
            // set cursor loaded
            _cursor_loaded = true;

            // update texture from cursor and call base
            if( _UpdateTextureFromCursor( hcursor ) && BaseCursor::SetCursor(id, variation, size) )
                return true;
        }

        return false;
    }

    // private
    //-------------------------------------------------------------------------
    Bool BasicCursor::_OnInitialize()
    {
        // disable auto scale
        _SetAutoScale(false);

        // initialized cursor vault
        _cursor_vault.Initialize( App::Instance().GetHostPath() );

        return true;
    }

    void BasicCursor::_OnShutdown()
    {
        ASSERT( IsInitialized() );

        // shutdown texture
        if( _texture.IsInitialized() )
            _texture.Shutdown();

        // shutdown mesh
        if( _mesh.IsInitialized() )
            _mesh.Shutdown();

        // shutdown cursor vault
        _cursor_vault.Shutdown();
    }

    void BasicCursor::_OnUpdate( UpdateDef& def )
    {
        // static position, no animation
    }

    //-------------------------------------------------------------------------
    Bool BasicCursor::_InitializeGeometry( const Vector2f& hotspot, const Vector2f& size )
    {
        // determine rectangle coordinates at origin with hotspot in mind
        Vector2f tl = -hotspot;
        Vector2f br = size - hotspot;

        // define vertices
        ShaderVertex VERTICES[] = {
            { { tl.x, tl.y, 0 }, {1,1,1,1}, {0,0,1}, {0,0} },
            { { br.x, tl.y, 0 }, {1,1,1,1}, {0,0,1}, {1,0} },
            { { br.x, br.y, 0 }, {1,1,1,1}, {0,0,1}, {1,1} },
            { { tl.x, br.y, 0 }, {1,1,1,1}, {0,0,1}, {0,1} },
        };

        // define indices
        Index3 INDICES[] = {
            { 0, 1, 2 },
            { 0, 2, 3 },
        };

        // initialize geometry
        return BaseCursor::_mesh.Initialize({
            *_render_context,
            Array<Index3>      (INDICES, COUNT(INDICES)),
            Array<ShaderVertex>(VERTICES, COUNT(VERTICES))
        });
    }

    //-------------------------------------------------------------------------
    Bool BasicCursor::_UpdateTextureFromCursor( HCURSOR hcursor )
    {
        ICONINFO icon_info = {};
        BITMAP   bitmap_info;
        Bool     status = false;

        // get base icon and bitmap info from cursor
        if( GetIconInfo(hcursor, &icon_info) &&
            icon_info.hbmColor != NULL &&
            GetObject(icon_info.hbmColor, sizeof(BITMAP), &bitmap_info) != 0 )//icon_info.hbmColor
        {
            Byte4*   color_pixels;
            Vector2l color_size;

            // read color pixels
            if( WindowTools::ReadHBitmapPixels( color_pixels, color_size, icon_info.hbmColor ) )
            {
                ULong    pixel_count = color_size.x * color_size.y;
                Byte4*   mask_pixels;
                Vector2l mask_size;

                // if mask (alpha) bitmap exists, read mask pixels
                if( icon_info.hbmMask != NULL && 
                    WindowTools::ReadHBitmapPixels( mask_pixels, mask_size, icon_info.hbmMask ) )
                {
                    // require matching size
                    if( mask_size == color_size )
                    {
                        // mod alpha of color pixels using mask pixel values
                        for( Index i = 0; i < pixel_count; ++i)
                        {
                            // get current color pixel alpha value
                            Byte& alpha_pixel = reinterpret_cast<Byte*>(color_pixels + i)[3];

                            // skip if color pixel already has legit alpha value
                            if( alpha_pixel == 0xff || alpha_pixel == 0 )
                                alpha_pixel = mask_pixels[i] ? 0 : 0xff;
                        }
                    }

                    // cleanup
                    delete[] mask_pixels;
                }

                // translate bitmap size to NDC coordinates
                Float    resolution_y = static_cast<Float>(Overlay::Instance().GetWindow().GetSize().y);
                Vector2f nds_hotspot = Vector2l(icon_info.xHotspot, icon_info.yHotspot).Cast<Float>() / resolution_y;
                Vector2f nds_size =    color_size.Cast<Float>() / resolution_y;

                // shutdown previous
                if( _texture.IsInitialized() )
                    _texture.Shutdown();
                if( _mesh.IsInitialized() )
                    _mesh.Shutdown();

                // initialize geometry and texture 
                if (_InitializeGeometry( nds_hotspot, nds_size ) &&
                    _texture.Initialize({ *_render_context, color_pixels, color_size }))
                    status = true;

                // cleanup
                delete[] color_pixels;
            }
        }
        else
            LOG("BasicCursor.UpdateTextureFromCursor.GetIconInfo");

        // cleanup resources
        if( icon_info.hbmColor != NULL )
            DeleteObject(icon_info.hbmColor);
        if( icon_info.hbmMask != NULL )
            DeleteObject(icon_info.hbmMask);

        return status;
    }
}
