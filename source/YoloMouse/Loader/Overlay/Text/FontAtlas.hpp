#pragma once
#include <YoloMouse/Loader/Overlay/Rendering/RenderContext.hpp>
#include <YoloMouse/Loader/Overlay/Rendering/Assets/Texture.hpp>

namespace Yolomouse
{
    /**/
    class FontAtlas
    {
    public:
        // types
        struct Glyph
        {
            Bool        active = false;
            Vector2f    offset;
            Vector2f    size;
            Vector2f    uv_min;
            Vector2f    uv_max;
        };

        /**/
        FontAtlas();
        ~FontAtlas();

        /**/
        Bool Initialize( RenderContext& render_context, const WChar* font_name );
        void Shutdown();

        /**/
        Bool IsInitialized() const;

        /**/
        const Glyph*   GetGlyph( Char character ) const;
        const Texture& GetTexture() const;

    private:
        // aliases
        typedef FlatArray<Glyph, 0x80> GlyphTable;

        // fields: parameters
        RenderContext*              _render_context;
        Texture                     _texture;
        GlyphTable                  _glyphs;
    };
}
