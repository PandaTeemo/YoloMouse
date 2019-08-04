#pragma once
#include <YoloMouse/Loader/Overlay/Rendering/RenderContext.hpp>
#include <YoloMouse/Loader/Overlay/Rendering/Assets/Texture.hpp>
#include <Gdiplus.h>

namespace Yolomouse
{
    /**/
    class FontMapBuilder
    {
    public:
        /**/
        FontMapBuilder();
        ~FontMapBuilder();

        /**/
        Bool Initialize( const WChar* font_name, Float font_size, ULong map_size );
        void Shutdown();

        /**/
        Bool IsInitialized() const;

        /**/
        Bool GetCharacterSize( Vector2f& size, WChar character );

        /**/
        Bool WriteCharacter( const Vector2f& offset, WChar character );

        /**/
        Bool ToDxMap( Texture& texture, RenderContext& render_context );

    private:
        // fields
        ULong                   _map_size;
        ULONG_PTR               _token;
        Gdiplus::Graphics*      _graphics;
        Gdiplus::Font*          _font;
        Gdiplus::Bitmap*        _bitmap;
        Gdiplus::StringFormat*  _string_format;
    };
}
